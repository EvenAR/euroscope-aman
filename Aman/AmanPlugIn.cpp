#include "stdafx.h"

#include "AmanAircraft.h"
#include "AmanController.h"
#include "AmanPlugIn.h"
#include "AmanTimeline.h"
#include "AmanWindow.h"
#include "stdafx.h"
#include "windows.h"
#include "rapidjson/document.h"

#include <algorithm>
#include <ctime>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>

#define SETTINGS_TIMELINE_DELIMITER '|'
#define SETTINGS_PARAM_DELIMITER ';'

#define TO_UPPERCASE(str) std::transform(str.begin(), str.end(), str.begin(), ::toupper);
#define REMOVE_EMPTY(strVec, output)                                                                                   \
    std::copy_if(strVec.begin(), strVec.end(), std::back_inserter(output), [](std::string i) { return !i.empty(); });
#define REMOVE_LAST_CHAR(str)                                                                                          \
    if (str.length() > 0)                                                                                              \
        str.pop_back();
#define DISPLAY_WARNING(str) DisplayUserMessage("Aman", "Warning", str, true, true, true, true, false);

AmanPlugIn::AmanPlugIn() : CPlugIn(COMPATIBILITY_CODE, "Arrival Manager", "1.4.0", "Even Rognlien", "Open source") {
    // Find directory of this .dll
    char fullPluginPath[_MAX_PATH];
    GetModuleFileNameA((HINSTANCE)&__ImageBase, fullPluginPath, sizeof(fullPluginPath));
    std::string fullPluginPathStr(fullPluginPath);
    pluginDirectory = fullPluginPathStr.substr(0, fullPluginPathStr.find_last_of("\\"));

    loadTimelines("aman_profiles.json");

    amanController = std::make_shared<AmanController>(this);
    amanController->openWindow();
    amanController->dataUpdated();
}

AmanPlugIn::~AmanPlugIn() { 
    int i = 0;
}

std::set<std::string> AmanPlugIn::getAvailableIds() {
    std::set<std::string> set;
    for (auto timeline : timelines) {
        set.insert(timeline->getIdentifier());
    }
    return set;
}

std::vector<AmanAircraft> AmanPlugIn::getInboundsForFix(const std::string& fixName, std::vector<std::string> viaFixes) {
    long int timeNow = static_cast<long int>(std::time(nullptr)); // Current UNIX-timestamp in seconds

    CRadarTarget asel = RadarTargetSelectASEL();
    CRadarTarget rt;
    std::vector<AmanAircraft> aircraftList;
    for (rt = RadarTargetSelectFirst(); rt.IsValid(); rt = RadarTargetSelectNext(rt)) {
        float groundSpeed = rt.GetPosition().GetReportedGS();
        if (groundSpeed < 60) {
            continue;
        }

        CFlightPlanExtractedRoute route = rt.GetCorrelatedFlightPlan().GetExtractedRoute();
        CFlightPlanPositionPredictions predictions = rt.GetCorrelatedFlightPlan().GetPositionPredictions();
        bool isSelectedAircraft = asel.IsValid() && rt.GetCallsign() == asel.GetCallsign();

        int targetFixIndex = getFixIndexByName(route, fixName);

        if (targetFixIndex != -1 &&
            route.GetPointDistanceInMinutes(targetFixIndex) > -1) { // Target fix found and has not been passed
            bool fixIsDestination = targetFixIndex == route.GetPointsNumber() - 1;
            int timeToFix;

            if (fixIsDestination) {
                float restDistance = predictions.GetPosition(predictions.GetPointsNumber() - 1)
                    .DistanceTo(route.GetPointPosition(targetFixIndex));
                timeToFix = (predictions.GetPointsNumber() - 1) * 60 + (restDistance / groundSpeed) * 60.0 * 60.0;
            } else {
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
                aircraftList.push_back(
                    { rt.GetCallsign(), fixName, rt.GetCorrelatedFlightPlan().GetFlightPlanData().GetArrivalRwy(),
                     rt.GetCorrelatedFlightPlan().GetFlightPlanData().GetAircraftFPType(),
                     rt.GetCorrelatedFlightPlan().GetControllerAssignedData().GetDirectToPointName(),
                     getFirstViaFixIndex(route, viaFixes), rt.GetCorrelatedFlightPlan().GetTrackingControllerIsMe(),
                     isSelectedAircraft, rt.GetCorrelatedFlightPlan().GetFlightPlanData().GetAircraftWtc(),
                     timeNow + timeToFix - rt.GetPosition().GetReceivedTime(),
                     findRemainingDist(rt, route, targetFixIndex), 0 });
            }
        }
    }
    if (aircraftList.size() > 0) {
        std::sort(aircraftList.begin(), aircraftList.end());
        std::reverse(aircraftList.begin(), aircraftList.end());
        for (int i = 0; i < aircraftList.size() - 1; i++) {
            auto curr = &aircraftList[i];
            auto next = &aircraftList[i + 1];

            curr->secondsBehindPreceeding = curr->eta - next->eta;
        }
    }

    return aircraftList;
}

void AmanPlugIn::OnTimer(int Counter) {
    // Runs every second
    amanController->dataUpdated();
}

