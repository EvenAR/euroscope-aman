#pragma once
#include <string>
#include <vector>
#include <string>

class AmanAircraft;
class TagItem;

class AmanTimeline {
public:
    AmanTimeline(
        std::vector<std::string> fixes, 
        std::vector<std::string> viaFixes, 
        std::vector<std::string> destinationAirports, 
        std::vector<std::shared_ptr<TagItem>> tagItems,
        const std::string& alias
    );

    std::string getIdentifier();
    bool isDual();
    bool containsListForFix(std::string fixName);

    std::vector<AmanAircraft>* getAircraftList() { return &aircraftList; }
    std::vector<AmanAircraft> getAircraftList(std::vector<std::string> fixNames);
    std::vector<std::string> getFixes() { return fixes; }
    std::vector<std::string> getViaFixes() { return viaFixes; }
    std::vector<std::string> getDestinationAirports() { return destinationAirports; }
    std::vector<std::shared_ptr<TagItem>> getTagItems() { return tagItems; }
    uint32_t getWidth() { return width; }

    ~AmanTimeline();

private:
    std::string alias;
    std::vector<AmanAircraft> aircraftList;
    std::vector<std::string> viaFixes;
    std::vector<std::string> fixes;
    std::vector<std::string> destinationAirports;
    std::vector<std::shared_ptr<TagItem>> tagItems;
    uint32_t width;
};

