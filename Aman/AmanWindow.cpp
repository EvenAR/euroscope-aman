#include "stdafx.h"

#include "AmanPlugIn.h"
#include "AmanController.h"
#include "AmanTimeline.h"
#include "AmanWindow.h"

#define AMAN_BRUSH_BACKGROUND			CreateSolidBrush(RGB(48, 48, 48))

#include <ctime>
#include <sstream>
#include <iomanip>

AmanController* gpController;
std::vector<AmanTimeline>* gpTimelines = NULL;
HANDLE renderMutex;
bool closing = true;

AmanWindow::AmanWindow(AmanController* controller) {
	gpController = controller;
	closing = false;
	CreateThread(0, NULL, AmanWindow::ThreadProc, (LPVOID)(&this->timelines), NULL, &threadId);
	
	renderMutex = CreateMutex(NULL, FALSE, NULL);
}

void AmanWindow::render(std::vector<AmanTimeline>* timelines) {
	this->timelines = timelines;
	gpTimelines = this->timelines;

	WaitForSingleObject(renderMutex, INFINITE);
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
	wc.hbrBackground = AMAN_BRUSH_BACKGROUND;

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
	HPEN whitePen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	SelectObject(memdc, whitePen);

	if (gpTimelines) {
		int column = 0;
		for (int i = 0; i < gpTimelines->size(); i++) {
			if (gpTimelines->at(i).dual) {
				column++;
			}
			gpTimelines->at(i).render(clinetRect, memdc, column);
			column++;
		}
	}
	
	BitBlt(hDC, 0, 0, winWidth, winHeight, memdc, 0, 0, SRCCOPY);
	DeleteObject(hBmp);
	DeleteDC(memdc);
	DeleteDC(hDC);
	EndPaint(hwnd, &ps);

	ReleaseMutex(renderMutex);
}

AmanWindow::~AmanWindow() {
	ReleaseMutex(renderMutex);
	bool ok = TerminateThread(&threadId, 0);
	closing = true;
}