#include "stdafx.h"

#include <iterator>
#include <sstream>
#include <numeric>

#include "AmanAircraft.h"
#include "AmanTimeline.h"
#include "AmanTagItem.h"

AmanTimeline::AmanTimeline(
                std::vector<std::string> fixes,
                std::vector<std::string> viaFixes, 
                std::vector<std::string> destinations, 
                std::vector<std::shared_ptr<TagItem>> tagItems, 
                const std::string& alias,
                uint32_t defaultTimeSpan) {
    this->fixes = fixes;
    this->viaFixes = viaFixes;
    this->aircraftList = std::vector<AmanAircraft>();
    this->destinationAirports = destinations;
    this->tagItems = tagItems;
    this->alias = alias;
    this->defaultTimeSpan = defaultTimeSpan;

    auto addWidth = [](uint32_t acc, std::shared_ptr<TagItem> tagItem) { return acc + tagItem->getWidth(); };
    this->width = std::accumulate(tagItems.begin(), tagItems.end(), 0, addWidth);
}

std::string AmanTimeline::getIdentifier() {
    return alias;
}

bool AmanTimeline::isDual() { return fixes.size() == 2; }

bool AmanTimeline::containsListForFix(std::string fixName) {
    for each (auto tuple in fixes) {
        if (tuple == fixName)
            return true;
    }
    return false;
};

std::vector<AmanAircraft> AmanTimeline::getAircraftList(std::vector<std::string> fixNames) {
    std::vector<AmanAircraft> output;
    for each (auto aircraft in aircraftList) {
        for each (auto fixName in fixNames) {
            if (aircraft.finalFix == fixName) {
                output.push_back(aircraft);
            };
        }
    }
    return output;
}

AmanTimeline::~AmanTimeline() {}
