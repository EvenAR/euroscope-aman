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
std::vector<AmanTimeline*> gTimelines;
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
				std::string vias = timelineParams.at(1);
				this->addTimeline(id, vias);
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

std::vector<AmanAircraft> AmanPlugIn::getInboundsForFix(const std::string& fixName, std::vector<std::string> viaFixes) {
	long int timeNow = static_cast<long int> (std::time(nullptr));			// Current UNIX-timestamp in seconds

	CRadarTarget asel = pMyPlugIn->RadarTargetSelectASEL();
	CRadarTarget rt;
	std::vector<AmanAircraft> aircraftList;
	for (rt = pMyPlugIn->RadarTargetSelectFirst(); rt.IsValid(); rt = pMyPlugIn->RadarTargetSelectNext(rt)) {
		float groundSpeed = rt.GetPosition().GetReportedGS();
		if (groundSpeed < 60) {
			continue;
		}
			
		CFlightPlanExtractedRoute route = rt.GetCorrelatedFlightPlan().GetExtractedRoute();
		CFlightPlanPositionPredictions predictions = rt.GetCorrelatedFlightPlan().GetPositionPredictions();
		bool isSelectedAircraft = asel.IsValid() && rt.GetCallsign() == asel.GetCallsign();

		int targetFixIndex = getFixIndexByName(route, fixName);

		if (targetFixIndex != -1 && route.GetPointDistanceInMinutes(targetFixIndex) > -1) {	// Target fix found and has not been passed
			bool fixIsDestination = targetFixIndex == route.GetPointsNumber() - 1;
			int timeToFix;

			if (fixIsDestination) {
				float restDistance = predictions.GetPosition(predictions.GetPointsNumber() - 1).DistanceTo(route.GetPointPosition(targetFixIndex));
				timeToFix = (predictions.GetPointsNumber() - 1) * 60 + (restDistance / groundSpeed) * 60.0 * 60.0;
			}
			else {
				// Find the two position prediction points closest to the target point
				float min1dist = INFINITE;
				float min2dist = INFINITE;
				float minScore = INFINITE;
				int predIndexBeforeWp = 0;

				for (int p = 0; p < predictions.GetPointsNumber(); p++) {
					float dist1 = predictions.GetPosition(p).DistanceTo(route.GetPointPosition(targetFixIndex));
					float dist2 = predictions.GetPosition(p + 1).DistanceTo(route.GetPointPosition(targetFixIndex));

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
					fixName,
					rt.GetCorrelatedFlightPlan().GetFlightPlanData().GetArrivalRwy(),
					rt.GetCorrelatedFlightPlan().GetFlightPlanData().GetAircraftFPType(),
					rt.GetCorrelatedFlightPlan().GetControllerAssignedData().GetDirectToPointName(),
					getFirstViaFixIndex(route, viaFixes),
					rt.GetCorrelatedFlightPlan().GetTrackingControllerIsMe(),
					isSelectedAircraft,
					rt.GetCorrelatedFlightPlan().GetFlightPlanData().GetAircraftWtc(),
					timeNow + timeToFix - rt.GetPosition().GetReceivedTime(),
					findRemainingDist(rt, route, targetFixIndex),
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

			curr->timeToNextAircraft = curr->eta - next->eta;
		}
	}
	
	return aircraftList;
}

void AmanPlugIn::OnTimer(int Counter) {
	// Runs every second
	amanController->dataUpdated(getTimelines());
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
			gTimelines.clear();
			timelinesChanged = true;
			cmdHandled = true;
		}
		else if (args.size() >= 3) {
			std::string id = args.at(2);
			std::transform(id.begin(), id.end(), id.begin(), ::toupper);

			// Add a new timeline
			if (command == "add") {
				std::string vias = "";
				if (args.size() >= 4) {
					vias = args.at(3);
					std::transform(vias.begin(), vias.end(), vias.begin(), ::toupper);
				}

				this->addTimeline(id, vias);
				timelinesChanged = true;
				cmdHandled = true;
			}
			// Remove a timeline
			if (command == "del") {
				for (int i = 0; i < gTimelines.size(); i++) {
					auto timeline = gTimelines.at(i);
					auto fixNames = timeline->getAircraftList();
					auto idsFromArg = AmanPlugIn::splitString(id, '/');

					if (timeline->isDual() && idsFromArg.size() == 2) {
						if (timeline->containsListForFix(idsFromArg.at(0)) && timeline->containsListForFix(idsFromArg.at(1))) {
							gTimelines.erase(gTimelines.begin() + i);
							timelinesChanged = true;
							cmdHandled = true;
						}
					}
					else if (!timeline->isDual() && timeline->containsListForFix(id)) {
						gTimelines.erase(gTimelines.begin() + i);
						timelinesChanged = true;
						cmdHandled = true;
					}
				}
			}
		}
	}
	if (timelinesChanged) {
		amanController->dataUpdated(getTimelines());
		this->saveToSettings();
	}
	
	return cmdHandled;
}

