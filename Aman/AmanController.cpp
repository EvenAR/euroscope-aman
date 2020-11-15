#include "stdafx.h"

#include "AmanController.h"
#include "AmanPlugIn.h"
#include "AmanTimeline.h"
#include "AmanTimelineView.h"
#include "AmanWindow.h"
#include "TitleBar.h"

#define THREE_HOURS 10800
#define FIVE_MINUTES 300

AmanController::AmanController(AmanPlugIn* plugin) {
    this->amanPlugin = plugin;
    this->amanWindow = NULL;

    this->titleBar = new TitleBar();

    this->titleBar->on("COLLAPSE_CLICKED", [&]() {
        if (this->amanWindow->isExpanded()) {
            this->amanWindow->collapse();
        } else {
            this->amanWindow->expand();
        }
    });

    this->titleBar->on("MOUSE_PRESSED", [&]() { this->moveWindow = true; });

    this->titleBar->on("RESIZE_PRESSED", [&]() {
        if (this->amanWindow->isExpanded()) {
            this->doResize = true;
        }
        });
}

void AmanController::openWindow() {
    if (this->amanWindow == NULL) {
        this->amanWindow = new AmanWindow(this, titleBar);
    }
}

void AmanController::dataUpdated(const std::vector<AmanTimeline*>& timelines) {
    if (this->amanWindow != NULL) {
        this->amanWindow->update(timelines);
    }
}

void AmanController::mousePressed(CPoint cursorPosition) { titleBar->mousePressed(cursorPosition); }

void AmanController::mouseReleased(CPoint cursorPosition) {
    this->moveWindow = false;
    this->doResize = false;
}

void AmanController::mouseMoved(CPoint cursorPosition) {
    if (this->doResize) {
        CPoint diff = cursorPosition - this->previousMousePosition;
        this->amanWindow->resizeWindowBy(diff);
    } else if (this->moveWindow && this->amanWindow != NULL) {
        CPoint diff = cursorPosition - this->previousMousePosition;
        this->amanWindow->moveWindowBy(diff);
    }

    this->previousMousePosition = cursorPosition;
}

void AmanController::mouseWheelSrolled(CPoint cursorPosition, short delta) {
    auto& allTimelines = this->amanPlugin->getTimelines();
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

void AmanController::windowClosed() { this->amanWindow = NULL; }

AmanController::~AmanController() {
    if (this->amanWindow != NULL) {
        delete this->amanWindow;
        this->amanWindow = NULL;
    }
}
