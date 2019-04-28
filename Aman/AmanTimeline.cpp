#include "stdafx.h"
#include "AmanTimeline.h"
#include "AmanAircraft.h"
#include "Constants.h"

#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>

AmanTimeline::AmanTimeline(std::string fix, int seconds, int resolution) {
	this->dual = false;
	this->seconds = seconds;
	this->resolution = resolution;

	this->fixes[0] = fix;
	this->fixes[1] = "";

	this->aircraftLists = new std::vector<AmanAircraft>[2];
};

AmanTimeline::AmanTimeline(std::string fixLeft, std::string fixRight, int seconds, int resolution) {
	this->dual = true;
	this->seconds = seconds;
	this->resolution = resolution;

	this->fixes[0] = fixLeft;
	this->fixes[1] = fixRight;

	this->aircraftLists = new std::vector<AmanAircraft>[2];
};

void AmanTimeline::render(RECT clinetRect, HDC hdc, int column) {
	long int now = static_cast<long int> (std::time(nullptr));			// Current UNIX-timestamp in seconds
	int minutesNow = (now / 60 + 1);									// UNIX time in minutes

	int xOffset = column * AMAN_WIDTH;

	// Window size-dependent calculations
	int top = clinetRect.top;											// Top of timeline (future) in pixels
	int bottom = clinetRect.bottom - AMAN_TIMELINE_REALTIME_OFFSET;		// Bottom of timeline (now) in pixels
	double pixelsPerSec = (float)(bottom - top) / (float)this->seconds;
	double pixelsPerMin = 60.0 * pixelsPerSec;

	

	// Timeline bar
	int secToNextMin = 60 - (now % 60);

	RECT futureRect = { xOffset, 0, xOffset + AMAN_TIMELINE_WIDTH, clinetRect.bottom };
	FillRect(hdc, &futureRect, AMAN_BRUSH_TIMELINE_AHEAD);
	RECT pastRect = { xOffset, clinetRect.bottom - AMAN_TIMELINE_REALTIME_OFFSET, xOffset + AMAN_TIMELINE_WIDTH, clinetRect.bottom };
	FillRect(hdc, &pastRect, AMAN_BRUSH_TIMELINE_PAST);
	
	// Vertical white line(s)
	HPEN oldPen = (HPEN)SelectObject(hdc, AMAN_VERTICAL_LINE_PEN);
	MoveToEx(hdc, xOffset + AMAN_TIMELINE_WIDTH, clinetRect.bottom, NULL);
	LineTo(hdc, xOffset + AMAN_TIMELINE_WIDTH, clinetRect.top);
	if (dual) {
		MoveToEx(hdc, xOffset, clinetRect.bottom, NULL);
		LineTo(hdc, xOffset, clinetRect.top);
	}

	// Render the horizontal bars + times
	HFONT oldFont = (HFONT)SelectObject(hdc, AMAN_TIME_FONT);
	COLORREF oldColor = SetTextColor(hdc, AMAN_COLOR_TIME_TEXT);
	int oldBackground = SetBkMode(hdc, TRANSPARENT);
	int nextMinutePosition = bottom - secToNextMin * pixelsPerSec;
	
	int nextMinute = minutesNow % 60 ;		// Clock minute time

	for (int min = 0; min < this->seconds/60; min++) {
		int linePos = nextMinutePosition - (min * pixelsPerMin);
		int minAtLine = (nextMinute + min) % 60;
		std::stringstream timeStr;

		int tickLength = 4;
		if (minAtLine % 5 == 0) {
			tickLength = 8;
		}

		if (minAtLine % 10 == 0) {
			int hoursAtLine = ((minutesNow + min) / 60) % 24;
			timeStr << std::setfill('0') << std::setw(2) << hoursAtLine << ":" << std::setw(2) << minAtLine;
			RECT rect = { xOffset, linePos - 6, xOffset + AMAN_TIMELINE_WIDTH, linePos + 6 };
			DrawText(hdc, timeStr.str().c_str(), strlen(timeStr.str().c_str()), &rect, DT_CENTER);
		} else {
			timeStr << std::setfill('0') << std::setw(2) << minAtLine;
			RECT rect = { xOffset, linePos - 6, xOffset + AMAN_TIMELINE_WIDTH, linePos + 6 };
			DrawText(hdc, timeStr.str().c_str(), strlen(timeStr.str().c_str()), &rect, DT_CENTER);
		}

		MoveToEx(hdc, xOffset + AMAN_TIMELINE_WIDTH, linePos, NULL);
		LineTo(hdc, xOffset + AMAN_TIMELINE_WIDTH - tickLength, linePos);
		if (this->dual) {
			MoveToEx(hdc, xOffset, linePos, NULL);
			LineTo(hdc, xOffset + tickLength, linePos);
		}
	}

	// Draw aircraft
	SelectObject(hdc, AMAN_LABEL_FONT);
	drawAircraftChain(hdc, now, xOffset, bottom, pixelsPerSec, false, this->aircraftLists[0]);
	if (this->dual) {
		drawAircraftChain(hdc, now, xOffset, bottom, pixelsPerSec, true, this->aircraftLists[1]);
	}
	
	// Draw the fix id
	COLORREF oldBackgroundColor = SetBkColor(hdc, AMAN_COLOR_FIX_BACKGROUND);
	SetBkMode(hdc, OPAQUE);
	SetTextColor(hdc, AMAN_COLOR_FIX_TEXT);
	SelectObject(hdc, AMAN_FIX_FONT);
	RECT rect = { xOffset - AMAN_TIMELINE_WIDTH, clinetRect.bottom - 20, xOffset + 2*AMAN_TIMELINE_WIDTH, clinetRect.bottom };
	std::string text = this->dual ? this->fixes[1] + "/" + this->fixes[0] : this->fixes[0];
	DrawText(hdc, text.c_str(), text.length(), &rect, DT_CENTER);

	// Restore settings
	SetTextColor(hdc, oldColor);
	SelectObject(hdc, oldFont);
	SelectObject(hdc, oldPen);
	SetBkMode(hdc, oldBackground);
	SetBkColor(hdc, oldBackgroundColor);
}

