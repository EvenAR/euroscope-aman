#pragma once
#include "AmanWindow.h"

class AmanPlugIn;
class AmanTimeline;

class AmanController
{
private:
	AmanPlugIn* amanPlugin;	// Model
	AmanWindow* amanWindow;	// View

public:
	AmanController(AmanPlugIn* plugin);
	void openWindow();
	void timelinesUpdated(std::vector<AmanTimeline> aircraftList);

	~AmanController();
};

