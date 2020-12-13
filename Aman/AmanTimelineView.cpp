#include "stdafx.h"

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <tuple>

#include "AmanAircraft.h"
#include "AmanTimeline.h"
#include "AmanTimelineView.h"
#include "Constants.h"

CRect AmanTimelineView::getArea(AmanTimeline* timeline, CRect clientRect, int xOffset) {
    int totalWidth;

    if (timeline->isDual()) {
        totalWidth = AMAN_WIDTH * 2 - AMAN_TIMELINE_WIDTH;
    } else {
        totalWidth = AMAN_WIDTH;
    }

    return CRect(xOffset, AMAN_TITLEBAR_HEIGHT, xOffset + totalWidth, clientRect.bottom);
}

CRect AmanTimelineView::render(AmanTimeline* timeline, CRect clientRect, HDC hdc, int xOffset) {
    CRect myTotalArea = getArea(timeline, clientRect, xOffset);

    long int unixTimestamp = static_cast<long int>(std::time(nullptr)); // Current UNIX-unixTimestamp in seconds
    int unixTimestampAsMinutes = (unixTimestamp / 60 + 1);              // UNIX time in minutes

    int timelineStartX = myTotalArea.left;
    if (timeline->isDual()) {
        timelineStartX += AMAN_WIDTH - AMAN_TIMELINE_WIDTH;
    }

    int presentTimeStartY = myTotalArea.bottom - AMAN_TIMELINE_REALTIME_OFFSET;
    double pixelsPerSecY = (float)(presentTimeStartY - myTotalArea.top) / (float)timeline->getRange();
    double pixelsPerMinuteY = 60.0 * pixelsPerSecY;

    // Timeline bar
    int secToNextMin = 60 - (unixTimestamp % 60);

    CRect futureBackground = { timelineStartX, 0, timelineStartX + AMAN_TIMELINE_WIDTH, myTotalArea.bottom };
    FillRect(hdc, &futureBackground, AMAN_BRUSH_TIMELINE_AHEAD);
    CRect pastBackground = { timelineStartX, myTotalArea.bottom - AMAN_TIMELINE_REALTIME_OFFSET,
                            timelineStartX + AMAN_TIMELINE_WIDTH, myTotalArea.bottom };
    FillRect(hdc, &pastBackground, AMAN_BRUSH_TIMELINE_PAST);

    // Vertical white border(s)
    HPEN oldPen = (HPEN)SelectObject(hdc, AMAN_VERTICAL_LINE_PEN);
    MoveToEx(hdc, timelineStartX + AMAN_TIMELINE_WIDTH, myTotalArea.bottom, NULL);
    LineTo(hdc, timelineStartX + AMAN_TIMELINE_WIDTH, myTotalArea.top);
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
    int futureMinutesToDisplay = floor(timeline->getRange() / 60);

    for (int minute = -pastMinutesToDisplay; minute < futureMinutesToDisplay; minute++) {
        int tickPosY = firstMinutePositionY - (minute * pixelsPerMinuteY);
        int minutesAtTick = (firstMinute + minute) % 60;

        std::stringstream timeSs;
        if (minutesAtTick % 10 == 0) {
            // 10 minute label
            int hoursAtLine = ((unixTimestampAsMinutes + minute) / 60) % 24;
            timeSs << std::setfill('0') << std::setw(2) << hoursAtLine << ":" << std::setw(2) << minutesAtTick;
        } else if (minutesAtTick % 5 == 0 || pixelsPerMinuteY > AMAN_TIMELINE_MAX_TICK_DENSITY) {
            // 5 and 1 minute label
            timeSs << std::setfill('0') << std::setw(2) << minutesAtTick;
        }

        auto timeString = timeSs.str();
        if (!timeString.empty()) {
            CRect rect = { timelineStartX, tickPosY - 6, timelineStartX + AMAN_TIMELINE_WIDTH, tickPosY + 6 };
            DrawText(hdc, timeString.c_str(), strlen(timeString.c_str()), &rect, DT_CENTER);
        }

        int tickLength = minutesAtTick % 5 == 0 ? AMAN_TIMELINE_TICK_WIDTH_5_MIN : AMAN_TIMELINE_TICK_WIDTH_1_MIN;

        MoveToEx(hdc, timelineStartX + AMAN_TIMELINE_WIDTH, tickPosY, NULL);
        LineTo(hdc, timelineStartX + AMAN_TIMELINE_WIDTH - tickLength, tickPosY);
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
    drawHourglass(timelineStartX + AMAN_TIMELINE_WIDTH, presentTimeStartY);
    if (timeline->isDual()) {
        drawHourglass(timelineStartX, presentTimeStartY);
    }

    // Draw aircraft
    SelectObject(hdc, AMAN_LABEL_FONT);

    if (timeline->isDual()) {
        auto fixes = timeline->getFixes();
        auto inboundsLeft = timeline->getAircraftList({ fixes[0] });
        auto inboundsRight = timeline->getAircraftList({ fixes[1] });
        drawAircraftChain(hdc, unixTimestamp, timelineStartX, presentTimeStartY, pixelsPerSecY, true, inboundsLeft);
        drawAircraftChain(hdc, unixTimestamp, timelineStartX, presentTimeStartY, pixelsPerSecY, false, inboundsRight);
    } else {
        drawAircraftChain(hdc, unixTimestamp, timelineStartX, presentTimeStartY, pixelsPerSecY, false,
            *timeline->getAircraftList());
    }

    // Draw the fix id
    COLORREF oldBackgroundColor = SetBkColor(hdc, AMAN_COLOR_FIX_BACKGROUND);
    SetBkMode(hdc, OPAQUE);
    SetTextColor(hdc, AMAN_COLOR_FIX_TEXT);
    SelectObject(hdc, AMAN_FIX_FONT);
    CRect rect = { timelineStartX - AMAN_TIMELINE_WIDTH, myTotalArea.bottom - 20,
                  timelineStartX + 2 * AMAN_TIMELINE_WIDTH, myTotalArea.bottom };
    std::string text = timeline->getIdentifier();
    DrawText(hdc, text.c_str(), text.length(), &rect, DT_CENTER);

    // Draw color legend
    SelectObject(hdc, AMAN_LEGEND_FONT);
    std::vector<std::tuple<int, bool, COLORREF, std::string>> legendEntries;
    for (int i = 0; i < timeline->getViaFixes().size(); i++) {
        legendEntries.push_back({ 6, false, VIA_FIX_COLORS[i], timeline->getViaFixes().at(i) });
    }
    drawMultiColorTextLine(hdc,
        { myTotalArea.left + (timeline->isDual() ? 0 : AMAN_TIMELINE_WIDTH + 1), myTotalArea.top,
         myTotalArea.right, myTotalArea.bottom },
        legendEntries);

    // Restore settings
    SetTextColor(hdc, oldColor);
    SelectObject(hdc, oldFont);
    SelectObject(hdc, oldPen);
    SetBkMode(hdc, oldBackground);
    SetBkColor(hdc, oldBackgroundColor);

    return myTotalArea;
}

void AmanTimelineView::drawAircraftChain(HDC hdc, int timeNow, int xStart, int yStart, float pixelsPerSec, bool left,
    std::vector<AmanAircraft> aircraftList) {
    std::sort(aircraftList.begin(), aircraftList.end());

    int prevTop = -1;
    int offset = 0;

    for (int ac = 0; ac < aircraftList.size(); ac++) {
        AmanAircraft aircraft = aircraftList.at(ac);
        int acPos = yStart - (aircraft.eta - timeNow) * pixelsPerSec;
        CRect rect;

        COLORREF oldTextColor;
        COLORREF defaultColor = aircraft.trackedByMe ? AMAN_COLOR_TRACKED : AMAN_COLOR_UNTRACKED;
        HBRUSH brush = aircraft.trackedByMe ? AMAN_TRACKED_BRUSH : AMAN_UNTRACKED_BRUSH;
        HPEN pen = aircraft.trackedByMe ? AMAN_WHITE_PEN : AMAN_GRAY_PEN;

        SelectObject(hdc, pen);
        SelectObject(hdc, brush);
        oldTextColor = SetTextColor(hdc, defaultColor);

        bool hasKnownViaFix = aircraft.viaFixIndex > -1 && aircraft.viaFixIndex < N_VIA_FIX_COLORS;
        COLORREF directRoutingColor = hasKnownViaFix ? VIA_FIX_COLORS[aircraft.viaFixIndex] : defaultColor;

        // Left side of timeline
        if (left) {
            int rectLeft = xStart - AMAN_WIDTH + AMAN_TIMELINE_WIDTH + AMAN_LABEL_SEP_FROM_TIMELINE;
            int rectTop = acPos - AMAN_AIRCRAFT_LINE_HEIGHT / 2;
            int rectRight = xStart - AMAN_LABEL_SEP_FROM_TIMELINE;
            int rectBottom = acPos + AMAN_AIRCRAFT_LINE_HEIGHT / 2;

            if (prevTop >= 0 && rectBottom > prevTop) {
                int diff = rectBottom - prevTop;
                rectTop = rectTop - diff;
                rectBottom = rectBottom - diff;
            }

            Ellipse(hdc, xStart - AMAN_DOT_RADIUS, acPos - AMAN_DOT_RADIUS, xStart + AMAN_DOT_RADIUS,
                acPos + AMAN_DOT_RADIUS);
            rect = { rectLeft, rectTop, rectRight, rectBottom };

            MoveToEx(hdc, xStart, acPos, NULL);
            LineTo(hdc, rect.right, rect.top + AMAN_AIRCRAFT_LINE_HEIGHT / 2);

            prevTop = rectTop;
        }
        // Right side of timeline
        else {
            int rectLeft = xStart + AMAN_TIMELINE_WIDTH + AMAN_LABEL_SEP_FROM_TIMELINE;
            int rectTop = acPos - AMAN_AIRCRAFT_LINE_HEIGHT / 2 + offset;
            int rectRight = xStart + AMAN_WIDTH - AMAN_LABEL_SEP_FROM_TIMELINE;
            int rectBottom = acPos + AMAN_AIRCRAFT_LINE_HEIGHT / 2 + offset;

            if (prevTop >= 0 && rectBottom > prevTop) {
                int diff = rectBottom - prevTop;
                rectTop = rectTop - diff;
                rectBottom = rectBottom - diff;
            }

            Ellipse(hdc, xStart + AMAN_TIMELINE_WIDTH - AMAN_DOT_RADIUS, acPos - AMAN_DOT_RADIUS,
                xStart + AMAN_TIMELINE_WIDTH + AMAN_DOT_RADIUS, acPos + AMAN_DOT_RADIUS);
            rect = { rectLeft, rectTop, rectRight, rectBottom };

            MoveToEx(hdc, xStart + AMAN_TIMELINE_WIDTH, acPos, NULL);
            LineTo(hdc, rect.left, rect.top + AMAN_AIRCRAFT_LINE_HEIGHT / 2);

            prevTop = rectTop;
        }

        const char* nextFix = "-----";
        if (strlen(aircraft.nextFix.c_str()) > 0) {
            nextFix = aircraft.nextFix.c_str();
        }

        int minutesBehindPreceeding = round(aircraft.secondsBehindPreceeding / 60);
        int remainingDistance = round(aircraft.distLeft);

        drawMultiColorTextLine(hdc, rect, {
            {4, false, defaultColor, aircraft.arrivalRunway},
            {9, false, defaultColor, aircraft.callsign},
            {5, false, defaultColor, aircraft.icaoType},
            {2, false, defaultColor, {aircraft.wtc}},
            {7, false, directRoutingColor, nextFix},
            {3, false, defaultColor, std::to_string(minutesBehindPreceeding)},
            {4, true, defaultColor, std::to_string(remainingDistance)}
        });

        if (aircraft.isSelected) {
            rect.InflateRect(2, 2);
            FrameRect(hdc, rect, brush);
        }

        prevTop -= AMAN_AIRCRAFT_LINE_SEPARATION;
        SetTextColor(hdc, oldTextColor);
    }
}

void AmanTimelineView::drawMultiColorTextLine(HDC hdc, CRect rect,
    std::vector<std::tuple<int, bool, COLORREF, std::string>> texts) {
    for each (auto item in texts) {
        auto& width = std::get<0>(item);
        auto& rightAligned = std::get<1>(item);
        auto& color = std::get<2>(item);
        auto& text = std::get<3>(item);

        std::stringstream ss;
        ss << (rightAligned ? std::right : std::left) << std::setw(width) << text;

        std::string outputText = ss.str();

        SetTextColor(hdc, color);
        DrawText(hdc, outputText.c_str(), outputText.length(), &rect, DT_LEFT | DT_CALCRECT);
        DrawText(hdc, outputText.c_str(), outputText.length(), &rect, DT_LEFT);
        rect.MoveToX(rect.right);
    }
}