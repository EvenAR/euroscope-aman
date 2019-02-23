#include "stdafx.h"
#include "AmanTimeline.h"

#include <ctime>
#include <sstream>
#include <iomanip>

AmanTimeline::AmanTimeline(std::string fix, int seconds, int resolution) {
	this->dual = false;
	this->seconds = seconds;
	this->resolution = resolution;

	this->fixes[0] = fix;
	this->fixes[1] = "";
};

AmanTimeline::AmanTimeline(std::string fixLeft, std::string fixRight, int seconds, int resolution) {
	this->dual = true;
	this->seconds = seconds;
	this->resolution = resolution;

	this->fixes[0] = fixLeft;
	this->fixes[1] = fixRight;
};

void AmanTimeline::render(RECT clinetRect, HDC memdc, int column) {
	// Timeline bar
	long int now = static_cast<long int> (std::time(nullptr));			// Current UNIX-timestamp in seconds
	int left = column * AMAN_WIDTH;
	int separatorSpace = this->resolution * 60;							// Number of seconds between each bar on the timeline
	RECT futureRect = { left, 0, left + AMAN_TIMELINE_WIDTH, clinetRect.bottom };
	FillRect(memdc, &futureRect, AMAN_BRUSH_TIMELINE_AHEAD);
	RECT pastRect = { left, clinetRect.bottom - AMAN_TIMELINE_REALTIME_OFFSET, left + AMAN_TIMELINE_WIDTH, clinetRect.bottom };
	FillRect(memdc, &pastRect, AMAN_BRUSH_TIMELINE_PAST);

	// Window size-dependent calculations
	int top = clinetRect.top;											// Top of timeline (future) in pixels
	int bottom = clinetRect.bottom - AMAN_TIMELINE_REALTIME_OFFSET;		// Bottom of timeline (now) in pixels

	double pixelsPerSec = (float)(bottom - top) / (float)this->seconds;
	int secToNextMin = separatorSpace - (now % separatorSpace);
	COLORREF oldTextColor = SetTextColor(memdc, RGB(255, 255, 255));

	(HFONT)SelectObject(memdc, AMAN_TIME_FONT);
	SetBkMode(memdc, TRANSPARENT);

	// Render the bars and times
	SelectObject(memdc, AMAN_WHITE_PEN);
	for (int sec = secToNextMin; sec < this->seconds; sec += separatorSpace) {
		int linePos = bottom - sec * pixelsPerSec;
		int lineTime = now + sec;
		int hours = (lineTime / 60 / 60) % 24;
		int minutes = (lineTime / 60) % 60;


		bool showTime = false;
		std::stringstream timeStr;
		if (minutes % 10 == 0) {
			MoveToEx(memdc, left + AMAN_TIMELINE_WIDTH, linePos, NULL);
			LineTo(memdc, left + AMAN_TIMELINE_WIDTH - 10, linePos);
			showTime = true;
			timeStr << std::setfill('0') << std::setw(2) << hours << ":";

			if (this->dual) {
				MoveToEx(memdc, left, linePos, NULL);
				LineTo(memdc, left + 10, linePos);
			}
		}
		else if (minutes % this->resolution == 0) {
			MoveToEx(memdc, left + AMAN_TIMELINE_WIDTH, linePos, NULL);
			LineTo(memdc, left + AMAN_TIMELINE_WIDTH - 5, linePos);
			showTime = true;

			if (this->dual) {
				MoveToEx(memdc, left, linePos, NULL);
				LineTo(memdc, left + 5, linePos);
			}
		}
		timeStr << std::setfill('0') << std::setw(2) << minutes;

		if (showTime) {
			RECT rect = { left, linePos - 6, left + AMAN_TIMELINE_WIDTH, linePos + 6 };
			DrawText(memdc, timeStr.str().c_str(), strlen(timeStr.str().c_str()), &rect, DT_CENTER);
		}

	}

	// Draw aircraft
	(HFONT)SelectObject(memdc, AMAN_LABEL_FONT);
	drawAircraftChain(memdc, left, bottom, pixelsPerSec, false, this->aircraftLists[0]);
	if (this->dual) {
		drawAircraftChain(memdc, left, bottom, pixelsPerSec, true, this->aircraftLists[1]);
	}
	
	// Draw the fix id
	SetBkMode(memdc, OPAQUE);
	SetTextColor(memdc, oldTextColor);
	RECT rect = { left - AMAN_TIMELINE_WIDTH, clinetRect.bottom - 20, left + 2*AMAN_TIMELINE_WIDTH, clinetRect.bottom };
	std::string text = this->dual ? this->fixes[1] + "/" + this->fixes[0] : this->fixes[0];
	DrawText(memdc, text.c_str(), text.length(), &rect, DT_CENTER);
}

void AmanTimeline::drawAircraftChain(HDC hdc, int xStart, int yStart, float pixelsPerSec, bool left, std::vector<AmanAircraft> aircraftList) {
	for (int ac = 0; ac < aircraftList.size(); ac++) {
		AmanAircraft aircraft = aircraftList.at(ac);
		int acPos = yStart - aircraft.eta * pixelsPerSec;
		RECT rect;

		std::stringstream acStr;
		acStr
			<< std::left << std::setfill(' ') << std::setw(4)
			<< aircraft.arrivalRunway
			<< std::left << std::setfill(' ') << std::setw(9)
			<< aircraft.callsign
			<< std::left << std::setfill(' ') << std::setw(7)
			<< this->fixes[left ? 1 : 0].c_str()
			<< std::left << std::setfill(' ') << std::setw(5)
			<< round(aircraft.distLeft)
			<< std::left << std::setfill(' ') << std::setw(5)
			<< aircraft.eta;

		if (left) {
			Ellipse(hdc, xStart - 3, acPos - 3, xStart + 3, acPos + 3);
			rect = { xStart - AMAN_WIDTH, acPos - 10, xStart - 15, acPos + 10 };
		}
		else {
			Ellipse(hdc, xStart + AMAN_TIMELINE_WIDTH - 3, acPos - 3, xStart + AMAN_TIMELINE_WIDTH + 3, acPos + 3);
			rect = { xStart + AMAN_TIMELINE_WIDTH + 15, acPos - 10, xStart + AMAN_TIMELINE_WIDTH + 15 + AMAN_WIDTH, acPos + 10 };
		}
		DrawText(hdc, acStr.str().c_str(), strlen(acStr.str().c_str()), &rect, left ? DT_RIGHT : DT_LEFT);
	}
}