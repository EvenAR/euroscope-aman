#include "stdafx.h"

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "AmanAircraft.h"
#include "AmanTimeline.h"
#include "AmanTimelineView.h"
#include "AmanTagItem.h"
#include "Constants.h"

#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))

#define CHAR_WIDTH 7
#define LANE_WIDTH(tagWidth) (tagWidth * CHAR_WIDTH + AMAN_TAG_SEP_FROM_TIMELINE + AMAN_TIMELINE_SEPARATION)

CRect AmanTimelineView::getArea(std::shared_ptr<AmanTimeline> timeline, CRect clientRect, int xOffset) {
    int totalWidth;
    int laneWidth = LANE_WIDTH(timeline->getWidth());

    if (timeline->isDual()) {
        totalWidth = laneWidth * 2 + AMAN_RULER_WITDH;
    } else {
        totalWidth = laneWidth + AMAN_RULER_WITDH;
    }

    return CRect(xOffset, clientRect.top, xOffset + totalWidth, clientRect.bottom);
}

CRect AmanTimelineView::render(HDC hdc, std::shared_ptr<AmanTimeline> timeline, CRect clientRect, uint32_t zoom, int xOffset) {
    CRect myTotalArea = getArea(timeline, clientRect, xOffset);

    long int unixTimestamp = static_cast<long int>(std::time(nullptr)); // Current UNIX-unixTimestamp in seconds
    int unixTimestampAsMinutes = (unixTimestamp / 60 + 1);              // UNIX time in minutes

    int timelineStartX = myTotalArea.left;
    if (timeline->isDual()) {
        timelineStartX += LANE_WIDTH(timeline->getWidth());
    }

    int presentTimeStartY = myTotalArea.bottom - AMAN_TIMELINE_REALTIME_OFFSET;
    double pixelsPerSecY = (float)(presentTimeStartY - myTotalArea.top) / (float)zoom;
    double pixelsPerMinuteY = 60.0 * pixelsPerSecY;

    // Timeline bar
    int secToNextMin = 60 - (unixTimestamp % 60);

    CRect futureBackground = { timelineStartX, myTotalArea.top, timelineStartX + AMAN_RULER_WITDH, myTotalArea.bottom };
    FillRect(hdc, &futureBackground, AMAN_BRUSH_TIMELINE_AHEAD);
    CRect pastBackground = { timelineStartX, myTotalArea.bottom - AMAN_TIMELINE_REALTIME_OFFSET,
                            timelineStartX + AMAN_RULER_WITDH, myTotalArea.bottom };
    FillRect(hdc, &pastBackground, AMAN_BRUSH_TIMELINE_PAST);

    // Vertical white border(s)
    HPEN oldPen = (HPEN)SelectObject(hdc, AMAN_VERTICAL_LINE_PEN);
    MoveToEx(hdc, timelineStartX + AMAN_RULER_WITDH, myTotalArea.bottom, NULL);
    LineTo(hdc, timelineStartX + AMAN_RULER_WITDH, myTotalArea.top);
    if (timeline->isDual()) {
        MoveToEx(hdc, timelineStartX, myTotalArea.bottom, NULL);
        LineTo(hdc, timelineStartX, myTotalArea.top);
    }

    // Render minute ticks with MM:SS-labels
    HFONT oldFont = (HFONT)SelectObject(hdc, AMAN_TIME_FONT);
    COLORREF oldColor = SetTextColor(hdc, AMAN_COLOR_TIME_TEXT);
    int oldBackground = SetBkMode(hdc, TRANSPARENT);

    int firstMinute = unixTimestampAsMinutes % 60;
    int firstMinutePositionY = floor(presentTimeStartY - secToNextMin * pixelsPerSecY);
    int pastMinutesToDisplay = floor(AMAN_TIMELINE_REALTIME_OFFSET / pixelsPerMinuteY);
    int futureMinutesToDisplay = floor(zoom / 60);

    for (int minute = -pastMinutesToDisplay; minute < futureMinutesToDisplay; minute++) {
        int tickPosY = firstMinutePositionY - (minute * pixelsPerMinuteY);
        int minutesAtTick = (firstMinute + minute) % 60;

        std::stringstream timeSs;
        if (minutesAtTick % 10 == 0) {
            // 10 minute label
            int hoursAtLine = ((unixTimestampAsMinutes + minute) / 60) % 24;
            timeSs << std::setfill('0') << std::setw(2) << hoursAtLine << ":" << std::setw(2) << minutesAtTick;
        } else if (minutesAtTick % 5 == 0 || pixelsPerMinuteY > AMAN_RULER_MAX_TICK_DENSITY) {
            // 5 and 1 minute label
            timeSs << std::setfill('0') << std::setw(2) << minutesAtTick;
        }

        auto timeString = timeSs.str();
        if (!timeString.empty()) {
            CRect rect = { timelineStartX, tickPosY - 6, timelineStartX + AMAN_RULER_WITDH, tickPosY + 6 };
            DrawText(hdc, timeString.c_str(), strlen(timeString.c_str()), &rect, DT_CENTER);
        }

        int tickLength = minutesAtTick % 5 == 0 ? AMAN_RULER_TICK_WIDTH_5_MIN : AMAN_RULER_TICK_WIDTH_1_MIN;

        MoveToEx(hdc, timelineStartX + AMAN_RULER_WITDH, tickPosY, NULL);
        LineTo(hdc, timelineStartX + AMAN_RULER_WITDH - tickLength, tickPosY);
        if (timeline->isDual()) {
            MoveToEx(hdc, timelineStartX, tickPosY, NULL);
            LineTo(hdc, timelineStartX + tickLength, tickPosY);
        }
    }

    auto drawHourglass = [hdc](int x, int y) {
        POINT polygon[] = { POINT{x - 4, y - 4}, POINT{x + 4, y + 4}, POINT{x + 4, y - 4}, POINT{x - 4, y + 4} };
        Polygon(hdc, polygon, 4);
    };
    oldPen = (HPEN)SelectObject(hdc, AMAN_WHITE_PEN);
    drawHourglass(timelineStartX + AMAN_RULER_WITDH, presentTimeStartY);
    if (timeline->isDual()) {
        drawHourglass(timelineStartX, presentTimeStartY);
    }

    // Draw aircraft
    SelectObject(hdc, AMAN_LABEL_FONT);

    if (timeline->isDual()) {
        auto fixes = timeline->getFixes();
        auto inboundsLeft = timeline->getAircraftList({ fixes[0] });
        auto inboundsRight = timeline->getAircraftList({ fixes[1] });
        drawAircraftChain(hdc, unixTimestamp, timelineStartX, presentTimeStartY, pixelsPerSecY, true, inboundsLeft, timeline->getTagItems());
        drawAircraftChain(hdc, unixTimestamp, timelineStartX, presentTimeStartY, pixelsPerSecY, false, inboundsRight, timeline->getTagItems());
    } else {
        drawAircraftChain(hdc, unixTimestamp, timelineStartX, presentTimeStartY, pixelsPerSecY, false,
            *timeline->getAircraftList(), timeline->getTagItems());
    }

    // Draw the fix id
    COLORREF oldBackgroundColor = SetBkColor(hdc, AMAN_COLOR_FIX_BACKGROUND);
    SetBkMode(hdc, OPAQUE);
    SetTextColor(hdc, AMAN_COLOR_FIX_TEXT);
    SelectObject(hdc, AMAN_FIX_FONT);
    CRect rect = { timelineStartX - AMAN_RULER_WITDH, myTotalArea.bottom - 20,
                  timelineStartX + 2 * AMAN_RULER_WITDH, myTotalArea.bottom };
    std::string text = timeline->getIdentifier();
    DrawText(hdc, text.c_str(), text.length(), &rect, DT_CENTER);

    // Draw color legend
    drawViafixColorLegend(hdc, timeline, myTotalArea.TopLeft());

    // Restore settings
    SetTextColor(hdc, oldColor);
    SelectObject(hdc, oldFont);
    SelectObject(hdc, oldPen);
    SetBkMode(hdc, oldBackground);
    SetBkColor(hdc, oldBackgroundColor);

    return myTotalArea;
}


