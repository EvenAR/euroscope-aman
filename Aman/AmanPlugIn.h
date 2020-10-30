#pragma once
#include <vector>

#include "EuroScopePlugIn.h"

using namespace EuroScopePlugIn;

class AmanAircraft;
class AmanTimeline;

class AmanPlugIn : public CPlugIn {
private:
	int getFixIndexByName(CRadarTarget radarTarget, const char* fixName);
	double findRemainingDist(CRadarTarget radarTarget, int fixIndex);
	std::vector<AmanAircraft> getAllInbounds(const char* fixName);
	void addTimeline(std::string id);
	void saveToSettings();

	static std::vector<std::string> splitString(std::string string, const char delim);
public:
	AmanPlugIn();
	virtual ~AmanPlugIn();

	virtual void OnTimer(int Counter);
	virtual bool OnCompileCommand(const char * sCommandLine);

	std::vector<AmanTimeline*>* getTimelines();
};

