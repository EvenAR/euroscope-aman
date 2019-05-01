#include "stdafx.h"
#include "AmanController.h"
#include "AmanWindow.h"

AmanController::AmanController(AmanPlugIn* plugin) {
	this->amanPlugin = plugin;
	this->amanWindow = NULL;
}

void AmanController::openWindow() {
	if (this->amanWindow == NULL) {
		this->amanWindow = new AmanWindow(this);
	}
	else {

	}
}

void AmanController::timelinesUpdated(std::vector<AmanTimeline>* timelines) {
	if (this->amanWindow != NULL) {
		this->amanWindow->render(timelines);
	}
}

void AmanController::requestRepaint() {
	if (this->amanWindow != NULL) {
		this->amanWindow->render(NULL);
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

void AmanController::resizebuttonPressed() {
	this->doResize = true;
}

void AmanController::closeButtonPressed() {
	this->amanWindow->minimize();
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
