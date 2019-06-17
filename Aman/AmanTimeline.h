#pragma once
#include <vector>

class AmanAircraft;

class AmanTimeline
{
private:
	int seconds;
	int resolution;
	std::string identifier;

	void drawAircraftChain(HDC hdc, int timeNow, int xStart, int yStart, float pixelsPerSec, bool left, std::vector<AmanAircraft> aircraftList);

	bool dual;
	std::string fixNames[2];
	std::vector<AmanAircraft>* aircraftLists;

public:
	AmanTimeline(std::string fix, int seconds, int resolution);
	AmanTimeline(std::string fixLeft, std::string fixRight, int seconds, int resolution);

	std::vector<AmanAircraft>* getAircraftList() { return aircraftLists; }
	std::string getIdentifier() { return identifier; }
	std::string* getFixNames() { return fixNames; }
	int getLength() { return seconds; }
	int getInterval() { return resolution; }
	bool isDual() { return dual; }

	void render(CRect clientRect, HDC memdc, int i);
};

