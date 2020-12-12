#include "stdafx.h"

#include "AmanController.h"
#include "AmanPlugIn.h"
#include "AmanTimelineView.h"
#include "AmanWindow.h"
#include "Constants.h"
#include "TitleBar.h"

#include <ctime>

AmanWindow::AmanWindow(AmanController* controller, TitleBar* titleBar) {
    gpController = controller;
    gpTitleBar = titleBar;

    create();
    show(SW_SHOWNORMAL);

    // Thread responsible for updating the window
    CreateThread(0, NULL, AmanWindow::lookForMessages, this, NULL, &threadId);
}

AmanWindow::~AmanWindow() {
    if (HWND hWnd = FindWindow(AMAN_WINDOW_CLASS_NAME, NULL)) {
        SendMessage(hWnd, WM_CLOSE, 0, 0);
        WaitForSingleObject(&threadId, INFINITE);
        TerminateThread(&threadId, 0);
    }
}

bool AmanWindow::create() {
    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASSEX wc;

    wc.hInstance = hInstance;
    wc.lpszClassName = AMAN_WINDOW_CLASS_NAME;
    wc.lpfnWndProc = AmanWindow::messageRouter;
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

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST,
        AMAN_WINDOW_CLASS_NAME,
        AMAN_WINDOW_TITLE,
        WS_POPUP,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        600,
        900,
        NULL,
        NULL,
        hInstance,
        this
    );

    SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);

    return true;
}

void AmanWindow::show(int nCmdShow) {
    ShowWindow(hwnd, SW_SHOWNORMAL);
}

// Window thread procedure
DWORD WINAPI AmanWindow::lookForMessages(LPVOID lpParam) {
    AmanWindow* amanWindow = (AmanWindow*)lpParam;
    while (true) {
        if (!amanWindow->handleMessages()) break;
    }
    return true;
}

void AmanWindow::update(const std::vector<AmanTimeline*>& timelines) {
    renderTimelinesMutex.lock(); // Wait for current render to complete
    gpCurrentTimelines = timelines;
    renderTimelinesMutex.unlock();

    ::RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE); // Triggers WM_PAINT
}

bool AmanWindow::handleMessages() {
    static MSG msg;

    if (!hwnd)
        throw std::runtime_error(std::string("Window not yet created"));

    if (::GetMessage(&msg, hwnd, 0, 0)) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    return true;
}

LRESULT CALLBACK AmanWindow::messageRouter(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    AmanWindow* window = 0;

    if (message == WM_NCCREATE) {
        // retrieve Window instance from window creation data and associate
        window = reinterpret_cast<AmanWindow*>(((LPCREATESTRUCT)lparam)->lpCreateParams);
        ::SetWindowLong(hwnd, GWL_USERDATA, reinterpret_cast<long>(window));
        window->hwnd = hwnd;
    } else {
        // retrieve associated Window instance
        window = reinterpret_cast<AmanWindow*>(::GetWindowLong(hwnd, GWL_USERDATA));
    }

    return window->handleMessage(message, wparam, lparam);
}

// The window procedure
LRESULT CALLBACK AmanWindow::handleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
    CRect windowRect;
    CPoint cursorPosition;
    HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);

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
