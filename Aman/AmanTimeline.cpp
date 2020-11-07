#include "stdafx.h"
#include "AmanTimeline.h"
#include "AmanAircraft.h"
#include <sstream>
#include <iterator>

AmanTimeline::AmanTimeline(std::vector<std::string> fixes, std::vector<std::string> viaFixes) {
	this->fixes = fixes;
	this->viaFixes = viaFixes;
	this->aircraftList = std::vector<AmanAircraft>();
}

std::string AmanTimeline::getIdentifier() {
	switch (fixes.size())
	{
	case 1:
		return fixes.at(0);
	case 2:
		return fixes.at(0) + "/" + fixes.at(1);
	default:
		std::ostringstream fixesSs;
		copy(fixes.begin(), fixes.end(), std::ostream_iterator<std::string>(fixesSs, "/"));
		std::string output = fixesSs.str();
		output.pop_back(); // Remove last '/'
		return output;
	}
}

bool AmanTimeline::isDual() {
	return fixes.size() == 2;
}

bool AmanTimeline::containsListForFix(std::string fixName) {
	for each (auto tuple in fixes)	{
		if (tuple == fixName) return true;
	}
	return false;
};

std::vector<AmanAircraft> AmanTimeline::getAircraftList(std::vector<std::string> fixNames) {
	std::vector<AmanAircraft> output;
	for each (auto aircraft in aircraftList) {
		for (int i = 0; i < fixNames.size(); i++) {
			if (aircraft.finalFix == fixNames[i]) {
				output.push_back(aircraft);
			};
		}
	}
	return output;
}

AmanTimeline::~AmanTimeline()
{
}
