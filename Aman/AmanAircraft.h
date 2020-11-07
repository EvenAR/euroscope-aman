#pragma once

class AmanAircraft {
public: 
	const char* callsign;
	const char* arrivalRunway;
	const char* icaoType;
	const char* nextFix;
	int viaFixIndex;

	bool trackedByMe;
	bool isSelected;
	char wtc;
	int eta;
	double distLeft;
	int timeToNextAircraft;

	bool operator< (const AmanAircraft &other) const {
		return eta < other.eta;
	}
};