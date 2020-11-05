#pragma once
#include <vector>
#include <string>
#include "AmanAircraft.h"

class AmanAircraft;

class AmanTimeline
{
private:
	int seconds = 3600;
	std::string identifier;
	bool dual;

	void drawAircraftChain(HDC hdc, int timeNow, int xStart, int yStart, float pixelsPerSec, bool left, std::vector<AmanAircraft> aircraftList);

	std::string fixNames[2];
	std::vector<AmanAircraft>* aircraftLists;

public:
	AmanTimeline(std::string fix);
	AmanTimeline(std::string fixLeft, std::string fixRight);

	std::vector<AmanAircraft>* getAircraftList() { return aircraftLists; }
	std::string getIdentifier() { return identifier; }
	std::string* getFixNames() { return fixNames; }

	bool isDual() { return dual; }
	int getRange() { return seconds; }
	void setRange(int seconds) { this->seconds = seconds; };

	~AmanTimeline();
};

