#pragma once

class AmanAircraft {
public: 
	const char* callsign;
	const char* arrivalRunway;
	const char* icaoType;
	const char* nextFix;
	bool trackedByMe;
	char wtc;
	int eta;
	double distLeft;

	bool operator< (const AmanAircraft &other) const {
		return eta < other.eta;
	}
};