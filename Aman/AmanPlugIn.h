#pragma once
#include <vector>

#include "EuroScopePlugIn.h"

using namespace EuroScopePlugIn;

class AmanAircraft;

class AmanPlugIn : public CPlugIn {
private:
	int getFixIndexByName(CRadarTarget radarTarget, const char* fixName);
	double findRemainingDist(CRadarTarget radarTarget, int fixIndex);
	std::vector<AmanAircraft> getFixInboundList(const char* fixName);
public:
	AmanPlugIn();
	virtual ~AmanPlugIn();

	virtual void OnTimer(int Counter);
	virtual bool OnCompileCommand(const char * sCommandLine);
};

