#pragma once
#include <vector>

#define AIRCRAFT_DATA		(WM_APP + 101) 

class AmanController;
class AmanTimeline;

class AmanWindow
{
	
public:
	AmanWindow(AmanController* controller);
	~AmanWindow();

	void render(std::vector<AmanTimeline>* timelines);
private:	
	DWORD threadId;

	static LRESULT CALLBACK DLLWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static void DrawStuff(HWND hwnd);
	static DWORD WINAPI ThreadProc(LPVOID lpParam);
	
};