void AmanTimeline::drawAircraftChain(HDC hdc, int timeNow, int xStart, int yStart, float pixelsPerSec, bool left, std::vector<AmanAircraft> aircraftList) {
	std::sort(aircraftList.begin(), aircraftList.end());

	int prevTop = -1;
	int offset = 0;


	for (int ac = 0; ac < aircraftList.size(); ac++) {
		AmanAircraft aircraft = aircraftList.at(ac);
		int acPos = yStart - (aircraft.eta - timeNow) * pixelsPerSec;
		COLORREF oldTextColor;
		RECT rect;

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
			<< std::left << std::setfill(' ') << std::setw(5)
			<< round(aircraft.timeToNextAircraft / 60);


		if (aircraft.trackedByMe) {
			oldTextColor = SetTextColor(hdc, AMAN_COLOR_TRACKED_BY_ME);
			SelectObject(hdc, AMAN_WHITE_PEN);
			SetBkColor(hdc, AMAN_COLOR_TRACKED_BY_ME);
			SelectObject(hdc, AMAN_TRACKED_BRUSH);
		}
		else {
			oldTextColor = SetTextColor(hdc, AMAN_COLOR_NOT_TRACKED_BY_ME);
			SelectObject(hdc, AMAN_GRAY_PEN);
			SetBkColor(hdc, AMAN_COLOR_NOT_TRACKED_BY_ME);
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
			LineTo(hdc, rect.left - 5, rect.top + AMAN_AIRCRAFT_LINE_HEIGHT / 2);

			prevTop = rectTop;
		}
		DrawText(hdc, acStr.str().c_str(), strlen(acStr.str().c_str()), &rect, left ? DT_RIGHT : DT_LEFT);
		SetTextColor(hdc, oldTextColor);
	}
}