void AmanTimelineView::drawAircraftChain(HDC hdc, int timeNow, int xStart, int yStart, float pixelsPerSec, bool left,
    std::vector<AmanAircraft> aircraftList, std::vector<std::shared_ptr<TagItem>> tagItems) {
    std::sort(aircraftList.begin(), aircraftList.end());

    int maxLabelWidth = 0;
    int previousLabelTop = -1;
    int offset = 0;

    for (int ac = 0; ac < aircraftList.size(); ac++) {
        AmanAircraft aircraft = aircraftList.at(ac);
        int acPosY = yStart - (aircraft.eta - timeNow) * pixelsPerSec;

        COLORREF defaultColor = aircraft.trackedByMe ? AMAN_COLOR_TRACKED : AMAN_COLOR_UNTRACKED;
        HBRUSH brush = aircraft.trackedByMe ? AMAN_TRACKED_BRUSH : AMAN_UNTRACKED_BRUSH;
        HPEN pen = aircraft.trackedByMe ? AMAN_WHITE_PEN : AMAN_GRAY_PEN;

        auto oldPen = SelectObject(hdc, pen);
        auto oldBrush = SelectObject(hdc, brush);
        auto oldTextColor = SetTextColor(hdc, defaultColor);
        auto labelSegments = generateLabel(aircraft, tagItems, defaultColor);

        if (maxLabelWidth == 0) {
            // For the first label, do a test render outside the view to estimate the width
            CRect boundingBox = drawMultiColorText(hdc, { -INFINITE, 0 }, labelSegments);
            maxLabelWidth = boundingBox.Width();
        }

        int labelPosX = left ? xStart - maxLabelWidth - AMAN_TAG_SEP_FROM_TIMELINE : xStart + AMAN_RULER_WITDH + AMAN_TAG_SEP_FROM_TIMELINE;
        int labelTop = acPosY - AMAN_TAG_LINE_HEIGHT / 2;
        int labelBottom = labelTop + AMAN_TAG_LINE_HEIGHT;
        if (previousLabelTop >= 0 && labelBottom > previousLabelTop) {
            int diff = labelBottom - previousLabelTop;
            labelTop -= diff;
            labelBottom -= diff;
        }

        CRect boundingBox = drawMultiColorText(hdc, { labelPosX, labelTop }, labelSegments);

        int rulerBorderX = left ? xStart : xStart + AMAN_RULER_WITDH;
        Ellipse(hdc, rulerBorderX - AMAN_DOT_RADIUS, acPosY - AMAN_DOT_RADIUS, rulerBorderX + AMAN_DOT_RADIUS, acPosY + AMAN_DOT_RADIUS);
        MoveToEx(hdc, rulerBorderX, acPosY, NULL);
        LineTo(hdc, left ? boundingBox.right : boundingBox.left, boundingBox.top + AMAN_TAG_LINE_HEIGHT / 2);
        
        previousLabelTop = boundingBox.top;
        maxLabelWidth = max(maxLabelWidth, boundingBox.Width());

        if (aircraft.isSelected) {
            boundingBox.InflateRect(2, 2);
            FrameRect(hdc, boundingBox, brush);
        }

        previousLabelTop -= AMAN_TAG_VERTICAL_SEPARATION;

        SetTextColor(hdc, oldTextColor);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
    }
}

