#include "stdafx.h"
#include "AmanController.h"
#include "AmanTimelineView.h"
#include "AmanWindow.h"
#include "TitleBar.h"
#include "AmanTimeline.h"
#include "PopupMenu.h"
#include "MenuBar.h"

#include <algorithm> 

// In minutes:
#define MAX_HORIZON 240
#define MIN_HORIZON 5

#define TIMELINES_RELOAD "[Reload config] "

AmanWindow::AmanWindow(AmanController* controller) : Window("AmanWindow", "AMAN") {
    this->controller = controller;
    this->titleBar = std::make_shared<TitleBar>();
    this->menuBar = std::make_shared<MenuBar>();

    static auto onSelection = [controller, this](const std::string& timelineId) {
        if (timelineId == TIMELINES_RELOAD) {
            controller->reloadProfiles();
            this->zoomLevels.clear();
        } else {
            controller->toggleTimeline(timelineId);
        }
    };
    
    this->popupMenu = std::make_shared<PopupMenu>("Menu", std::vector<std::string>(), onSelection);
    this->menuBar->addPopupMenu(popupMenu);

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

void AmanWindow::setAvailableTimelines(std::set<std::string> availProfiles) {
    std::vector<std::string> profileOptions(availProfiles.begin(), availProfiles.end());
    profileOptions.push_back(TIMELINES_RELOAD);
    this->popupMenu->setMenuItems(profileOptions);
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
    if (timelinesToRender != nullptr) {
        for (auto& timeline : *timelinesToRender) {
            auto zoomSec = getZoomLevel(timeline) * 60;
            previousTimelineArea = AmanTimelineView::render(hdc, timeline, timelineView, zoomSec, previousTimelineArea.right);
            timelineIds.push_back(timeline->getIdentifier());
        }
    }
    renderTimelinesMutex.unlock();

    CRect titleBarRect = titleBar->render(clientRect, hdc);
    titleBarRect.MoveToY(titleBarRect.bottom);
    titleBarRect.bottom = titleBarRect.top + 15;

    popupMenu->setActiveItems(timelineIds);
    this->menuBar->render(hdc, titleBarRect);
}

uint32_t AmanWindow::getZoomLevel(const std::shared_ptr<AmanTimeline>& timeline) {
    auto &id = timeline->getIdentifier();

    if (this->zoomLevels.count(id)) {
        return this->zoomLevels[id];
    } else {
        return min(timeline->getDefaultZoom(), MAX_HORIZON);
    }
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

void AmanWindow::mouseMoved(CPoint cursorPosClient, CPoint cursorPosScreen) {
    if (menuBar->onMouseMove(cursorPosClient)) {
        requestRepaint();
    }

    CPoint diff = cursorPosScreen - prevMousePosition;
    if (this->doResize) {
        this->resizeWindowBy(diff);
    } else if (this->moveWindow) {
        this->moveWindowBy(diff);
    }
    prevMousePosition = cursorPosScreen;
}

void AmanWindow::mouseWheelSrolled(CPoint cursorPosClient, short delta) {
    if (menuBar->onMouseScroll(delta)) {
        requestRepaint();
        return;
    }

    auto timelinePointedAt = getTimelineAt(timelinesToRender, cursorPosClient);

    if (timelinePointedAt) {
        auto currentRange = getZoomLevel(timelinePointedAt);
        auto newRange = currentRange - delta / 60;
        auto limitReached = newRange < MIN_HORIZON || newRange > MAX_HORIZON;

        if (!limitReached) {
            this->zoomLevels[timelinePointedAt->getIdentifier()] = newRange;
            requestRepaint();
        }
    }
}

void AmanWindow::closeRequested() {
    controller->closeWindow();
}

void AmanWindow::windowClosed() {
}