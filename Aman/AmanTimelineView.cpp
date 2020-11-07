#include "stdafx.h"
#include "AmanTimelineView.h"
#include "AmanAircraft.h"
#include "AmanTimeline.h"

#include "Constants.h"

#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <tuple>

CRect AmanTimelineView::getArea(AmanTimeline* timeline, CRect clientRect, int xOffset) {
	int totalWidth;

	if (timeline->isDual()) {
		totalWidth = AMAN_WIDTH * 2 - AMAN_TIMELINE_WIDTH;
	}
	else {
		totalWidth = AMAN_WIDTH;
	}

	return CRect(
		xOffset,
		AMAN_TITLEBAR_HEIGHT, 
		xOffset + totalWidth,
		clientRect.bottom
	);
}

CRect AmanTimelineView::render(AmanTimeline* timeline, CRect clientRect, HDC hdc, int offset) {
	CRect myTotalArea = getArea(timeline, clientRect, offset);

	long int now = static_cast<long int> (std::time(nullptr));			// Current UNIX-timestamp in seconds
	int minutesNow = (now / 60 + 1);									// UNIX time in minutes

	int timelineStartX = myTotalArea.left;
	if (timeline->isDual()) {
		timelineStartX += AMAN_WIDTH - AMAN_TIMELINE_WIDTH;
	}

	// Window size-dependent calculations
	int topHeight = myTotalArea.top;												// Top of timeline (future) in pixels
	int bottomHeight = myTotalArea.bottom - AMAN_TIMELINE_REALTIME_OFFSET;		// Bottom of timeline (now) in pixels
	double pixelsPerSec = (float)(bottomHeight - topHeight) / (float)timeline->getRange();
	double pixelsPerMin = 60.0 * pixelsPerSec;

	// Timeline bar
	int secToNextMin = 60 - (now % 60);

	CRect futureBackground = { timelineStartX, 0, timelineStartX + AMAN_TIMELINE_WIDTH, myTotalArea.bottom };
	FillRect(hdc, &futureBackground, AMAN_BRUSH_TIMELINE_AHEAD);
	CRect pastBackground = { timelineStartX, myTotalArea.bottom - AMAN_TIMELINE_REALTIME_OFFSET, timelineStartX + AMAN_TIMELINE_WIDTH, myTotalArea.bottom };
	FillRect(hdc, &pastBackground, AMAN_BRUSH_TIMELINE_PAST);
	
	// Vertical white border(s)
	HPEN oldPen = (HPEN)SelectObject(hdc, AMAN_VERTICAL_LINE_PEN);
	MoveToEx(hdc, timelineStartX + AMAN_TIMELINE_WIDTH, myTotalArea.bottom, NULL);
	LineTo(hdc, timelineStartX + AMAN_TIMELINE_WIDTH, myTotalArea.top);
	if (timeline->isDual()) {
		MoveToEx(hdc, timelineStartX, myTotalArea.bottom, NULL);
		LineTo(hdc, timelineStartX, myTotalArea.top);
	}

	// Render minute ticks with times
	HFONT oldFont = (HFONT)SelectObject(hdc, AMAN_TIME_FONT);
	COLORREF oldColor = SetTextColor(hdc, AMAN_COLOR_TIME_TEXT);
	int oldBackground = SetBkMode(hdc, TRANSPARENT);
	int nextMinutePosition = bottomHeight - secToNextMin * pixelsPerSec;
	
	int nextMinute = minutesNow % 60 ;		// Clock minute time

	for (int min = 0; min < timeline->getRange()/60; min++) {
		int linePos = nextMinutePosition - (min * pixelsPerMin);
		int minAtLine = (nextMinute + min) % 60;
		

		int tickLength = 4;
		if (minAtLine % 5 == 0) {
			tickLength = 8;
		}
		std::stringstream timeSs;
		if (minAtLine % 10 == 0) {
			// 10 minute label
			int hoursAtLine = ((minutesNow + min) / 60) % 24;
			timeSs << std::setfill('0') << std::setw(2) << hoursAtLine << ":" << std::setw(2) << minAtLine;
		} else if (minAtLine % 5 == 0 || pixelsPerMin > 15) {
			// 5 and 1 minute label
			timeSs << std::setfill('0') << std::setw(2) << minAtLine;
		}

		auto timeString = timeSs.str();
		if (!timeString.empty()) {
			CRect rect = { timelineStartX, linePos - 6, timelineStartX + AMAN_TIMELINE_WIDTH, linePos + 6 };
			DrawText(hdc, timeString.c_str(), strlen(timeString.c_str()), &rect, DT_CENTER);
		}

		MoveToEx(hdc, timelineStartX + AMAN_TIMELINE_WIDTH, linePos, NULL);
		LineTo(hdc, timelineStartX + AMAN_TIMELINE_WIDTH - tickLength, linePos);
		if (timeline->isDual()) {
			MoveToEx(hdc, timelineStartX, linePos, NULL);
			LineTo(hdc, timelineStartX + tickLength, linePos);
		}
	}

	// Draw aircraft
	SelectObject(hdc, AMAN_LABEL_FONT);
	
	if (timeline->isDual()) {
		auto fixes = timeline->getFixes();
		auto inboundsLeft = timeline->getAircraftList({ fixes[0] });
		auto inboundsRight = timeline->getAircraftList({ fixes[1] });
		drawAircraftChain(hdc, now, timelineStartX, bottomHeight, pixelsPerSec, true, inboundsLeft);
		drawAircraftChain(hdc, now, timelineStartX, bottomHeight, pixelsPerSec, false, inboundsRight);
	}
	else {
		drawAircraftChain(hdc, now, timelineStartX, bottomHeight, pixelsPerSec, false, *timeline->getAircraftList());
	}
	
	// Draw the fix id
	COLORREF oldBackgroundColor = SetBkColor(hdc, AMAN_COLOR_FIX_BACKGROUND);
	SetBkMode(hdc, OPAQUE);
	SetTextColor(hdc, AMAN_COLOR_FIX_TEXT);
	SelectObject(hdc, AMAN_FIX_FONT);
	CRect rect = { timelineStartX - AMAN_TIMELINE_WIDTH, myTotalArea.bottom - 20, timelineStartX + 2*AMAN_TIMELINE_WIDTH, myTotalArea.bottom };
	std::string text = timeline->getIdentifier();
	DrawText(hdc, text.c_str(), text.length(), &rect, DT_CENTER);

	// Draw color legend
	SelectObject(hdc, AMAN_LEGEND_FONT);
	std::vector<std::tuple<int, bool, COLORREF, std::string>> legendEntries;
	for (int i = 0; i < timeline->getViaFixes().size(); i++) {
		legendEntries.push_back({ 6, false, VIA_FIX_COLORS[i], timeline->getViaFixes().at(i) });
	}
	drawMultiColorTextLine(hdc, 
		{
			myTotalArea.left + (timeline->isDual() ? 0 : AMAN_TIMELINE_WIDTH + 1),
			myTotalArea.top,
			myTotalArea.right, 
			myTotalArea.bottom 
		}, 
		legendEntries
	);

	// Restore settings
	SetTextColor(hdc, oldColor);
	SelectObject(hdc, oldFont);
	SelectObject(hdc, oldPen);
	SetBkMode(hdc, oldBackground);
	SetBkColor(hdc, oldBackgroundColor);

	return myTotalArea;
}

