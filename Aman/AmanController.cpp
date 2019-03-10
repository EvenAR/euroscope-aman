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

void AmanController::windowClosed() {
	this->amanWindow = NULL;
}

AmanController::~AmanController() {
	if (this->amanWindow != NULL) {
		delete this->amanWindow;
		this->amanWindow = NULL;
	}
}
