#include "stdafx.h"

#include "AmanController.h"
#include "AmanTimelineView.h"
#include "AmanWindow.h"
#include "Constants.h"
#include "TitleBar.h"

AmanWindow::AmanWindow(AmanController* controller, TitleBar* titleBar) : Window("AmanWindow", "AMAN") {
    this->controller = controller;
    this->titleBar = titleBar;
}

AmanWindow::~AmanWindow() {}

void AmanWindow::update(const std::vector<AmanTimeline*>& timelines) {
    renderTimelinesMutex.lock(); // Wait for current render to complete
    currentTimelines = timelines;
    renderTimelinesMutex.unlock();

    requestRepaint();
}

void AmanWindow::drawContent(HDC hdc, CRect windowRect) {
    FillRect(hdc, &windowRect, AMAN_BRUSH_MAIN_BACKGROUND);

    // This code runs on the window's thread, so we must make sure
    // the main thread is not currently writing to the shared AmanTimeline-vector
    renderTimelinesMutex.lock(); 
    CRect lastTimelineArea;
    for (AmanTimeline* timeline : currentTimelines) {
        lastTimelineArea = AmanTimelineView::render(timeline, windowRect, hdc, lastTimelineArea.right);
    }
    renderTimelinesMutex.unlock();

    titleBar->render(windowRect, hdc);
}

AmanTimeline* AmanWindow::getTimelineAt(const std::vector<AmanTimeline*>& timelines, CPoint cursorPosition) {
    CRect windowRect;
    GetWindowRect(hwnd, &windowRect);
    ScreenToClient(hwnd, &cursorPosition);

    CRect coveringArea;
    for (AmanTimeline* timeline : timelines) {
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
}

void AmanWindow::mouseReleased(CPoint cursorPosition) {
    controller->mouseReleased(cursorPosition);
}

void AmanWindow::mouseMoved(CPoint cursorPosition) {
    controller->mouseMoved(cursorPosition);
}

void AmanWindow::mouseWheelSrolled(CPoint cursorPosition, short delta) {
    controller->mouseWheelSrolled(cursorPosition, delta);
}

void AmanWindow::windowClosed() {
    controller->windowClosed();
}