void AmanTimelineView::drawAircraftChain(HDC hdc, int timeNow, int xStart, int yStart, float pixelsPerSec, bool left, std::vector<AmanAircraft> aircraftList) {
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

		COLORREF callsignColor;

		if (aircraft.viaFixIndex > -1 && aircraft.viaFixIndex < N_VIA_FIX_COLORS) {
			callsignColor = VIA_FIX_COLORS[aircraft.viaFixIndex];
		}
		else {
			callsignColor = defaultColor;
		}

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

			Ellipse(hdc, xStart - AMAN_DOT_RADIUS, acPos - AMAN_DOT_RADIUS, xStart + AMAN_DOT_RADIUS, acPos + AMAN_DOT_RADIUS);
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

			Ellipse(hdc, xStart + AMAN_TIMELINE_WIDTH - AMAN_DOT_RADIUS, acPos - AMAN_DOT_RADIUS, xStart + AMAN_TIMELINE_WIDTH + AMAN_DOT_RADIUS, acPos + AMAN_DOT_RADIUS);
			rect = { rectLeft, rectTop, rectRight, rectBottom };

			MoveToEx(hdc, xStart + AMAN_TIMELINE_WIDTH, acPos, NULL);
			LineTo(hdc, rect.left, rect.top + AMAN_AIRCRAFT_LINE_HEIGHT / 2);

			prevTop = rectTop;
		}

		const char* nextFix = "-----";
		if (strlen(aircraft.nextFix.c_str()) > 0) {
			nextFix = aircraft.nextFix.c_str();
		}

		int minutesBehindPreceeding = round(aircraft.timeToNextAircraft / 60);
		int remainingDistance = round(aircraft.distLeft);

		drawMultiColorTextLine(hdc, rect, {
			{ 4, false, defaultColor, aircraft.arrivalRunway },
			{ 9, false, callsignColor, aircraft.callsign },
			{ 5, false, defaultColor, aircraft.icaoType },
			{ 2, false, defaultColor, { aircraft.wtc } },
			{ 7, false, defaultColor, nextFix },
			{ 3, false, defaultColor, std::to_string(minutesBehindPreceeding) },
			{ 4, true, defaultColor, std::to_string(remainingDistance) }
		});

		if (aircraft.isSelected) {
			rect.InflateRect(2, 2);
			FrameRect(hdc, rect, brush);
		}

		prevTop -= AMAN_AIRCRAFT_LINE_SEPARATION;
		SetTextColor(hdc, oldTextColor);
	}
}

void AmanTimelineView::drawMultiColorTextLine(HDC hdc, CRect rect, std::vector<std::tuple<int, bool, COLORREF, std::string>> texts) {
	for each (auto item in texts) {
		auto width = std::get<0>(item);
		auto rightAligned = std::get<1>(item);
		auto color = std::get<2>(item);
		auto text = std::get<3>(item);

		std::stringstream ss;
		ss << (rightAligned ? std::right : std::left) << std::setw(width) << text;

		std::string outputText = ss.str();

		SetTextColor(hdc, color);
		DrawText(hdc, outputText.c_str(), outputText.length(), &rect, DT_LEFT | DT_CALCRECT);
		DrawText(hdc, outputText.c_str(), outputText.length(), &rect, DT_LEFT);
		rect.MoveToX(rect.right);
	}
}