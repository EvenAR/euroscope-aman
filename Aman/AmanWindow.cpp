#include "stdafx.h"

#include "AmanController.h"
#include "AmanPlugIn.h"
#include "AmanTimelineView.h"
#include "AmanWindow.h"
#include "Constants.h"
#include "TitleBar.h"

#include <condition_variable>
#include <ctime>
#include <iomanip>
#include <mutex>
#include <sstream>

AmanController* gpController;
std::vector<AmanTimeline*> gpCurrentTimelines;

std::mutex renderTimelinesMutex;
std::condition_variable cv;

HINSTANCE hInstance;
CPoint position;
HWND hwnd;
TitleBar* gpTitleBar;

int originalHeight;
bool minimized = false;

AmanWindow::AmanWindow(AmanController* controller, TitleBar* titleBar) {
    hInstance = GetModuleHandle(NULL);
    gpController = controller;
    gpTitleBar = titleBar;
    CreateThread(0, NULL, AmanWindow::threadProc, NULL, NULL, &threadId);
}

void AmanWindow::update(const std::vector<AmanTimeline*>& timelines) {
    renderTimelinesMutex.lock(); // Wait for current render to complete
    gpCurrentTimelines = timelines;
    renderTimelinesMutex.unlock();

    // Tell the window that new aircraft data is available
    PostThreadMessage(threadId, AIRCRAFT_DATA, 0, NULL);
}

// Window thread procedure
DWORD WINAPI AmanWindow::threadProc(LPVOID lpParam) {
    WNDCLASSEX wc{};
    wc.hInstance = hInstance;
    wc.lpszClassName = AMAN_WINDOW_CLASS_NAME;
    wc.lpfnWndProc = AmanWindow::windowProc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wc.hIconSm = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    wc.lpszMenuName = NULL;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = CreateSolidBrush(RGB(48, 48, 48));

    if (!RegisterClassEx(&wc)) {
        return false;
    }

    HWND prnt_hWnd = NULL;
    hwnd = CreateWindowEx(WS_EX_TOPMOST, AMAN_WINDOW_CLASS_NAME, AMAN_WINDOW_TITLE, WS_POPUP, CW_USEDEFAULT,
        CW_USEDEFAULT, 600, 900, prnt_hWnd, NULL, hInstance, NULL);

    SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);

    ShowWindow(hwnd, SW_SHOWNORMAL);

    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        switch (msg.message) {
        case AIRCRAFT_DATA:
            RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE); // Triggers WM_PAINT
        }
    }
    return true;
}

// The window procedure
LRESULT CALLBACK AmanWindow::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    CRect windowRect;
    CPoint cursorPosition;
    GetWindowRect(hwnd, &windowRect);
    GetCursorPos(&cursorPosition);

    switch (message) {
    case WM_DESTROY: {
        PostQuitMessage(0);
    } break;
    case WM_SIZING: {
        InvalidateRect(hwnd, NULL, FALSE);
        AmanWindow::drawContent(hwnd);
    } break;
    case WM_SIZE: {
        InvalidateRect(hwnd, NULL, FALSE);
        AmanWindow::drawContent(hwnd);
    } break;
    case WM_PAINT: {
        InvalidateRect(hwnd, NULL, FALSE);
        AmanWindow::drawContent(hwnd);
    } break;
    case WM_CLOSE: {
        DestroyWindow(hwnd);
        UnregisterClass(AMAN_WINDOW_CLASS_NAME, hInstance);
        gpController->windowClosed();
    } break;
    case WM_LBUTTONDOWN: {
        SetCapture(hwnd);
        ScreenToClient(hwnd, &cursorPosition);
        gpController->mousePressed(cursorPosition);
    } break;
    case WM_LBUTTONUP: {
        ReleaseCapture();
        gpController->mouseReleased(cursorPosition);
    } break;
    case WM_MOUSEMOVE: {
        gpController->mouseMoved(cursorPosition);
        ScreenToClient(hwnd, &cursorPosition);
    } break;
    case WM_MOUSEWHEEL: {
        short delta = GET_WHEEL_DELTA_WPARAM(wParam);
        if (windowRect.PtInRect(cursorPosition)) {
            gpController->mouseWheelSrolled(cursorPosition, delta);
        }
    } break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
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
    gpTitleBar->render(clientRect, memdc);

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

void AmanWindow::moveWindowBy(CPoint delta) {
    CRect windowRect;
    GetWindowRect(hwnd, &windowRect);
    windowRect.MoveToXY(windowRect.TopLeft() + delta);
    MoveWindow(hwnd, windowRect.left, windowRect.top, windowRect.Width(), windowRect.Height(), false);
}

void AmanWindow::resizeWindowBy(CPoint delta) {
    CRect windowRect;
    GetWindowRect(hwnd, &windowRect);
    windowRect.right += delta.x;
    windowRect.top += delta.y;
    MoveWindow(hwnd, windowRect.left, windowRect.top, windowRect.Width(), windowRect.Height(), false);
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

AmanWindow::~AmanWindow() {
    if (HWND hWnd = FindWindow(AMAN_WINDOW_CLASS_NAME, NULL)) {
        SendMessage(hWnd, WM_CLOSE, 0, 0);
        WaitForSingleObject(&threadId, INFINITE);
        TerminateThread(&threadId, 0);
    }
}