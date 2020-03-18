#pragma once

#include "CommonData.h"
#include "BitMask.h"


class CombineChunks
{
	static char* rest;
	static int counter;

	// Bitmasken für Decodierung
	static BitMask maskDF;			// Downlink-Format (ADS-B: 17 oder 18)
	static BitMask maskCA;			// Capability für DF17 oder DF18
	static BitMask maskICAO;		// FlugzeugID für DF17 oder DF18
	static BitMask maskDATA;		// Daten für DF17 oder DF18
	static BitMask maskTC;			// Type Code für DF17 oder DF18
	static BitMask maskPI;			// Parität für DF17 oder DF18

	static BitMask maskMB;			// MB für DF20,DF21
	static BitMask maskAP;			// AP/DP für DF20,DF21

private:
	static uint32_t checkParity(BYTE*,int);
	static void checkParity(ADSBLongBitset, ADSBMessageState&);
	static int processMessage(char*, char*);
	static char* duplicateBuffer(char*);
	static char* detectRawMessages(char*);
	static string binToCharacter(BYTE*, int);
	static ADSBLongBitset characterToBits(string);
	static BYTE* bitsToBin(ADSBLongBitset);
	static string reveseStringBytes(string);
	static string reveseStringNibbles(string);
	static string decodeAircraftID(ADSBLongBitset);
	static ADSBMessageTypeCode typeCode(ADSBLongBitset);
	static void decodeDF17Message(ADSBLongBitset, ADSBMessageState&, ADSBMessageCode, 
		unsigned long&, unsigned char&, string&);
	static void decodeDF20Message(ADSBLongBitset, ADSBMessageState&, ADSBMessageCode);
	static void decodeMessage(string, ADSBMessageState&, unsigned long&, ADSBMessageCode&,
		unsigned long&, unsigned char&, string&);
	static string* decodedString(ADSBMessageState, unsigned long, ADSBMessageCode, unsigned long, unsigned char, string);

protected:
	CombineChunks(ADSBData*, ICAOData*);
	~CombineChunks() { delete rest; };

public:
	static ADSBData* rawQueue;
	static ICAOData* icaoData;
	static void nextChunk(char*);
};
