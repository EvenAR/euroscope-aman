#include "stdafx.h"
#include "AmanController.h"
#include "AmanWindow.h"
#include "AmanPlugIn.h"
#include "AmanTimeline.h"
#include "AmanTimelineView.h"

#define THREE_HOURS 10800
#define FIVE_MINUTES 300

AmanController::AmanController(AmanPlugIn* plugin) {
	this->amanPlugin = plugin;
	this->amanWindow = NULL;
}

void AmanController::openWindow() {
	if (this->amanWindow == NULL) {
		this->amanWindow = new AmanWindow(this);
	}
}

void AmanController::dataUpdated(std::vector<AmanTimeline*>* timelines) {
	if (this->amanWindow != NULL) {
		this->amanWindow->update(timelines);
	}
}

void AmanController::mousePressed(CRect windowRect, CPoint cursorPosition) {
	this->mouseDownPosition = cursorPosition;
}

void AmanController::mouseReleased(CRect windowRect, CPoint cursorPosition) {
	this->moveWindow = false;
	this->doResize = false;
}

void AmanController::mouseMoved(CRect windowRect, CPoint cursorPosition) {
	if (this->doResize) {
		CPoint diff = cursorPosition - this->previousMousePosition;

		CRect newRect = {
			windowRect.left, 
			windowRect.top + diff.y,
			windowRect.right + diff.x,
			windowRect.bottom 
		};

		this->amanWindow->setWindowPosition(newRect);

	} else if (this->moveWindow && this->amanWindow != NULL) {
		CPoint diff = cursorPosition - this->previousMousePosition;
		windowRect.MoveToXY(windowRect.TopLeft() + diff);
		this->amanWindow->setWindowPosition(windowRect);
	}
	this->previousMousePosition = cursorPosition;
}

void AmanController::mouseWheelSrolled(CPoint cursorPosition, short delta) {
	auto allTimelines = this->amanPlugin->getTimelines();
	auto timelinePointedAt = this->amanWindow->getTimelineAt(allTimelines, cursorPosition);
	if (timelinePointedAt) {
		auto currentRange = timelinePointedAt->getRange();
		auto newRange = currentRange - delta;
		auto limitReached = newRange < FIVE_MINUTES || newRange > THREE_HOURS;

		if (!limitReached) {
			timelinePointedAt->setRange(newRange);
			dataUpdated(allTimelines);
		}
	}
}

void AmanController::resizebuttonPressed() {
	if (this->expanded) {
		this->doResize = true;
	}
}

void AmanController::collapseButtonPressed() {
	if (this->expanded) {
		this->amanWindow->collapse();
	}
	else {
		this->amanWindow->expand();
	}
	this->expanded = !this->expanded;
}

void AmanController::titleBarPressed() {
	this->moveWindow = true;
}

void AmanController::windowClosed() {
	this->amanWindow = NULL;
}

AmanController::~AmanController() {
	if (this->amanWindow != NULL) {
		delete this->amanWindow;
		this->amanWindow = NULL;
	}
}
