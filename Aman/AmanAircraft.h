#pragma once

class AmanAircraft {
public: 
	const char* callsign;
	const char* arrivalRunway;
	const char* icaoType;
	bool trackedByMe;
	int eta;
	double distLeft;

	bool operator< (const AmanAircraft &other) const {
		return eta < other.eta;
	}
};