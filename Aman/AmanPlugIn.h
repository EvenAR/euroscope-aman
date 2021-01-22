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
    bool shouldRunASELWatcher() { return runAselWatcher; };

    void requestReload();
    void onNewAsel(const std::string& asel);
    virtual ~AmanPlugIn();

private:
    std::shared_ptr<AmanController> amanController;
    std::vector<std::shared_ptr<AmanTimeline>> timelines;
    std::string pluginDirectory;

    virtual void OnTimer(int Counter);

    int getFixIndexByName(CFlightPlanExtractedRoute extractedRoute, const std::string& fixName);
    int getFirstViaFixIndex(CFlightPlanExtractedRoute extractedRoute, std::vector<std::string> viaFixes);
    double findRemainingDist(CRadarTarget radarTarget, CFlightPlanExtractedRoute extractedRoute, int fixIndex);

    std::vector<AmanAircraft> getInboundsForFix(const std::string& fixName, std::vector<std::string> viaFixes);
    void loadTimelines(const std::string& filename);
    
    static std::vector<std::string> splitString(const std::string& string, const char delim);

    HANDLE aselWatcherThread;
    DWORD aselWatcherThreadId;
    bool runAselWatcher;

    // For quicker detection when ASEL changes
    static DWORD WINAPI lookForNewASEL(LPVOID lpParam) {
        AmanPlugIn* plugin = (AmanPlugIn*)lpParam;
        std::string oldAsel = std::string(plugin->RadarTargetSelectASEL().GetCallsign());
        while (plugin->shouldRunASELWatcher()) {
            auto newAsel = plugin->RadarTargetSelectASEL().GetCallsign();
            if (oldAsel != newAsel) {
                plugin->onNewAsel(newAsel);
                oldAsel = std::string(newAsel);
            }
            Sleep(50);
        }
        return 0;
    };
};