CRect AmanTimelineView::drawMultiColorText(HDC hdc, CPoint pt, std::vector<TextSegment> texts, bool vertical) {
    CRect startRect;
    startRect.MoveToXY(pt);
    COLORREF oldColor = GetTextColor(hdc);
    COLORREF prevColor = oldColor;
    CRect accRect = { pt.x, pt.y, pt.x, pt.y };

    for each (auto item in texts) {
        std::stringstream ss;
        ss << (item.rightAligned ? std::right : std::left) << std::setw(item.width) << item.text;

        std::string outputText = ss.str();

        if (prevColor != item.color) {
            oldColor = SetTextColor(hdc, item.color);
            prevColor = item.color;
        }
        DrawText(hdc, outputText.c_str(), outputText.length(), &startRect, DT_LEFT | DT_CALCRECT);
        DrawText(hdc, outputText.c_str(), outputText.length(), &startRect, DT_LEFT);

        if (vertical) {
            accRect.right = max(accRect.right, startRect.right);
            accRect.bottom += startRect.Height();
            startRect.MoveToY(startRect.bottom);
        } else {
            accRect.right += startRect.Width();
            accRect.bottom = max(accRect.bottom, startRect.bottom);
            startRect.MoveToX(startRect.right);
        }
    }

    if (oldColor != prevColor) {
        SetTextColor(hdc, oldColor);
    }

    return accRect;
}

