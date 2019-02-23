#pragma once
#include <vector>
#include "AmanAircraft.h"
#include "AmanTimeline.h"

#define AIRCRAFT_DATA		(WM_APP + 101) 

class AmanWindow
{
	
public:
	AmanWindow();
	~AmanWindow();

	void render(std::vector<AmanTimeline> timelines);
private:	
	std::vector<AmanTimeline> timelines;
	DWORD threadId;

	static LRESULT CALLBACK DLLWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static void DrawStuff(HWND hwnd);
	static DWORD WINAPI ThreadProc(LPVOID lpParam);
	
};

