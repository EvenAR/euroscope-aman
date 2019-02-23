#pragma once
#include "EuroScopePlugIn.h"
#include "AmanController.h"

using namespace EuroScopePlugIn;

class AmanPlugIn : public CPlugIn {
private:
	int getFixIndexByName(CRadarTarget radarTarget, const char* fixName);
	double findRemainingDist(CRadarTarget radarTarget, int fixIndex);
	std::vector<AmanTimeline> timelines;
	std::vector<AmanAircraft> getFixInboundList(const char* fixName);
public:
	AmanPlugIn();
	virtual ~AmanPlugIn();

	virtual void OnTimer(int Counter);
	virtual bool OnCompileCommand(const char * sCommandLine);
};

