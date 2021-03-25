#pragma once
#include <string>
#include <vector>
#include <string>

class AmanAircraft;

class AmanTimeline {
public:
    AmanTimeline(std::vector<std::string> fixes, std::vector<std::string> viaFixes, std::vector<std::string> destinationAirports, const std::string& alias);
    std::string getIdentifier();
    bool isDual();
    bool containsListForFix(std::string fixName);

    std::vector<AmanAircraft>* getAircraftList() { return &aircraftList; }
    std::vector<AmanAircraft> getAircraftList(std::vector<std::string> fixNames);
    std::vector<std::string> getFixes() { return fixes; }
    std::vector<std::string> getViaFixes() { return viaFixes; }
    std::vector<std::string> getDestinationAirports() { return destinationAirports; }

    ~AmanTimeline();

private:
    std::string alias;
    std::vector<AmanAircraft> aircraftList;
    std::vector<std::string> viaFixes;
    std::vector<std::string> fixes;
    std::vector<std::string> destinationAirports;
};

