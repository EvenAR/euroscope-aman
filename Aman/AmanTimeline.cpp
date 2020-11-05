#include "stdafx.h"
#include "AmanTimeline.h"

AmanTimeline::AmanTimeline(std::string fix) {
	this->dual = false;
	this->identifier = fix;

	this->fixNames[0] = fix;
	this->fixNames[1] = "";

	this->aircraftLists = new std::vector<AmanAircraft>[2];
};

AmanTimeline::AmanTimeline(std::string fixLeft, std::string fixRight) {
	this->dual = true;
	this->identifier = fixLeft + "/" + fixRight;

	this->fixNames[0] = fixRight;
	this->fixNames[1] = fixLeft;

	this->aircraftLists = new std::vector<AmanAircraft>[2];
};

AmanTimeline::~AmanTimeline()
{
}
