#include "stdafx.h"
#include "AmanController.h"
#include "AmanTimelineView.h"
#include "AmanWindow.h"
#include "TitleBar.h"
#include "AmanTimeline.h"
#include "PopupMenu.h"
#include "MenuBar.h"

#include <algorithm> 

#define THREE_HOURS 10800
#define FIVE_MINUTES 300

AmanWindow::AmanWindow(AmanController* controller, std::set<std::string> availProfiles) : Window("AmanWindow", "AMAN") {
    this->controller = controller;
    this->titleBar = std::make_shared<TitleBar>();
    this->menuBar = std::make_shared<MenuBar>();

    static auto onSelection = [controller](const std::string& timelineId) {
        controller->toggleTimeline(timelineId);
    };

    std::vector<std::string> profileOptions(availProfiles.begin(), availProfiles.end());
    this->profilesMenu = std::make_shared<PopupMenu>("Load", profileOptions, onSelection);
    this->menuBar->addPopupMenu(profilesMenu);

    this->titleBar->on("COLLAPSE_CLICKED", [&]() {
        if (this->isExpanded()) {
            this->collapse();
        } else {
            this->expand();
        }
    });

    this->titleBar->on("MOUSE_PRESSED", [&]() { this->moveWindow = true; });

    this->titleBar->on("RESIZE_PRESSED", [&]() {
        if (this->isExpanded()) {
            this->doResize = true;
        }
    });
}

AmanWindow::~AmanWindow() {}

void AmanWindow::update(timelineCollection timelines) {
    renderTimelinesMutex.lock(); // Wait for current render to complete
    timelinesToRender = timelines;
    renderTimelinesMutex.unlock();

    requestRepaint();
}

void AmanWindow::drawContent(HDC hdc, CRect clientRect) {
    FillRect(hdc, &clientRect, AMAN_BRUSH_MAIN_BACKGROUND);

    CRect previousTimelineArea;
    std::vector<std::string> timelineIds;

    timelineView = clientRect;
    timelineView.top += 35;

    // This code runs on the window's thread, so we must make sure
    // the main thread is not currently writing to the shared AmanTimeline-vector
    renderTimelinesMutex.lock();
    for (auto& timeline : *timelinesToRender) {
        previousTimelineArea = AmanTimelineView::render(timeline, timelineView, hdc, previousTimelineArea.right);
        timelineIds.push_back(timeline->getIdentifier());
    }
    renderTimelinesMutex.unlock();

    CRect titleBarRect = titleBar->render(clientRect, hdc);
    titleBarRect.MoveToY(titleBarRect.bottom);
    titleBarRect.bottom = titleBarRect.top + 15;

    profilesMenu->setActiveItems(timelineIds);
    this->menuBar->render(hdc, titleBarRect);
}

std::shared_ptr<AmanTimeline> AmanWindow::getTimelineAt(timelineCollection timelines, CPoint cursorPosition) {
    int nextOffsetX = 0;
    for (auto& timeline : *timelines) {
        CRect area = AmanTimelineView::getArea(timeline, timelineView, nextOffsetX);
        if (area.PtInRect(cursorPosition)) {
            return timeline;
        }
        nextOffsetX = area.right;
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

void AmanWindow::mousePressed(CPoint cursorPosClient) {
    if (menuBar->onMouseClick(cursorPosClient)) {
        requestRepaint();
    }
    titleBar->mousePressed(cursorPosClient);
    this->mouseDownPosition = cursorPosClient;
}

void AmanWindow::mouseReleased(CPoint cursorPosClient) {
    this->moveWindow = false;
    this->doResize = false;
}

void AmanWindow::mouseMoved(CPoint cursorPosClient) {
    ClientToScreen(hwnd, &cursorPosClient);

    CPoint diff = cursorPosClient - prevMousePosition;

    if (this->doResize) {
        this->resizeWindowBy(diff);
    } else if (this->moveWindow) {
        this->moveWindowBy(diff);
    }

    if (menuBar->onMouseMove(cursorPosClient)) {
        requestRepaint();
    }

    prevMousePosition = cursorPosClient;
}

void AmanWindow::mouseWheelSrolled(CPoint cursorPosClient, short delta) {
    auto timelinePointedAt = getTimelineAt(timelinesToRender, cursorPosClient);
    if (timelinePointedAt) {
        auto currentRange = timelinePointedAt->getRange();
        auto newRange = currentRange - delta;
        auto limitReached = newRange < FIVE_MINUTES || newRange > THREE_HOURS;

        if (!limitReached) {
            timelinePointedAt->setRange(newRange);
            requestRepaint();
        }
    }
    if (menuBar->onMouseScroll(delta)) {
        requestRepaint();
    }
}

void AmanWindow::windowClosed() {
}