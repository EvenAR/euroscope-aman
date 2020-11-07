#pragma once
#include <vector>

#include "EuroScopePlugIn.h"

using namespace EuroScopePlugIn;

class AmanAircraft;
class AmanTimeline;

class AmanPlugIn : public CPlugIn {
private:
	int getFixIndexByName(CFlightPlanExtractedRoute extractedRoute, const std::string& fixName);
	int getFirstViaFixIndex(CFlightPlanExtractedRoute extractedRoute, std::vector<std::string> viaFixes);
	double findRemainingDist(CRadarTarget radarTarget, CFlightPlanExtractedRoute extractedRoute, int fixIndex);

	std::vector<AmanAircraft> getInboundsForFix(const std::string& fixName, std::vector<std::string> viaFixes);
	void addTimeline(std::string id, std::string viaFixes);
	void saveToSettings();

	static std::vector<std::string> splitString(std::string string, const char delim);
public:
	AmanPlugIn();
	virtual ~AmanPlugIn();

	// EuroScope events:
	virtual void OnTimer(int Counter);
	virtual bool OnCompileCommand(const char * sCommandLine);

	std::vector<AmanTimeline*>* getTimelines();
	std::vector<AmanTimeline*>* getTimelineForFix();
};

