#include "stdafx.h"
#include "AmanController.h"
#include "AmanPlugIn.h"
#include "AmanTimeline.h"

AmanController::AmanController(AmanPlugIn* plugin) {
	this->amanPlugin = plugin;
}

void AmanController::openWindow() {
	this->amanWindow = new AmanWindow();
}

void AmanController::timelinesUpdated(std::vector<AmanTimeline> aircraftList) {
	if (this->amanWindow) {
		this->amanWindow->render(aircraftList);
	}
}

AmanController::~AmanController()
{
}
