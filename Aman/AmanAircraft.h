#pragma once

#include <string>

class AmanAircraft {
public:
    std::string callsign;
    std::string finalFix;
    std::string arrivalRunway;
    std::string icaoType;
    std::string nextFix;

    int viaFixIndex;

    bool trackedByMe;
    bool isSelected;
    char wtc;
    int eta;
    double distLeft;
    int secondsBehindPreceeding;

    bool operator< (const AmanAircraft& other) const {
        return eta < other.eta;
    }
};