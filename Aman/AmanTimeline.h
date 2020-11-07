#pragma once
#include <vector>
#include <string>

class AmanAircraft;

class AmanTimeline
{
public:
	AmanTimeline(std::vector<std::string> fixes, std::vector<std::string> viaFixes);
	std::string getIdentifier();
	bool isDual();
	bool containsListForFix(std::string fixName);

	std::vector<AmanAircraft>* getAircraftList() { return &aircraftList; }
	std::vector<AmanAircraft> getAircraftList(std::vector<std::string> fixNames);
	std::vector<std::string> getFixes() { return fixes; }
	std::vector<std::string> getViaFixes() { return viaFixes; }

	int getRange() { return seconds; }
	void setRange(int seconds) { this->seconds = seconds; };

	~AmanTimeline();

private:
	int seconds = 3600;

	std::vector<AmanAircraft> aircraftList;
	std::vector<std::string> viaFixes;
	std::vector<std::string> fixes;
};

