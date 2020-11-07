#pragma once
#include <vector>
#include <string>
#include "AmanAircraft.h"

class AmanAircraft;

class AmanTimeline
{
public:
	AmanTimeline(std::string fix, std::vector<std::string> viaFixes);
	AmanTimeline(std::string fixLeft, std::string fixRight, std::vector<std::string> viaFixes);

	std::vector<AmanAircraft>* getAircraftLists() { return aircraftLists; }
	std::string getIdentifier() { return identifier; }
	std::string* getFixNames() { return fixNames; }
	std::vector<std::string> getViaFixes() { return viaFixes; }

	bool isDual() { return dual; }
	int getRange() { return seconds; }
	void setRange(int seconds) { this->seconds = seconds; };

	~AmanTimeline();

private:
	int seconds = 3600;
	std::string identifier;
	bool dual;

	void drawAircraftChain(HDC hdc, int timeNow, int xStart, int yStart, float pixelsPerSec, bool left, std::vector<AmanAircraft> aircraftList);

	std::string fixNames[2];
	std::vector<AmanAircraft>* aircraftLists;
	std::vector<std::string> viaFixes;

};