void AmanTimelineView::drawViafixColorLegend(HDC hdc, std::shared_ptr<AmanTimeline> timeline, CPoint position) {
    if (!timeline->getViaFixes().empty()) {
        SelectObject(hdc, AMAN_LEGEND_FONT);
        CRect startRect;
        startRect.MoveToXY(position);

        for (int i = 0; i < timeline->getViaFixes().size(); i++) {
            auto viafix = "  " + timeline->getViaFixes().at(i) + " ";
            DrawText(hdc, viafix.c_str(), viafix.length(), &startRect, DT_LEFT | DT_CALCRECT);
            DrawText(hdc, viafix.c_str(), viafix.length(), &startRect, DT_LEFT);
            SelectObject(hdc, VIA_FIX_PENS[i]);
            MoveToEx(hdc, startRect.left + 5, startRect.top + 3, NULL);
            LineTo(hdc, startRect.left + 5, startRect.bottom - 5);
            startRect.MoveToY(startRect.bottom);
        }
    }
}

std::string AmanTimelineView::formatTime(uint32_t totalSeconds, bool minutesOnly = false) {
    if (minutesOnly) {
        int minutesRounded = round((float)totalSeconds / 60.0f);
        return std::to_string(minutesRounded);
    } else {
        int minutes = totalSeconds / 60;
        auto minutesString = std::to_string(minutes);
        auto secondsString = std::to_string(totalSeconds - minutes * 60);
        std::string mm = std::string(2 - minutesString.length(), '0') + minutesString;
        std::string ss = std::string(2 - secondsString.length(), '0') + secondsString;
        return mm + ":" + ss;
    }
}

std::vector<AmanTimelineView::TextSegment> AmanTimelineView::generateLabel(AmanAircraft aircraft, std::vector<std::shared_ptr<TagItem>> tagItems, COLORREF defaultColor) {
    int remainingDistance = round(aircraft.distLeft);

    bool hasKnownViaFix = aircraft.viaFixIndex > -1 && aircraft.viaFixIndex < N_VIA_FIX_COLORS;

    std::vector<TextSegment> segments;
    std::transform(tagItems.begin(), tagItems.end(), std::back_inserter(segments), [&](std::shared_ptr<TagItem> tagItem) -> TextSegment {
        std::string sourceId = tagItem->getSource();
        std::string displayValue;

        bool timeRelevant = aircraft.secondsBehindPreceeding > 0;

        if (sourceId == "callsign") displayValue = aircraft.callsign;
        else if (sourceId == "assignedRunway") displayValue = aircraft.arrivalRunway;
        else if (sourceId == "aircraftType") displayValue = aircraft.icaoType;
        else if (sourceId == "aircraftWtc") displayValue = { aircraft.wtc };
        else if (sourceId == "minutesBehindPreceedingRounded") displayValue = timeRelevant ? formatTime(aircraft.secondsBehindPreceeding, true) : "";
        else if (sourceId == "timeBehindPreceeding") displayValue = timeRelevant ? formatTime(aircraft.secondsBehindPreceeding) : "";
        else if (sourceId == "remainingDistance") displayValue = std::to_string(remainingDistance);
        else if (sourceId == "directRouting") displayValue = aircraft.nextFix.size() > 0 ? aircraft.nextFix : tagItem->getDefaultValue();
        else if (sourceId == "scratchPad") displayValue = aircraft.scratchPad;
        else if (sourceId == "static") displayValue = tagItem->getDefaultValue();
        else displayValue = "?";

        uint32_t maxWidth = tagItem->getWidth();
        if (displayValue.length() > maxWidth) {
            displayValue = displayValue.substr(0, maxWidth - 1) + "…";
        }
         
        COLORREF textColor = hasKnownViaFix && tagItem->getIsViaFixIndicator() ? VIA_FIX_COLORS[aircraft.viaFixIndex] : defaultColor;
        return { tagItem->getWidth(), tagItem->isRightAligned(), textColor, displayValue };
    });
    
    return segments;
}
