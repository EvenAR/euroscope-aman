#include "stdafx.h"
#include "AmanTimeline.h"

AmanTimeline::AmanTimeline(std::string fix, std::vector<std::string> viaFixes) {
	this->dual = false;
	this->identifier = fix;

	this->fixNames[0] = fix;
	this->fixNames[1] = "";

	this->aircraftLists = new std::vector<AmanAircraft>[2];
	this->viaFixes = viaFixes;
};

AmanTimeline::AmanTimeline(std::string fixLeft, std::string fixRight, std::vector<std::string> viaFixes) {
	this->dual = true;
	this->identifier = fixLeft + "/" + fixRight;

	this->fixNames[0] = fixRight;
	this->fixNames[1] = fixLeft;

	this->aircraftLists = new std::vector<AmanAircraft>[2];
	this->viaFixes = viaFixes;
};

AmanTimeline::~AmanTimeline()
{
}
