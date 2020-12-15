#include "stdafx.h"
#include "Window.h"
#include <memory>

Window::Window(const std::string& className, const std::string& windowName) {
    this->className = className;
    this->windowName = windowName;

    create();
    show(SW_SHOWNORMAL);

    // Thread responsible for updating the window
    exit = false;
    CreateThread(0, NULL, lookForMessages, this, NULL, &threadId);
}

Window::~Window() {
    exit = true;
    //PostQuitMessage(0);
    SendMessage(hwnd, WM_CLOSE, 0, 0);
    WaitForSingleObject(&threadId, INFINITE);
    int i  = 32;
    //TerminateThread(&threadId, 0);
}

// Window thread procedure
DWORD WINAPI Window::lookForMessages(LPVOID lpParam) {
    auto amanWindow = (Window*)lpParam;
    while (true) {
        if (!amanWindow->processNextMessage()) break;
    }
    return true;
}

bool Window::processNextMessage() {
    static MSG msg;

    if (::GetMessage(&msg, hwnd, 0, 0)) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    } else if(exit) {
        return false;
    }

    return true;
}

void Window::requestRepaint() {
    ::RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE); // Triggers WM_PAINT
}

bool Window::create() {
    HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASSEX wc;

    wc.hInstance = hInstance;
    wc.lpszClassName = className.c_str();
    wc.lpfnWndProc = messageRouter;
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
        className.c_str(),
        windowName.c_str(),
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

void Window::show(int nCmdShow) {
    ShowWindow(hwnd, SW_SHOWNORMAL);
}

LRESULT Window::messageRouter(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    Window* window = 0;

    if (message == WM_NCCREATE) {
        // retrieve Window instance from window creation data and associate
        window = reinterpret_cast<Window*>(((LPCREATESTRUCT)lparam)->lpCreateParams);
        ::SetWindowLong(hwnd, GWL_USERDATA, reinterpret_cast<long>(window));
        window->hwnd = hwnd;
    } else {
        // retrieve associated Window instance
        window = reinterpret_cast<Window*>(::GetWindowLong(hwnd, GWL_USERDATA));
    }

    return window->handleMessage(message, wparam, lparam);
}

// The window procedure
LRESULT CALLBACK Window::handleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
    CRect windowRect;
    CPoint cursorPosScreen;
    HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);

    GetWindowRect(hwnd, &windowRect);
    GetCursorPos(&cursorPosScreen);

    CPoint cursorPosClient = cursorPosScreen;
    ScreenToClient(hwnd, &cursorPosClient);

    switch (message) {
    case WM_CLOSE: {
        DestroyWindow(hwnd);
        UnregisterClass(className.c_str(), hInstance);
    } break;
    case WM_DESTROY: {
        windowClosed();
    } break;
    case WM_SIZING: {
        InvalidateRect(hwnd, NULL, FALSE);
        render(hwnd);
    } break;
    case WM_SIZE: {
        InvalidateRect(hwnd, NULL, FALSE);
        render(hwnd);
    } break;
    case WM_PAINT: {
        InvalidateRect(hwnd, NULL, FALSE);
        render(hwnd);
    } break;
    case WM_LBUTTONDOWN: {
        SetCapture(hwnd);
        mousePressed(cursorPosClient);
    } break;
    case WM_LBUTTONUP: {
        ReleaseCapture();
        mouseReleased(cursorPosClient);
    } break;
    case WM_MOUSEMOVE: {
        mouseMoved(cursorPosClient);
    } break;
    case WM_MOUSEWHEEL: {
        short delta = GET_WHEEL_DELTA_WPARAM(wParam);
        if (windowRect.PtInRect(cursorPosScreen)) {
            mouseWheelSrolled(cursorPosClient, delta);
        }
    } break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

void Window::render(HWND hwnd) {
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

    drawContent(memdc, clientRect);

    BitBlt(hDC, 0, 0, winWidth, winHeight, memdc, 0, 0, SRCCOPY);
    DeleteObject(hBmp);
    DeleteDC(memdc);
    DeleteDC(hDC);
    EndPaint(hwnd, &ps);
}

void Window::moveWindowBy(CPoint delta) {
    CRect windowRect;
    GetWindowRect(hwnd, &windowRect);
    windowRect.MoveToXY(windowRect.TopLeft() + delta);
    MoveWindow(hwnd, windowRect.left, windowRect.top, windowRect.Width(), windowRect.Height(), false);
}

void Window::resizeWindowBy(CPoint delta) {
    CRect windowRect;
    GetWindowRect(hwnd, &windowRect);
    windowRect.right += delta.x;
    windowRect.top += delta.y;
    MoveWindow(hwnd, windowRect.left, windowRect.top, windowRect.Width(), windowRect.Height(), false);
}