#include "stdafx.h"

#include "AmanPlugIn.h"
#include "AmanController.h"
#include "AmanTimeline.h"
#include "AmanWindow.h"		
#include "Constants.h"
#include "TitleBar.h"

#include <ctime>
#include <sstream>
#include <iomanip>
#include <mutex>

AmanController* gpController;
std::vector<AmanTimeline> gpTimelines;

std::mutex timelineDataMutex;
HINSTANCE  hInstance;
CPoint position;
HWND hwnd;

TitleBar* titleBar;

int originalHeight;
bool minimized = false;

AmanWindow::AmanWindow(AmanController* controller) {
	titleBar = new TitleBar(controller);
	hInstance = GetModuleHandle(NULL);
	gpController = controller;
	CreateThread(0, NULL, AmanWindow::threadProc, NULL, NULL, &threadId);
}

void AmanWindow::render(std::vector<AmanTimeline>* timelines) {
	if (timelines) {
		timelineDataMutex.lock();
		gpTimelines = *timelines;
		timelineDataMutex.unlock();
	}
	PostThreadMessage(threadId, AIRCRAFT_DATA, NULL, NULL);
}

void AmanWindow::setWindowPosition(CRect rect) {
	MoveWindow(
		hwnd,
		rect.left,
		rect.top,
		rect.Width(), 
		rect.Height(),
		false
	);
}

void AmanWindow::collapse() {
	CRect windowRect;
	GetWindowRect(hwnd, &windowRect);
	originalHeight = windowRect.Height();
	windowRect.bottom = windowRect.top + AMAN_TITLEBAR_HEIGHT;
	AmanWindow::setWindowPosition(windowRect);
}

void AmanWindow::expand() {
	CRect windowRect;
	GetWindowRect(hwnd, &windowRect);
	windowRect.bottom = windowRect.top + originalHeight;
	AmanWindow::setWindowPosition(windowRect);
}

// Window thread procedure
DWORD WINAPI AmanWindow::threadProc(LPVOID lpParam)
{
	WNDCLASSEX wc;
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
	hwnd = CreateWindowEx(
		WS_EX_TOPMOST, 
		AMAN_WINDOW_CLASS_NAME, 
		AMAN_WINDOW_TITLE, 
		WS_POPUP,
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		600, 
		900, 
		prnt_hWnd,
		NULL, 
		hInstance, 
		NULL
	);

	SetWindowLong(
		hwnd, 
		GWL_STYLE, 
		GetWindowLong(hwnd, GWL_STYLE) | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU
	);
	
	ShowWindow(hwnd, SW_SHOWNORMAL);

	MSG msg;
	BOOL bRet;
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		switch (msg.message) {
		case AIRCRAFT_DATA:
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
		}
	}	
	return true;
}

// The window procedure
LRESULT CALLBACK AmanWindow::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CRect windowRect;
	CPoint cursorPosition;
	GetWindowRect(hwnd, &windowRect);
	GetCursorPos(&cursorPosition);

	int xPos;
	int yPos;
	int res;
	switch (message) {
		case WM_DESTROY: {
			PostQuitMessage(0);
		}
		break;
	case WM_SIZING: {
			InvalidateRect(hwnd, NULL, FALSE);
			AmanWindow::drawContent(hwnd);
		}
		break;
	case WM_SIZE: {
			InvalidateRect(hwnd, NULL, FALSE);
			AmanWindow::drawContent(hwnd);
		}
		break;
	case WM_PAINT: {
			AmanWindow::drawContent(hwnd);
		}
		break;
	case WM_CLOSE: {
			res = DestroyWindow(hwnd);
			res = UnregisterClass(AMAN_WINDOW_CLASS_NAME, hInstance);
			gpController->windowClosed();
		}
		break;
	case WM_LBUTTONDOWN: {
			SetCapture(hwnd);
			ScreenToClient(hwnd, &cursorPosition);
			titleBar->mousePressed(windowRect, cursorPosition);
		}
		break;
	case WM_LBUTTONUP: {
			ReleaseCapture();
			gpController->mouseReleased(windowRect, cursorPosition);
		}
		break;
	case WM_MOUSEMOVE: {
			gpController->mouseMoved(windowRect, cursorPosition);
			ScreenToClient(hwnd, &cursorPosition);
			titleBar->mouseHover(windowRect, cursorPosition);
		}
		break;
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
	long int now = static_cast<long int> (t);

	// Draw tools
	FillRect(memdc, &clientRect, AMAN_BRUSH_MAIN_BACKGROUND);

	int column = 0;
	timelineDataMutex.lock();
	for (int i = 0; i < gpTimelines.size(); i++) {
		if (gpTimelines.at(i).dual) {
			column++;
		}
		gpTimelines.at(i).render(clientRect, memdc, column);
		column++;
	}
	timelineDataMutex.unlock();
	
	// Menu
	titleBar->render(clientRect, memdc);
	
	BitBlt(hDC, 0, 0, winWidth, winHeight, memdc, 0, 0, SRCCOPY);
	DeleteObject(hBmp);
	DeleteDC(memdc);
	DeleteDC(hDC);
	EndPaint(hwnd, &ps);	
}

AmanWindow::~AmanWindow() {
	if (HWND hWnd = FindWindow(AMAN_WINDOW_CLASS_NAME, NULL)) {
		SendMessage(hWnd, WM_CLOSE, 0, 0);
		WaitForSingleObject(&threadId, INFINITE);
		TerminateThread(&threadId, 0);
	}
}