#include "stdafx.h"
#include "AmanPlugIn.h"

#include "stdafx.h"
#include "windows.h"
#include "AmanWindow.h"
#include "AmanController.h"
#include "AmanTimeline.h"
#include "AmanAircraft.h"

#include <iterator>
#include <regex>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>
#include <ctime>
#include <unordered_map>

#define SETTINGS_TIMELINE_DELIMITER '|'
#define SETTINGS_PARAM_DELIMITER ';'

AmanPlugIn* pMyPlugIn;
AmanController* amanController;
std::vector<AmanTimeline*> timelines;
std::unordered_map<const char*, AmanAircraft> allAircraft;

AmanPlugIn::AmanPlugIn() : CPlugIn(COMPATIBILITY_CODE,
	"Arrival Manager",
	"1.3.0",
	"Even Rognlien",
	"Open source")
{
	const char* settings = this->GetDataFromSettings("AMAN");

	if (settings) {
		auto timelinesFromSettings = AmanPlugIn::splitString(settings, SETTINGS_TIMELINE_DELIMITER);
		for (std::string timeline : timelinesFromSettings) {
			try {
				auto timelineParams = AmanPlugIn::splitString(timeline.c_str(), SETTINGS_PARAM_DELIMITER);
				std::string id = timelineParams.at(0);
				this->addTimeline(id);
				this->DisplayUserMessage(
					"AMAN", 
					"Info",
					("The following timeline was loaded from settings: " + id).c_str(), 
					true, false, false, false, false);
			}
			catch (const std::exception& e) {
				std::string msg = "Invalid format in settings file: ";
				this->DisplayUserMessage(
					"AMAN", 
					"Error", 
					("Unable to load timeline from settings: " + timeline).c_str(),
					true, false, false, false, false);
			}	
		}
	}
}

AmanPlugIn::~AmanPlugIn()
{
	delete amanController;
}

std::vector<AmanAircraft> AmanPlugIn::getAllInbounds(const char* fixName) {
	long int timeNow = static_cast<long int> (std::time(nullptr));			// Current UNIX-timestamp in seconds

	CRadarTarget rt;
	std::vector<AmanAircraft> aircraftList;
	for (rt = pMyPlugIn->RadarTargetSelectFirst(); rt.IsValid(); rt = pMyPlugIn->RadarTargetSelectNext(rt)) {
		float groundSpeed = rt.GetPosition().GetReportedGS();

		if (rt.GetPosition().GetReportedGS() < 60) {
			continue;
		}

		CFlightPlanExtractedRoute route = rt.GetCorrelatedFlightPlan().GetExtractedRoute();
		CFlightPlanPositionPredictions predictions = rt.GetCorrelatedFlightPlan().GetPositionPredictions();

		int fixId = getFixIndexByName(rt, fixName);

		if (fixId != -1 && route.GetPointDistanceInMinutes(fixId) > -1) {	// Target fix found and has not been passed
			bool fixIsDestination = fixId == route.GetPointsNumber() - 1;
			int timeToFix;

			if (fixIsDestination) {
				float restDistance = predictions.GetPosition(predictions.GetPointsNumber() - 1).DistanceTo(route.GetPointPosition(fixId));
				timeToFix = (predictions.GetPointsNumber() - 1) * 60 + (restDistance / groundSpeed) * 60.0 * 60.0;
			}
			else {
				// Find the two position prediction points closest to the target point
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
				timeToFix = predIndexBeforeWp * 60.0 + ratio * 60.0;
			}

			if (timeToFix > 0) {
				aircraftList.push_back({
					rt.GetCallsign(),
					rt.GetCorrelatedFlightPlan().GetFlightPlanData().GetArrivalRwy(),
					rt.GetCorrelatedFlightPlan().GetFlightPlanData().GetAircraftFPType(),
					rt.GetCorrelatedFlightPlan().GetControllerAssignedData().GetDirectToPointName(),
					rt.GetCorrelatedFlightPlan().GetTrackingControllerIsMe(),
					rt.GetCorrelatedFlightPlan().GetFlightPlanData().GetAircraftWtc(),
					timeNow + timeToFix - rt.GetPosition().GetReceivedTime(),
					findRemainingDist(rt, fixId),
					0
					});
			}
		}
	}
	if (aircraftList.size() > 0) {
		std::sort(aircraftList.begin(), aircraftList.end());
		std::reverse(aircraftList.begin(), aircraftList.end());
		for (int i = 0; i < aircraftList.size() - 1; i++) {
			AmanAircraft* curr = &aircraftList[i];
			AmanAircraft* next = &aircraftList[i + 1];

			curr->secondsBehindPreceeding = curr->eta - next->eta;
		}
	}
	
	return aircraftList;
}

