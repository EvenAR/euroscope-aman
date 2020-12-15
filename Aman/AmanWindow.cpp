#include "stdafx.h"
#include "AmanController.h"
#include "AmanTimelineView.h"
#include "AmanWindow.h"
#include "TitleBar.h"
#include "AmanTimeline.h"
#include "PopupMenu.h"
#include "MenuBar.h"

#include <algorithm> 

AmanWindow::AmanWindow(AmanController* controller, std::shared_ptr<TitleBar> titleBar, std::set<std::string> ids) : Window("AmanWindow", "AMAN") {
    this->controller = controller;
    this->titleBar = titleBar;
    this->menuBar = std::make_shared<MenuBar>();

    std::vector<std::string> options(ids.begin(), ids.end());

    static auto onSelection = [controller](const std::string& timelineId) {
        controller->toggleTimeline(timelineId);
    };

    this->profilesMenu = std::make_shared<PopupMenu>("Load", options, onSelection);
    this->menuBar->addPopupMenu(profilesMenu);
}

AmanWindow::~AmanWindow() {}

void AmanWindow::update(std::shared_ptr<std::vector<std::shared_ptr<AmanTimeline>>> timelines) {
    renderTimelinesMutex.lock(); // Wait for current render to complete
    timelinesToRender = timelines;
    renderTimelinesMutex.unlock();

    requestRepaint();
}

void AmanWindow::drawContent(HDC hdc, CRect clientRect) {
    FillRect(hdc, &clientRect, AMAN_BRUSH_MAIN_BACKGROUND);

    CRect lastTimelineArea;
    std::vector<std::string> timelineIds;

    // This code runs on the window's thread, so we must make sure
    // the main thread is not currently writing to the shared AmanTimeline-vector
    renderTimelinesMutex.lock();
    timelineView = clientRect;
    timelineView.top += 35;
    for (auto& timeline : *timelinesToRender) {
        lastTimelineArea = AmanTimelineView::render(timeline, timelineView, hdc, lastTimelineArea.right);
        timelineIds.push_back(timeline->getIdentifier());
    }
    renderTimelinesMutex.unlock();

    CRect titleBarRect = titleBar->render(clientRect, hdc);
    titleBarRect.MoveToY(titleBarRect.bottom);
    titleBarRect.bottom = titleBarRect.top + 15;

    profilesMenu->setActiveItems(timelineIds);
    this->menuBar->render(hdc, titleBarRect);
}

std::shared_ptr<AmanTimeline> AmanWindow::getTimelineAt(std::shared_ptr<std::vector<std::shared_ptr<AmanTimeline>>> timelines, CPoint cursorPosition) {
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
    controller->mousePressed(cursorPosClient);
    if (menuBar->onMouseClick(cursorPosClient)) {
        requestRepaint();
    }
}

void AmanWindow::mouseReleased(CPoint cursorPosClient) {
    controller->mouseReleased(cursorPosClient);
}

void AmanWindow::mouseMoved(CPoint cursorPosClient) {
    if (menuBar->onMouseMove(cursorPosClient)) {
        requestRepaint();
    }
    ClientToScreen(hwnd, &cursorPosClient);
    controller->mouseMoved(cursorPosClient);
}

void AmanWindow::mouseWheelSrolled(CPoint cursorPosClient, short delta) {
    controller->mouseWheelSrolled(cursorPosClient, delta);
    if (menuBar->onMouseScroll(delta)) {
        requestRepaint();
    }
}

void AmanWindow::windowClosed() {
    controller->windowClosed();
}