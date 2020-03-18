#pragma once

#include <string>
#include <vector>
#include <map>
#include <utility>
#include <bitset>
#include "Common.h"


using namespace std;

enum class ADSBMessageState { Unknown = 0, OK, Parity };
enum class ADSBMessageCode { DF0, DF11, DF16, DF17, DF18, DF20, DF21, DF24, DFUnknown };
enum class ADSBMessageTypeCode
{
	AircraftID = 0, SurfacePos, AirbornePosBaroAlti, AirborneVelo, AirbornePosGNSSHeight,
	Reserved, AircraftStatus, TargetState, AircraftOperationStatus
};

// Datentypen
typedef struct ADSBStruct
{
	string* timeMark;
	string* rawMessage;
	string* decodedMessage;
	unsigned long *icao;
	ADSBMessageCode* code;
	ADSBMessageState* state;
} ADSBMessage;

typedef vector<ADSBMessage> ADSBData;
typedef map<unsigned long, string> ICAOData;

typedef bitset<ADSB_LONG_MESSAGE_BITS> ADSBLongBitset;
typedef bitset<ADSB_SHORT_MESSAGE_BITS> ADSBShortBitset;
