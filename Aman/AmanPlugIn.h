#pragma once
#include <string>
#include <vector>

#include "EuroScopePlugIn.h"

using namespace EuroScopePlugIn;

class AmanAircraft;
class AmanTimeline;

class AmanPlugIn : public CPlugIn {
public:
	AmanPlugIn();
	std::vector<AmanTimeline*>* getTimelines();
	std::vector<AmanTimeline*>* getTimelineForFix();
	virtual ~AmanPlugIn();

private:
	// EuroScope events:
	virtual void OnTimer(int Counter);
	virtual bool OnCompileCommand(const char* sCommandLine);

	int getFixIndexByName(CFlightPlanExtractedRoute extractedRoute, const std::string& fixName);
	int getFirstViaFixIndex(CFlightPlanExtractedRoute extractedRoute, std::vector<std::string> viaFixes);
	double findRemainingDist(CRadarTarget radarTarget, CFlightPlanExtractedRoute extractedRoute, int fixIndex);

	std::vector<AmanAircraft> getInboundsForFix(const std::string& fixName, std::vector<std::string> viaFixes);
	void addTimeline(std::string finalFixes, std::string viaFixes);
	bool removeTimeline(int id);

	void saveToSettings();

	static std::vector<std::string> splitString(const std::string& string, const char delim);

};

