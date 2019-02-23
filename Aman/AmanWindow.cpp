#include "stdafx.h"

#include "AmanPlugIn.h"
#include "AmanController.h"
#include "AmanTimeline.h"
#include "AmanWindow.h"		

#include <ctime>
#include <sstream>
#include <iomanip>
#include <mutex>

AmanController* gpController;
std::vector<AmanTimeline> gpTimelines;
bool closing = true;

std::mutex timelineMutex;

AmanWindow::AmanWindow(AmanController* controller) {
	gpController = controller;
	closing = false;
	CreateThread(0, NULL, AmanWindow::ThreadProc, NULL, NULL, &threadId);
}

void AmanWindow::render(std::vector<AmanTimeline>* timelines) {
	timelineMutex.lock();
	gpTimelines = *timelines;
	timelineMutex.unlock();
	bool result = PostThreadMessage(threadId, AIRCRAFT_DATA, NULL, NULL);
}

//The new thread
DWORD WINAPI AmanWindow::ThreadProc(LPVOID lpParam)
{
	HINSTANCE  inj_hModule = NULL;          //Injected Modules Handle
	HWND       prnt_hWnd;            //Parent Window Handle
	HWND	   hwnd;

	// Register window class
	WNDCLASSEX wc;
	wc.hInstance = inj_hModule;
	wc.lpszClassName = (LPCSTR)L"InjectedDLLWindowClass";
	wc.lpfnWndProc = AmanWindow::DLLWindowProc;
	wc.style = CS_DBLCLKS;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = CreateSolidBrush(RGB(48, 48, 48));

	if (!RegisterClassEx(&wc))
		return 0;
	
	///////////
	MSG msg;
	prnt_hWnd = FindWindow("Window Injected Into ClassName", "Window Injected Into Caption");
	hwnd = CreateWindowEx(WS_EX_TOPMOST, (LPCSTR)L"InjectedDLLWindowClass", "AMAN", WS_EX_PALETTEWINDOW | WS_EX_TOPMOST, CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, prnt_hWnd, NULL, inj_hModule, NULL);
	SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_THICKFRAME);
	
	ShowWindow(hwnd, SW_SHOWNORMAL);

	BOOL bRet;
	while (!closing && (bRet = GetMessage(&msg, NULL, 0, 0)) != 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		switch (msg.message) {
		case AIRCRAFT_DATA:
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
		}
	}
	UnregisterClass((LPCSTR)L"InjectedDLLWindowClass", inj_hModule);
	return 1;
}
//Our new windows proc
LRESULT CALLBACK AmanWindow::DLLWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		gpController->windowClosed();
		break;
	case WM_SIZING:
		InvalidateRect(hwnd, NULL, FALSE);
		AmanWindow::DrawStuff(hwnd);
		break;
	case WM_SIZE:
		InvalidateRect(hwnd, NULL, FALSE);
		AmanWindow::DrawStuff(hwnd);
		break;
	case WM_PAINT:
		AmanWindow::DrawStuff(hwnd);
		break;
	case WM_CLOSE:
		closing = true;
		DestroyWindow(hwnd);
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

void AmanWindow::DrawStuff(HWND hwnd) {	
	

	RECT clinetRect;
	int winWidth, winHeight;

	GetClientRect(hwnd, &clinetRect);
	winWidth = clinetRect.right - clinetRect.left;
	winHeight = clinetRect.bottom - clinetRect.top;

	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hwnd, &ps);
	HDC memdc = CreateCompatibleDC(hDC);
	HBITMAP hBmp = CreateCompatibleBitmap(hDC, winWidth, winHeight);
	SelectObject(memdc, hBmp);

	// Draw stuff ///////////////////////////////////
	std::time_t t = std::time(nullptr);
	long int now = static_cast<long int> (t);

	// Draw tools
	FillRect(memdc, &clinetRect, AMAN_BRUSH_MAIN_BACKGROUND);

	int column = 0;
	timelineMutex.lock();
	for (int i = 0; i < gpTimelines.size(); i++) {
		if (gpTimelines.at(i).dual) {
			column++;
		}
		gpTimelines.at(i).render(clinetRect, memdc, column);
		column++;
	}
	timelineMutex.unlock();
	
	BitBlt(hDC, 0, 0, winWidth, winHeight, memdc, 0, 0, SRCCOPY);
	DeleteObject(hBmp);
	DeleteDC(memdc);
	DeleteDC(hDC);
	EndPaint(hwnd, &ps);	
}

AmanWindow::~AmanWindow() {
	closing = true;
	bool result = PostThreadMessage(threadId, WM_CLOSE, NULL, NULL);
	WaitForSingleObject(&threadId, INFINITE);
	bool ok = TerminateThread(&threadId, 0);
}