bool AmanPlugIn::OnCompileCommand(const char* sCommandLine) {
    bool cmdHandled = false;
    bool timelinesChanged = false;

    auto args = AmanPlugIn::splitString(sCommandLine, ' ');

    if (args.size() > 1 && args.at(0) == ".aman") {
        std::string command = args.at(1);

        if (command == "show") {
            amanController->openWindow();
            cmdHandled = true;
        } else if (command == "clear") {
            timelines.clear();
            timelinesChanged = true;
            cmdHandled = true;
        } else if (command == "add" && args.size() > 2) {
            std::string finalFixes = args.at(2);
            std::string vias = args.size() > 3 ? args.at(3) : "";
            addTimeline(finalFixes, vias);
            timelinesChanged = true;
            cmdHandled = true;
        } else if (command == "del") {
            int id;
            try {
                id = stoi(args.at(2));
            } catch (std::exception& e) {
                id = 0;
            }
            timelinesChanged = cmdHandled = removeTimeline(id - 1);
        }
    }
    if (timelinesChanged) {
        amanController->dataUpdated();
        saveToSettings();
    }

    return cmdHandled;
}

void AmanPlugIn::saveToSettings() {
    std::stringstream ss;
    for (auto& timeline : timelines) {
        auto id = timeline->getIdentifier();
        auto viaFixes = timeline->getViaFixes();

        std::ostringstream viaFixesSs;
        copy(viaFixes.begin(), viaFixes.end(), std::ostream_iterator<std::string>(viaFixesSs, ","));
        std::string viaFixesOutput = viaFixesSs.str();
        REMOVE_LAST_CHAR(viaFixesOutput);

        ss << timeline->getIdentifier() << SETTINGS_PARAM_DELIMITER << viaFixesOutput << SETTINGS_TIMELINE_DELIMITER;
    }
    std::string toSave = ss.str();
    REMOVE_LAST_CHAR(toSave);
    SaveDataToSettings("AMAN", "", toSave.c_str());
}

void AmanPlugIn::loadTimelines(const std::string& filename) {
    std::ifstream file(pluginDirectory + "\\" + filename);
    std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    if (fileContent.empty()) {
        DISPLAY_WARNING((filename + ": the JSON-file was not found or is empty").c_str());
        return;
    }

    using namespace rapidjson;
    Document document;
    document.Parse(fileContent.c_str());

    if (document.HasParseError()) {
        ParseErrorCode code = document.GetParseError();
        size_t offset = document.GetErrorOffset();
        std::string message = filename + ": error when parsing JSON at position " + std::to_string(offset) + ": '" + fileContent.substr(offset, 10) + "'";
        DISPLAY_WARNING(message.c_str());
        return;
    }

    for (auto& v : document["profiles"].GetArray()) {
        auto object = v.GetObjectA();
        std::string alias = object["alias"].GetString();

        std::vector<std::string> finalFixes;
        for (auto& fix : object["fixes"].GetArray()) {
            finalFixes.push_back(fix.GetString());
        }

        std::vector<std::string> viaFixes;
        if (object.HasMember("vias") && object["vias"].IsArray()) {
            for (auto& fix : object["vias"].GetArray()) {
                viaFixes.push_back(fix.GetString());
            }
        }

        timelines.push_back(std::make_shared<AmanTimeline>(finalFixes, viaFixes, alias));
    }
}

void AmanPlugIn::addTimeline(std::string finalFixes, std::string viaFixes) {
    TO_UPPERCASE(finalFixes);
    TO_UPPERCASE(viaFixes);
    auto ids = AmanPlugIn::splitString(finalFixes, '/');
    auto vias = AmanPlugIn::splitString(viaFixes, ',');

    std::vector<std::string> nonEmptyFinalFixes, nonEmptyViaFixes;
    REMOVE_EMPTY(ids, nonEmptyFinalFixes);
    REMOVE_EMPTY(vias, nonEmptyViaFixes);

    timelines.push_back(std::make_shared<AmanTimeline>(nonEmptyFinalFixes, nonEmptyViaFixes, ""));
}

bool AmanPlugIn::removeTimeline(int id) {
    if (id >= 0 && id < timelines.size()) {
        timelines.erase(timelines.begin() + id);
        return true;
    }
    return false;
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
    double totalDistance =
        radarTarget.GetPosition().GetPosition().DistanceTo(extractedRoute.GetPointPosition(nextFixIndex));

    // Ignore waypoints prior to nextFixIndex
    for (int i = nextFixIndex; i < fixIndex; i++) {
        totalDistance += extractedRoute.GetPointPosition(i).DistanceTo(extractedRoute.GetPointPosition(i + 1));
    }
    return totalDistance;
}

std::shared_ptr<std::vector<std::shared_ptr<AmanTimeline>>> AmanPlugIn::getTimelines(std::vector<std::string>& ids) {
    auto result = std::make_shared<std::vector<std::shared_ptr<AmanTimeline>>>();

    for (auto& id : ids) {
        for each (auto& timeline in timelines) {
            if (timeline->getIdentifier() == id) {
                auto pAircraftList = timeline->getAircraftList();
                auto fixes = timeline->getFixes();
                auto viaFixes = timeline->getViaFixes();

                pAircraftList->clear();
                for each (auto finalFix in fixes) {
                    auto var = getInboundsForFix(finalFix, viaFixes);
                    pAircraftList->insert(pAircraftList->end(), var.begin(), var.end());
                }

                result->push_back(timeline);
            }
        }
    }

    return result;
}

std::vector<std::string> AmanPlugIn::splitString(const std::string& string, const char delim) {
    std::vector<std::string> output;
    size_t start;
    size_t end = 0;
    while ((start = string.find_first_not_of(delim, end)) != std::string::npos) {
        end = string.find(delim, start);
        output.push_back(string.substr(start, end - start));
    }
    return output;
}