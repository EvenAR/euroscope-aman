#pragma once

#include <vector>

class AmanWindow;
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
	void windowClosed();
	void timelinesUpdated(std::vector<AmanTimeline>* timelines);

	~AmanController();
};

