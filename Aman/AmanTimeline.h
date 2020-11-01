#pragma once
#include <string>
#include <vector>

class AmanAircraft;

class AmanTimeline
{
private:
	int seconds = 3600;
	std::string identifier;

	void drawAircraftChain(HDC hdc, int timeNow, int xStart, int yStart, float pixelsPerSec, bool left, std::vector<AmanAircraft> aircraftList);

	bool dual;
	std::string fixNames[2];
	std::vector<AmanAircraft>* aircraftLists;

public:
	AmanTimeline(std::string fix);
	AmanTimeline(std::string fixLeft, std::string fixRight);

	std::vector<AmanAircraft>* getAircraftList() { return aircraftLists; }
	std::string getIdentifier() { return identifier; }
	std::string* getFixNames() { return fixNames; }
	int getLength() { return seconds; }
	bool isDual() { return dual; }
	void zoom(int value);

	CRect render(CRect clientRect, HDC memdc, int offset);

	CRect getArea(CRect clientRect, int i);
};