void AmanPlugIn::OnTimer(int Counter) {
	for (int i = 0; i < timelines.size(); i++) {
		auto aircraftLists = timelines.at(i)->getAircraftList();
		auto fixNames = timelines.at(i)->getFixNames();

		aircraftLists[0] = getAllInbounds(fixNames[0].c_str());
		if (timelines.at(i)->isDual()) {
			aircraftLists[1] = getAllInbounds(fixNames[1].c_str());
		}
	}
	amanController->timelinesUpdated();
}

bool AmanPlugIn::OnCompileCommand(const char * sCommandLine) {
	bool cmdHandled = false;
	bool timelinesChanged = false;

	auto args = AmanPlugIn::splitString(sCommandLine, ' ');
	
	if (args.size() >= 2 && args.at(0) == ".aman") {
		std::string command = args.at(1).c_str();

		if (command == "show") {
			amanController->openWindow();
			cmdHandled = true;
		}
		if (command == "clear") {
			timelines.clear();
			timelinesChanged = true;
			cmdHandled = true;
		}
		else if (args.size() >= 3) {
			std::string id = args.at(2);
			std::transform(id.begin(), id.end(), id.begin(), ::toupper);

			// Add a new timeline
			if (command == "add") {
				this->addTimeline(id);
				timelinesChanged = true;
				cmdHandled = true;
			}
			// Remove a timeline
			if (command == "del") {
				for (int i = 0; i < timelines.size(); i++) {
					auto timeline = timelines.at(i);
					auto fixNames = timeline->getFixNames();
					auto idsFromArg = AmanPlugIn::splitString(id, '/');

					if (timeline->isDual() && idsFromArg.size() == 2) {
						if (idsFromArg.at(1) == fixNames[0] && idsFromArg.at(0) == fixNames[1]) {
							timelines.erase(timelines.begin() + i);
							timelinesChanged = true;
							cmdHandled = true;
						}
					}
					else if (!timeline->isDual() && id == fixNames[0]) {
						timelines.erase(timelines.begin() + i);
						timelinesChanged = true;
						cmdHandled = true;
					}
				}
			}
		}
	}
	if (timelinesChanged) {
		amanController->timelinesUpdated();
		this->saveToSettings();
	}
	
	return cmdHandled;
}

void AmanPlugIn::addTimeline(std::string id) {
	auto ids = AmanPlugIn::splitString(id, '/');
	if (ids.size() > 1) {
		auto ids = AmanPlugIn::splitString(id, '/');
		timelines.push_back(new AmanTimeline(ids.at(0), ids.at(1)));
	}
	else {
		timelines.push_back(new AmanTimeline(ids.at(0)));
	}
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

std::vector<AmanTimeline*>* AmanPlugIn::getTimelines() {
	return &timelines;
}

std::vector<std::string> AmanPlugIn::splitString(std::string string, const char delim) {
	std::vector<std::string> container;
	size_t start;
	size_t end = 0;
	while ((start = string.find_first_not_of(delim, end)) != std::string::npos) {
		end = string.find(delim, start);
		container.push_back(string.substr(start, end - start));
	}
	return container;
}

void AmanPlugIn::saveToSettings() {
	std::stringstream ss;
	for (AmanTimeline* timeline : timelines) {
		auto id = timeline->getIdentifier();
		ss << timeline->getIdentifier()
			<< SETTINGS_PARAM_DELIMITER
			<< timeline->getLength()
			<< SETTINGS_PARAM_DELIMITER;
	}
	ss.seekp(-1, std::ios_base::end);	// remove last delimieter
	pMyPlugIn->SaveDataToSettings("AMAN", "AMAN timelines", ss.str().c_str());
}

void __declspec (dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn ** ppPlugInInstance) {
	// allocate
	*ppPlugInInstance = pMyPlugIn = new AmanPlugIn;

	amanController = new AmanController(pMyPlugIn);
	amanController->openWindow();
	amanController->timelinesUpdated();
}

void __declspec (dllexport) EuroScopePlugInExit(void) {
	delete pMyPlugIn;
}
