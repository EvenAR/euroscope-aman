#pragma once
#include <vector>
#include "AmanAircraft.h"
#include "AmanTimeline.h"

class AmanSingleTimeline : AmanTimeline
{
public:
	int seconds;
	int resolution;

	std::string fixRight;
	std::string fixLeft;
	std::vector<AmanAircraft> aircraftList1;
	std::vector<AmanAircraft> aircraftList2;

	AmanTimeline(std::string fix, int seconds, int resolution) {
		fixRight = fix;
		dual = false;
	};
	AmanTimeline(std::string fixLeft, std::string fixRight, int seconds, int resolution) {

	};
};

