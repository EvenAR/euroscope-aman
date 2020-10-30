#include "stdafx.h"
#include "AmanTimeline.h"
#include "AmanAircraft.h"
#include "Constants.h"

#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>

AmanTimeline::AmanTimeline(std::string fix) {
	this->dual = false;
	this->identifier = fix;

	this->fixNames[0] = fix;
	this->fixNames[1] = "";

	this->aircraftLists = new std::vector<AmanAircraft>[2];
};

AmanTimeline::AmanTimeline(std::string fixLeft, std::string fixRight) {
	this->dual = true;
	this->identifier = fixLeft + "/" + fixRight;

	this->fixNames[0] = fixRight;
	this->fixNames[1] = fixLeft;

	this->aircraftLists = new std::vector<AmanAircraft>[2];
};

CRect AmanTimeline::getArea(CRect clientRect, int xOffset) {
	int totalWidth;

	if (this->dual) {
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

void AmanTimeline::zoom(int value) {
	this->seconds = value;
}

CRect AmanTimeline::render(CRect clientRect, HDC hdc, int offset) {
	CRect myTotalArea = getArea(clientRect, offset);

	long int now = static_cast<long int> (std::time(nullptr));			// Current UNIX-timestamp in seconds
	int minutesNow = (now / 60 + 1);									// UNIX time in minutes

	int timelineStartX = myTotalArea.left;
	if (this->dual) {
		timelineStartX += AMAN_WIDTH - AMAN_TIMELINE_WIDTH;
	}

	// Window size-dependent calculations
	int topHeight = myTotalArea.top;												// Top of timeline (future) in pixels
	int bottomHeight = myTotalArea.bottom - AMAN_TIMELINE_REALTIME_OFFSET;		// Bottom of timeline (now) in pixels
	double pixelsPerSec = (float)(bottomHeight - topHeight) / (float)this->seconds;
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
	if (dual) {
		MoveToEx(hdc, timelineStartX, myTotalArea.bottom, NULL);
		LineTo(hdc, timelineStartX, myTotalArea.top);
	}

	// Render minute ticks with times
	HFONT oldFont = (HFONT)SelectObject(hdc, AMAN_TIME_FONT);
	COLORREF oldColor = SetTextColor(hdc, AMAN_COLOR_TIME_TEXT);
	int oldBackground = SetBkMode(hdc, TRANSPARENT);
	int nextMinutePosition = bottomHeight - secToNextMin * pixelsPerSec;
	
	int nextMinute = minutesNow % 60 ;		// Clock minute time

	for (int min = 0; min < this->seconds/60; min++) {
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
		if (this->dual) {
			MoveToEx(hdc, timelineStartX, linePos, NULL);
			LineTo(hdc, timelineStartX + tickLength, linePos);
		}
	}

	// Draw aircraft
	SelectObject(hdc, AMAN_LABEL_FONT);
	drawAircraftChain(hdc, now, timelineStartX, bottomHeight, pixelsPerSec, false, this->aircraftLists[0]);
	if (this->dual) {
		drawAircraftChain(hdc, now, timelineStartX, bottomHeight, pixelsPerSec, true, this->aircraftLists[1]);
	}
	
	// Draw the fix id
	COLORREF oldBackgroundColor = SetBkColor(hdc, AMAN_COLOR_FIX_BACKGROUND);
	SetBkMode(hdc, OPAQUE);
	SetTextColor(hdc, AMAN_COLOR_FIX_TEXT);
	SelectObject(hdc, AMAN_FIX_FONT);
	CRect rect = { timelineStartX - AMAN_TIMELINE_WIDTH, myTotalArea.bottom - 20, timelineStartX + 2*AMAN_TIMELINE_WIDTH, myTotalArea.bottom };
	std::string text = this->dual ? this->fixNames[1] + "/" + this->fixNames[0] : this->fixNames[0];
	DrawText(hdc, text.c_str(), text.length(), &rect, DT_CENTER);

	// Restore settings
	SetTextColor(hdc, oldColor);
	SelectObject(hdc, oldFont);
	SelectObject(hdc, oldPen);
	SetBkMode(hdc, oldBackground);
	SetBkColor(hdc, oldBackgroundColor);

	return myTotalArea;
}

void AmanTimeline::drawAircraftChain(HDC hdc, int timeNow, int xStart, int yStart, float pixelsPerSec, bool left, std::vector<AmanAircraft> aircraftList) {
	std::sort(aircraftList.begin(), aircraftList.end());

	int prevTop = -1;
	int offset = 0;


	for (int ac = 0; ac < aircraftList.size(); ac++) {
		AmanAircraft aircraft = aircraftList.at(ac);
		int acPos = yStart - (aircraft.eta - timeNow) * pixelsPerSec;
		COLORREF oldTextColor;
		CRect rect;

		const char* nextFix = "-----";
		if (strlen(aircraft.nextFix) > 0) {
			nextFix = aircraft.nextFix;
		}

		std::stringstream acStr;
		acStr
			<< std::left << std::setfill(' ') << std::setw(4)
			<< aircraft.arrivalRunway
			<< std::left << std::setfill(' ') << std::setw(9)
			<< aircraft.callsign
			<< std::left << std::setfill(' ') << std::setw(5)
			<< aircraft.icaoType
			<< std::left << std::setfill(' ') << std::setw(2)
			<< aircraft.wtc
			<< std::left << std::setfill(' ') << std::setw(7)
			<< nextFix
			<< std::left << std::setfill(' ') << std::setw(3)
			<< round(aircraft.timeToNextAircraft / 60)
			<< std::right << std::setfill(' ') << std::setw(4)
			<< round(aircraft.distLeft);

		oldTextColor = SetTextColor(hdc, AMAN_COLOR_AIRCRAFT_LABEL);
		if (aircraft.trackedByMe) {
			SelectObject(hdc, AMAN_WHITE_PEN);
			SelectObject(hdc, AMAN_TRACKED_BRUSH);
		}
		else {
			SelectObject(hdc, AMAN_GRAY_PEN);
			SelectObject(hdc, AMAN_UNTRACKED_BRUSH);
		}

		// Left side of timeline
		if (left) {
			int rectLeft = xStart - AMAN_WIDTH;
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
			int rectRight = xStart + AMAN_TIMELINE_WIDTH + AMAN_LABEL_SEP_FROM_TIMELINE + AMAN_WIDTH;
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
		DrawText(hdc, acStr.str().c_str(), strlen(acStr.str().c_str()), &rect, left ? DT_RIGHT : DT_LEFT);
		SetTextColor(hdc, oldTextColor);
	}
}