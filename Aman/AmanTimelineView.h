#pragma once
#include <vector>

class AmanAircraft;
class AmanTimeline;

class AmanTimelineView
{
public:
	static CRect render(AmanTimeline* timeline, CRect clientRect, HDC memdc, int offset);
	static CRect getArea(AmanTimeline* timeline, CRect clientRect, int i);

private:
	static void drawAircraftChain(HDC hdc, int timeNow, int xStart, int yStart, float pixelsPerSec, bool left, std::vector<AmanAircraft> aircraftList);
};

