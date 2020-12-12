#include "stdafx.h"

#include <ctime>

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
    gpCurrentTimelines = timelines;
    renderTimelinesMutex.unlock();

    requestRepaint();
}

void AmanWindow::drawContent(HWND hwnd) {
    CRect clientRect;
    int winWidth, winHeight;

    GetClientRect(hwnd, &clientRect);
    winWidth = clientRect.right - clientRect.left;
    winHeight = clientRect.bottom - clientRect.top;

    PAINTSTRUCT ps;
    HDC hDC = BeginPaint(hwnd, &ps);
    HDC memdc = CreateCompatibleDC(hDC);
    HBITMAP hBmp = CreateCompatibleBitmap(hDC, winWidth, winHeight);
    SelectObject(memdc, hBmp);

    // Draw stuff ///////////////////////////////////
    std::time_t t = std::time(nullptr);
    long int now = static_cast<long int>(t);

    // Draw tools
    FillRect(memdc, &clientRect, AMAN_BRUSH_MAIN_BACKGROUND);

    renderTimelinesMutex.lock(); // Wait for potential pointer update to complete
    CRect rectangle;
    for (AmanTimeline* timeline : gpCurrentTimelines) {
        rectangle = AmanTimelineView::render(timeline, clientRect, memdc, rectangle.right);
    }
    renderTimelinesMutex.unlock();

    // Menu
    titleBar->render(clientRect, memdc);

    BitBlt(hDC, 0, 0, winWidth, winHeight, memdc, 0, 0, SRCCOPY);
    DeleteObject(hBmp);
    DeleteDC(memdc);
    DeleteDC(hDC);
    EndPaint(hwnd, &ps);
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