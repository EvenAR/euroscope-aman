#pragma once
#include <vector>
#include <string>

class AmanAircraft;
class AmanTimeline;

class AmanTimelineView {
public:
    static CRect render(AmanTimeline* timeline, CRect clientRect, HDC memdc, int xOffset);
    static CRect getArea(AmanTimeline* timeline, CRect clientRect, int i);

private:
    static void drawAircraftChain(HDC hdc, int timeNow, int xStart, int yStart, float pixelsPerSec, bool left, std::vector<AmanAircraft> aircraftList);
    static void drawMultiColorTextLine(HDC hdc, CRect rect, std::vector<std::tuple<int, bool, COLORREF, std::string>> texts);
};

