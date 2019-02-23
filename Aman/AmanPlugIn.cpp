#include "stdafx.h"
#include "AmanPlugIn.h"

#include "stdafx.h"
#include "windows.h"
#include "AmanWindow.h"
#include "AmanController.h"
#include "AmanTimeline.h"
#include "AmanAircraft.h"

#include <iterator>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>

AmanPlugIn* pMyPlugIn;
AmanController* amanController;
std::vector<AmanTimeline> timelines;

AmanPlugIn::AmanPlugIn() : CPlugIn(COMPATIBILITY_CODE,
	"Arrival Manager",
	"1.0.0",
	"Even Rognlien",
	"Open source")
{

}

AmanPlugIn::~AmanPlugIn()
{
}

std::vector<AmanAircraft> AmanPlugIn::getFixInboundList(const char* fixName) {
	CRadarTarget rt;
	std::vector<AmanAircraft> aircraftList;
	for (rt = pMyPlugIn->RadarTargetSelectFirst(); rt.IsValid(); rt = pMyPlugIn->RadarTargetSelectNext(rt)) {
		int fixId = getFixIndexByName(rt, fixName);
		bool fixIsDestination = fixId == rt.GetCorrelatedFlightPlan().GetExtractedRoute().GetPointsNumber() - 1;

		if (fixId != -1) {
			CFlightPlanExtractedRoute route = rt.GetCorrelatedFlightPlan().GetExtractedRoute();
			CFlightPlanPositionPredictions predictions = rt.GetCorrelatedFlightPlan().GetPositionPredictions();

			float min1dist = INFINITE;
			float min2dist = INFINITE;
			float minScore = INFINITE;

			int predIndexBeforeWp = 0;

			for (int p = 0; p < predictions.GetPointsNumber(); p++) {
				float dist1 = predictions.GetPosition(p).DistanceTo(route.GetPointPosition(fixId));
				float dist2 = predictions.GetPosition(p + 1).DistanceTo(route.GetPointPosition(fixId));

				if (dist1 + dist2 < minScore) {
					min1dist = dist1;
					min2dist = dist2;
					minScore = dist1 + dist2;
					predIndexBeforeWp = p;
				}
			}

			float ratio = (min1dist / (min1dist + min2dist));
			int timeToFix = predIndexBeforeWp * 60.0 + ratio * 60.0;
			double distToFix = findRemainingDist(rt, fixId);

			if (timeToFix > 0) {
				aircraftList.push_back({
					rt.GetCallsign(),
					rt.GetCorrelatedFlightPlan().GetFlightPlanData().GetArrivalRwy(),
					rt.GetCorrelatedFlightPlan().GetFlightPlanData().GetAircraftFPType(),
					rt.GetCorrelatedFlightPlan().GetTrackingControllerIsMe(),
					rt.GetCorrelatedFlightPlan().GetFlightPlanData().GetAircraftType(),
					timeToFix,
					distToFix
					});
			}
		}
	}
	return aircraftList;
}

void AmanPlugIn::OnTimer(int Counter) {
	for (int i = 0; i < timelines.size(); i++) {
		timelines.at(i).aircraftLists[0] = getFixInboundList(timelines.at(i).fixes[0].c_str());
		if (timelines.at(i).dual) {
			timelines.at(i).aircraftLists[1] = getFixInboundList(timelines.at(i).fixes[1].c_str());
		}
	}
	amanController->timelinesUpdated(&timelines);
}

bool AmanPlugIn::OnCompileCommand(const char * sCommandLine) {
	bool cmdHandled = false;
	std::istringstream iss(sCommandLine);
	std::vector<std::string> results((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());

	if (results.size() >= 2 && results.at(0) == ".aman") {
		std::string command = results.at(1).c_str();

		if (command == "show") {
			amanController->openWindow();
			cmdHandled = true;
		}
		else if (results.size() >= 3) {
			std::string id = results.at(2);
			std::transform(id.begin(), id.end(), id.begin(), ::toupper);

			// Add a new timeline
			if (command == "add") {
				int length = 60;
				int interval = 1;

				if (results.size() >= 4) {
					length = std::stoi(results.at(3));
					if (results.size() >= 5) {
						interval = std::stoi(results.at(4));
					}
				}

				if (id.find('/') != std::string::npos) {
					std::string id1 = id.substr(0, id.find('/'));
					std::string id2 = id.substr(id1.length() + 1);
					timelines.push_back(AmanTimeline(id2, id1, length * 60, interval));
				}
				else {
					timelines.push_back(AmanTimeline(id, length * 60, interval));
				}

				cmdHandled = true;
			}
			// Remove a timeline
			if (command == "del") {
				for (int i = 0; i < timelines.size(); i++) {
					if (timelines.at(i).dual && id.find('/') != std::string::npos) {
						std::string id1 = id.substr(0, id.find('/'));
						std::string id2 = id.substr(id1.length() + 1);

						if (timelines.at(i).fixes[0] == id2 && timelines.at(i).fixes[1] == id1) {
							timelines.erase(timelines.begin() + i);
							cmdHandled = true;
						}
					}
					else if (id == timelines.at(i).fixes[0]) {
						timelines.erase(timelines.begin() + i);
						cmdHandled = true;
					}
				}
			}
		}
	}
	amanController->timelinesUpdated(&timelines);
	return cmdHandled;
}

int AmanPlugIn::getFixIndexByName(CRadarTarget radarTarget, const char* fixName) {
	CFlightPlanExtractedRoute route = radarTarget.GetCorrelatedFlightPlan().GetExtractedRoute();

	for (int i = 0; i < route.GetPointsNumber(); i++) {
		if (!strcmp(route.GetPointName(i), fixName)) {
			return i;
		}
	}
	return -1;
}

double AmanPlugIn::findRemainingDist(CRadarTarget radarTarget, int fixIndex) {
	CFlightPlanExtractedRoute route = radarTarget.GetCorrelatedFlightPlan().GetExtractedRoute();
	int calcIndex = route.GetPointsCalculatedIndex();
	int clrdIndex = route.GetPointsAssignedIndex();

	int nextIndex = clrdIndex > -1 ? clrdIndex : calcIndex;

	double totDist = radarTarget.GetPosition().GetPosition().DistanceTo(route.GetPointPosition(nextIndex));

	for (int i = nextIndex; i < fixIndex; i++) {
		totDist += route.GetPointPosition(i).DistanceTo(route.GetPointPosition(i + 1));
	}
	return totDist;
}

void __declspec (dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn ** ppPlugInInstance)
{
	// allocate
	*ppPlugInInstance = pMyPlugIn = new AmanPlugIn;

	amanController = new AmanController(pMyPlugIn);
	amanController->openWindow();
	amanController->timelinesUpdated(&timelines);
}

void __declspec (dllexport) EuroScopePlugInExit(void)
{
	delete amanController;
	delete pMyPlugIn;
}
