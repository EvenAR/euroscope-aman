#include "stdafx.h"

#include <algorithm> 

#include "AmanController.h"
#include "AmanTimelineView.h"
#include "AmanWindow.h"
#include "TitleBar.h"
#include "AmanTimeline.h"
#include "PopupMenu.h"

AmanWindow::AmanWindow(AmanController* controller, TitleBar* titleBar, std::set<std::string> ids) : Window("AmanWindow", "AMAN") {
    this->controller = controller;
    this->titleBar = titleBar;

    std::vector<std::string> options(ids.begin(), ids.end());
    this->popupMenu = new PopupMenu(options);
}

AmanWindow::~AmanWindow() {}

void AmanWindow::update(std::shared_ptr<std::vector<AmanTimeline*>> timelines) {
    renderTimelinesMutex.lock(); // Wait for current render to complete
    timelinesToRender = timelines;
    renderTimelinesMutex.unlock();

    requestRepaint();
}

void AmanWindow::drawContent(HDC hdc, CRect windowRect) {
    interactiveAreas.clear();
    FillRect(hdc, &windowRect, AMAN_BRUSH_MAIN_BACKGROUND);

    CRect lastTimelineArea;
    std::vector<std::string> timelineIds;

    // This code runs on the window's thread, so we must make sure
    // the main thread is not currently writing to the shared AmanTimeline-vector
    renderTimelinesMutex.lock(); 
    for (AmanTimeline* timeline : *timelinesToRender) {
        lastTimelineArea = AmanTimelineView::render(timeline, windowRect, hdc, lastTimelineArea.right);
        timelineIds.push_back(timeline->getIdentifier());
    }
    renderTimelinesMutex.unlock();

    CRect titleBarRect = titleBar->render(windowRect, hdc);

    // Menu bar
    CRect menuBarRect{ windowRect.left, windowRect.top + titleBarRect.Height(), windowRect.right, windowRect.top + titleBarRect.Height() + 13 };
    FillRect(hdc, &menuBarRect, AMAN_BRUSH_TIMELINE_AHEAD);
    renderButton(hdc, "Open", menuBarRect, OPEN_BUTTON);
    
    if (showOpenMenu) {
        CRect menuArea = popupMenu->render(hdc, { menuBarRect.left, menuBarRect.bottom }, timelineIds);
        interactiveAreas[PROFILES_MENU] = { menuArea };
    }

}

CRect AmanWindow::renderButton(HDC hdc, const std::string& text, CRect area, AREA_ID id) {
    // Renders the button and saves its bounding rectangle for later click detection

    SelectObject(hdc, AMAN_LEGEND_FONT);
    SetTextColor(hdc, AMAN_COLOR_UNTRACKED);

    DrawText(hdc, text.c_str(), text.length(), &area, DT_LEFT | DT_CALCRECT);
    DrawText(hdc, text.c_str(), text.length(), &area, DT_LEFT);

    interactiveAreas[id] = { area };

    return area;
}

int AmanWindow::findButtonAt(CPoint point) {
    for (auto& it : interactiveAreas) {
        if (it.second.area.PtInRect(point)) {
            return it.first;
        }
    }
    return -1;
}

AmanTimeline* AmanWindow::getTimelineAt(std::shared_ptr<std::vector<AmanTimeline*>> timelines, CPoint cursorPosition) {
    CRect windowRect;
    GetWindowRect(hwnd, &windowRect);
    ScreenToClient(hwnd, &cursorPosition);

    CRect coveringArea;
    for (AmanTimeline* timeline : *timelines) {
        coveringArea = AmanTimelineView::getArea(timeline, windowRect, coveringArea.right);
        if (coveringArea.PtInRect(cursorPosition)) {
            return timeline;
        }
    }
    return nullptr;
}

void AmanWindow::collapse() {
    CRect windowRect;
    GetWindowRect(hwnd, &windowRect);
    originalHeight = windowRect.Height();
    windowRect.bottom = windowRect.top + AMAN_TITLEBAR_HEIGHT;
    MoveWindow(hwnd, windowRect.left, windowRect.top, windowRect.Width(), windowRect.Height(), false);
}

void AmanWindow::expand() {
    CRect windowRect;
    GetWindowRect(hwnd, &windowRect);
    windowRect.bottom = windowRect.top + originalHeight;
    MoveWindow(hwnd, windowRect.left, windowRect.top, windowRect.Width(), windowRect.Height(), false);
}

bool AmanWindow::isExpanded() {
    CRect windowRect;
    GetWindowRect(hwnd, &windowRect);
    return windowRect.Height() != AMAN_TITLEBAR_HEIGHT;
}

void AmanWindow::mousePressed(CPoint cursorPosition) {
    controller->mousePressed(cursorPosition);

    int pressedBtn = findButtonAt(cursorPosition);

    switch (pressedBtn) {
        case OPEN_BUTTON: {
            showOpenMenu = !showOpenMenu;
            requestRepaint();
        }
        break;
        case PROFILES_MENU: {
            auto clicked = popupMenu->getClickedItem(cursorPosition);
            controller->toggleTimeline(clicked);
        }
        break;
        default: {
            showOpenMenu = false;
            requestRepaint();
        }
        break;
    }
}

void AmanWindow::mouseReleased(CPoint cursorPosition) {
    controller->mouseReleased(cursorPosition);
}

void AmanWindow::mouseMoved(CPoint cursorPosition) {
    controller->mouseMoved(cursorPosition);

    ScreenToClient(hwnd, &cursorPosition);
    if (popupMenu->onMouseHover(cursorPosition)) {
        requestRepaint();
    }
}

void AmanWindow::mouseWheelSrolled(CPoint cursorPosition, short delta) {
    controller->mouseWheelSrolled(cursorPosition, delta);
}

void AmanWindow::windowClosed() {
    controller->windowClosed();
}