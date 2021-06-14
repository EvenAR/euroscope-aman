#pragma once

#include <string>

class AmanAircraft {
public:
    std::string callsign;
    std::string finalFix;
    std::string arrivalRunway;
    std::string assignedStar;
    std::string icaoType;
    std::string nextFix;
    std::string scratchPad;

    int viaFixIndex;

    bool trackedByMe;
    bool isSelected;
    char wtc;
    uint32_t targetFixEta;
    uint32_t destinationEta;
    double distLeft;
    uint32_t secondsBehindPreceeding;

    bool operator< (const AmanAircraft& other) const {
        return targetFixEta < other.targetFixEta;
    }
};