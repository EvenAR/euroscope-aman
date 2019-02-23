#pragma once
#include <vector>
#include <Windows.h>

#define AMAN_BRUSH_TIMELINE_AHEAD		CreateSolidBrush(RGB(115, 115, 115))
#define AMAN_BRUSH_TIMELINE_PAST		CreateSolidBrush(RGB(72, 72, 72))
#define AMAN_WHITE_PEN					CreatePen(PS_SOLID, 1, RGB(255, 255, 255))

#define AMAN_TIME_TEXT_COLOR			RGB(255, 255, 255)
#define AMAN_TRACKED_BY_ME_COLOR		RGB(255, 255, 255)			
#define AMAN_NOT_TRACKED_BY_ME_COLOR	RGB(120, 120, 120)			
#define AMAN_FIX_COLOR					RGB(0, 0, 0)			

#define AMAN_TIME_FONT					CreateFont(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 3, 2, 1, 49, "Courier New")
#define AMAN_LABEL_FONT					CreateFont(16, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 3, 2, 1, 49, "Courier New")
#define AMAN_FIX_FONT					CreateFont(18, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 3, 2, 1, 49, "Courier New")

#define AMAN_TIMELINE_WIDTH				60
#define AMAN_TIMELINE_TIME_POS			40
#define AMAN_TIMELINE_REALTIME_OFFSET	50
#define AMAN_WIDTH						350

#define AMAN_AIRCRAFT_LINE_HEIGHT		16

#define AMAN_LABEL_SEP_FROM_TIMELINE	20
#define AMAN_DOT_RADIUS					3

class AmanAircraft;

class AmanTimeline
{
private:
	int seconds;
	int resolution;

	void drawAircraftChain(HDC hdc, int xStart, int yStart, float pixelsPerSec, bool left, std::vector<AmanAircraft> aircraftList);
public:
	AmanTimeline(std::string fix, int seconds, int resolution);
	AmanTimeline(std::string fixLeft, std::string fixRight, int seconds, int resolution);
	void render(RECT clinetRect, HDC memdc, int i);

	bool dual;
	std::string fixes[2];
	std::vector<AmanAircraft>* aircraftLists;
};

