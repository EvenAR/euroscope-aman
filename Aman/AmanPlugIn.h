#pragma once
#include <string>
#include <vector>
#include <memory>
#include <set>
#include "EuroScopePlugIn.h"

using namespace EuroScopePlugIn;

class AmanAircraft;
class AmanTimeline;
class AmanController;

class AmanPlugIn : public CPlugIn {
public:
    AmanPlugIn();
    std::set<std::string> getAvailableIds();
    std::shared_ptr<std::vector<std::shared_ptr<AmanTimeline>>> getTimelines(std::vector<std::string>& ids);
    virtual ~AmanPlugIn();

private:
    std::shared_ptr<AmanController> amanController;
    std::vector<std::shared_ptr<AmanTimeline>> timelines;
    std::string pluginDirectory;

    // EuroScope events:
    virtual void OnTimer(int Counter);
    virtual bool OnCompileCommand(const char* sCommandLine);

    int getFixIndexByName(CFlightPlanExtractedRoute extractedRoute, const std::string& fixName);
    int getFirstViaFixIndex(CFlightPlanExtractedRoute extractedRoute, std::vector<std::string> viaFixes);
    double findRemainingDist(CRadarTarget radarTarget, CFlightPlanExtractedRoute extractedRoute, int fixIndex);

    std::vector<AmanAircraft> getInboundsForFix(const std::string& fixName, std::vector<std::string> viaFixes);
    void addTimeline(std::string finalFixes, std::string viaFixes);
    bool removeTimeline(int id);

    void loadTimelines(const std::string& filename);
    void saveToSettings();
    
    static std::vector<std::string> splitString(const std::string& string, const char delim);
};
