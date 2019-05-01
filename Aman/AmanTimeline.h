#pragma once
#include <vector>

class AmanAircraft;

class AmanTimeline
{
private:
	int seconds;
	int resolution;

	void drawAircraftChain(HDC hdc, int timeNow, int xStart, int yStart, float pixelsPerSec, bool left, std::vector<AmanAircraft> aircraftList);
public:
	AmanTimeline(std::string fix, int seconds, int resolution);
	AmanTimeline(std::string fixLeft, std::string fixRight, int seconds, int resolution);
	void render(CRect clientRect, HDC memdc, int i);

	bool dual;
	std::string fixes[2];
	std::vector<AmanAircraft>* aircraftLists;
};