void AmanPlugIn::addTimeline(std::string id, std::string viaFixes) {
	auto ids = AmanPlugIn::splitString(id, '/');
	auto vias = AmanPlugIn::splitString(viaFixes, ',');
	gTimelines.push_back(new AmanTimeline(ids, vias));
}

int AmanPlugIn::getFixIndexByName(CFlightPlanExtractedRoute extractedRoute, const std::string& fixName) {
	for (int i = 0; i < extractedRoute.GetPointsNumber(); i++) {
		if (!strcmp(extractedRoute.GetPointName(i), fixName.c_str())) {
			return i;
		}
	}
	return -1;
}

int AmanPlugIn::getFirstViaFixIndex(CFlightPlanExtractedRoute extractedRoute, std::vector<std::string> viaFixes) {
	for (int i = 0; i < viaFixes.size(); i++) {
		if (getFixIndexByName(extractedRoute, viaFixes[i]) != -1) {
			return i;
		}
	}
	return -1;
}

double AmanPlugIn::findRemainingDist(CRadarTarget radarTarget, CFlightPlanExtractedRoute extractedRoute, int fixIndex) {
	int closestFixIndex = extractedRoute.GetPointsCalculatedIndex();
	int assignedDirectFixIndex = extractedRoute.GetPointsAssignedIndex();

	int nextFixIndex = assignedDirectFixIndex > -1 ? assignedDirectFixIndex : closestFixIndex;
	double totalDistance = radarTarget.GetPosition().GetPosition().DistanceTo(extractedRoute.GetPointPosition(nextFixIndex));

	// Ignore waypoints prior to nextFixIndex
	for (int i = nextFixIndex; i < fixIndex; i++) {
		totalDistance += extractedRoute.GetPointPosition(i).DistanceTo(extractedRoute.GetPointPosition(i + 1));
	}
	return totalDistance;
}

std::vector<AmanTimeline*>* AmanPlugIn::getTimelines() {
	for each (auto timeline in gTimelines) {
		auto pAircraftList = timeline->getAircraftList();
		auto fixes = timeline->getFixes();
		auto viaFixes = timeline->getViaFixes();

		pAircraftList->clear();
		for each (auto finalFix in fixes) {
			auto var = getInboundsForFix(finalFix, viaFixes);
			pAircraftList->insert(pAircraftList->end(), var.begin(), var.end());
		}
	}
	return &gTimelines; 
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
	for (AmanTimeline* timeline : gTimelines) {
		auto id = timeline->getIdentifier();
		auto viaFixes = timeline->getViaFixes();
		std::ostringstream viaFixesSs;
		copy(viaFixes.begin(), viaFixes.end(), std::ostream_iterator<std::string>(viaFixesSs, ","));
		std::string viaFixesOutput = viaFixesSs.str();
		if(!viaFixesOutput.empty()) viaFixesOutput.pop_back(); // Remove last '/'
		ss << timeline->getIdentifier()
			<< SETTINGS_PARAM_DELIMITER
			<< viaFixesOutput
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
	amanController->dataUpdated(pMyPlugIn->getTimelines());
}

void __declspec (dllexport) EuroScopePlugInExit(void) {
	delete pMyPlugIn;
}
