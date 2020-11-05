#pragma once
#include <vector>

#define AIRCRAFT_DATA			(WM_APP + 101) 
#define CLOSE_WINDOW			(WM_APP + 101) 
#define AMAN_WINDOW_CLASS_NAME	"AmanWindow"
#define AMAN_WINDOW_TITLE		"AMAN"

class AmanController;
class AmanTimeline;
class AmanRenderer;

class AmanWindow
{
	
public:
	AmanWindow(AmanController* controller);
	~AmanWindow();

	void update(std::vector<AmanTimeline*>* timelines);
	void setWindowPosition(CRect rect);
	void collapse();
	void expand();
	AmanTimeline* getTimelineAt(std::vector<AmanTimeline*>* timelines, CPoint cursorPosition);

private:	
	DWORD threadId;

	static void drawContent(HWND hwnd);
	static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI threadProc(LPVOID lpParam);
	static void close(HINSTANCE inj_hModule, HWND hwnd);
	static void mouseHover(CRect windowRect, CPoint cursorPosition);
};

