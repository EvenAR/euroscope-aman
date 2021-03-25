#pragma once
#include <vector>
#include <string>
#include <memory>

class AmanAircraft;
class AmanTimeline;

class AmanTimelineView {
public:
    static CRect render(HDC memdc, std::shared_ptr<AmanTimeline> timeline, CRect clientRect, uint32_t zoom, int xOffset);
    static CRect getArea(std::shared_ptr<AmanTimeline> timeline, CRect clientRect, int i);

private:
    struct TextSegment {
        size_t width;
        bool rightAligned;
        COLORREF color;
        std::string text;
    };

    static void drawAircraftChain(HDC hdc, int timeNow, int xStart, int yStart, float pixelsPerSec, bool left, std::vector<AmanAircraft> aircraftList);
    static void drawMultiColorText(HDC hdc, CPoint pt, std::vector<TextSegment> texts, bool vertical = false);
    static void drawViafixColorLegend(HDC hdc, std::shared_ptr<AmanTimeline> timeline, CPoint position);
};
