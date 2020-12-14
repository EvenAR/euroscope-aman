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
    struct TextSegment {
        size_t width;
        bool rightAligned;
        COLORREF color;
        std::string text;
    };

    static void drawAircraftChain(HDC hdc, int timeNow, int xStart, int yStart, float pixelsPerSec, bool left, std::vector<AmanAircraft> aircraftList);
    static void drawMultiColorText(HDC hdc, CPoint pt, std::vector<TextSegment> texts, bool vertical = false);
};

