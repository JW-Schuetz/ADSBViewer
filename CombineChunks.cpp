#include "pch.h"
#include <vector>
#include <bitset>
#include <exception>

#include "CombineChunks.h"


#define ITOA_BUFFER_SIZE 10

// Deklaration der static class members
char* CombineChunks::rest;
int CombineChunks::counter;
ADSBData* CombineChunks::rawQueue;
ICAOData* CombineChunks::icaoData;

BitMask CombineChunks::maskDF;			// Downlink-Format (ADS-B: 17 oder 18)
BitMask CombineChunks::maskCA;			// Capability
BitMask CombineChunks::maskICAO;		// FlugzeugID
BitMask CombineChunks::maskDATA;		// Daten
BitMask CombineChunks::maskTC;			// Type Code
BitMask CombineChunks::maskPI;			// Parität
BitMask CombineChunks::maskMB;			// MB für DF20,DF21
BitMask CombineChunks::maskAP;			// AP/DP für DF20,DF21

CombineChunks::CombineChunks(ADSBData* rawQueue, ICAOData* icaoData)
{
	rest = NULL;
	counter = 0;
	this->rawQueue = rawQueue;
	this->icaoData = icaoData;

	// Initialisierung der Bitmasken für Decodierung
	maskDF = BitMask(5, 107);		// Downlink Format für DF17 oder DF18
	maskCA = BitMask(3, 104);		// Capability für DF17 oder DF18
	maskICAO = BitMask(24, 80);		// FlugzeugID für DF17 oder DF18
	maskDATA = BitMask(56, 24);		// Daten für DF17 oder DF18
	maskTC = BitMask(5, 75);		// Type Code für DF17 oder DF18
	maskPI = BitMask(24, 0);		// Parität für DF17 oder DF18

	maskMB = BitMask(56, 24);		// MB für DF20,DF21
	maskAP = BitMask(24, 0);		// AP/DP für DF20,DF21
}

uint32_t CombineChunks::checkParity(BYTE* msg, int bits)
{
	uint32_t mode_s_checksum_table[] =
	{
	 0x3935ea, 0x1c9af5, 0xf1b77e, 0x78dbbf, 0xc397db, 0x9e31e9, 0xb0e2f0, 0x587178,
	 0x2c38bc, 0x161c5e, 0x0b0e2f, 0xfa7d13, 0x82c48d, 0xbe9842, 0x5f4c21, 0xd05c14,
	 0x682e0a, 0x341705, 0xe5f186, 0x72f8c3, 0xc68665, 0x9cb936, 0x4e5c9b, 0xd8d449,
	 0x939020, 0x49c810, 0x24e408, 0x127204, 0x093902, 0x049c81, 0xfdb444, 0x7eda22,
	 0x3f6d11, 0xe04c8c, 0x702646, 0x381323, 0xe3f395, 0x8e03ce, 0x4701e7, 0xdc7af7,
	 0x91c77f, 0xb719bb, 0xa476d9, 0xadc168, 0x56e0b4, 0x2b705a, 0x15b82d, 0xf52612,
	 0x7a9309, 0xc2b380, 0x6159c0, 0x30ace0, 0x185670, 0x0c2b38, 0x06159c, 0x030ace,
	 0x018567, 0xff38b7, 0x80665f, 0xbfc92b, 0xa01e91, 0xaff54c, 0x57faa6, 0x2bfd53,
	 0xea04ad, 0x8af852, 0x457c29, 0xdd4410, 0x6ea208, 0x375104, 0x1ba882, 0x0dd441,
	 0xf91024, 0x7c8812, 0x3e4409, 0xe0d800, 0x706c00, 0x383600, 0x1c1b00, 0x0e0d80,
	 0x0706c0, 0x038360, 0x01c1b0, 0x00e0d8, 0x00706c, 0x003836, 0x001c1b, 0xfff409,
	 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
	 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
	 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000
	};

	uint32_t crc = 0;
	int offset = (bits == 112) ? 0 : (112 - 56);

	for (int j = 0; j < bits; j++) {
		int byte = j / 8;
		int bit = j % 8;
		int bitmask = 1 << (7 - bit);

		// If bit is set, xor with corresponding table entry.
		if (msg[byte] & bitmask)
			crc ^= mode_s_checksum_table[j + offset];
	}
	return crc; // 24 bit checksum.
}

string CombineChunks::binToCharacter(BYTE* buffer, int size)
{
	char itoabuffer[ITOA_BUFFER_SIZE];

	string s = "";

	for (int i = size - 1; i >= 0; --i)
	{
		BYTE low = buffer[i] & 0x0f;
		BYTE high = (buffer[i] & 0xf0) >> 4;

		memset(itoabuffer, 0, ITOA_BUFFER_SIZE);
		_itoa_s((int)high, itoabuffer, 16);	// 16: hexadezimal
		s += itoabuffer;

		memset(itoabuffer, 0, ITOA_BUFFER_SIZE);
		_itoa_s((int)low, itoabuffer, 16);	// 16: hexadezimal
		s += itoabuffer;
	}

	return s;
}

string CombineChunks::reveseStringBytes(string in)
{
	int len = in.size();

	string out = in;

	// Byte-Reihenfolge des "raw"-Strings invertieren
	for (int i = 0; i < len; i += 2)
	{
		out[i] = in[len - 2 - i];
		out[i + 1] = in[len - 1 - i];
	}

	return out;
}

string CombineChunks::reveseStringNibbles(string in)
{
	int len = in.size();

	string out = in;

	// Nibble-Reihenfolge des "raw"-Strings invertieren
	for (int i = 0; i < len; i += 2)
	{
		out[i] = in[i + 1];
		out[i + 1] = in[i];
	}

	return out;
}

ADSBLongBitset CombineChunks::characterToBits(string raw)
{
	// hexadezimalen ASCII-String in Bitformat konvertieren
	ADSBLongBitset nibble;
	ADSBLongBitset rawMessageBits;

	int len = raw.size();
	for (int i = 0; i < len; ++i)
	{
		switch (raw[i])
		{
		case '0': nibble = ADSBLongBitset("0000"); break;
		case '1': nibble = ADSBLongBitset("0001"); break;
		case '2': nibble = ADSBLongBitset("0010"); break;
		case '3': nibble = ADSBLongBitset("0011"); break;
		case '4': nibble = ADSBLongBitset("0100"); break;
		case '5': nibble = ADSBLongBitset("0101"); break;
		case '6': nibble = ADSBLongBitset("0110"); break;
		case '7': nibble = ADSBLongBitset("0111"); break;
		case '8': nibble = ADSBLongBitset("1000"); break;
		case '9': nibble = ADSBLongBitset("1001"); break;
		case 'a':
		case 'A': nibble = ADSBLongBitset("1010"); break;
		case 'b':
		case 'B': nibble = ADSBLongBitset("1011"); break;
		case 'c':
		case 'C': nibble = ADSBLongBitset("1100"); break;
		case 'd':
		case 'D': nibble = ADSBLongBitset("1101"); break;
		case 'e':
		case 'E': nibble = ADSBLongBitset("1110"); break;
		case 'f':
		case 'F': nibble = ADSBLongBitset("1111"); break;
		default:
			throw(1);
		}

		rawMessageBits |= nibble;
		if (i != len - 1)
			rawMessageBits <<= 4;	// Nibbel nach links verschieben (aber nicht das letzte)
	}

	return rawMessageBits;
}

BYTE* CombineChunks::bitsToBin(ADSBLongBitset bits)
{
	// Bitformat in binäres Format konvertieren, der gelieferte Pointer muss wieder freigegeben werden
	int sizeBits = bits.size();
	int sizeBytes = sizeBits / 8;
	BYTE* buffer = new BYTE[sizeBytes];

#define REVERS
#ifdef REVERS
	int byteNdx = sizeBytes - 1;
#else
	int byteNdx = 0;
#endif
	for (int j = 0; j < sizeBytes; ++j)
	{
		BYTE byte = 0;
		for (int i = 0; i < sizeBits; ++i)
		{
			int bitNdx = 8 * j + i % 8;
			if (bits[bitNdx]) byte |= (1 << i);
		}

		buffer[byteNdx] = byte;
#ifdef REVERS
		--byteNdx;
#else
		++byteNdx;
#endif
	}

	return buffer;
}

void CombineChunks::checkParity(ADSBLongBitset rawMessageBits, ADSBMessageState& state)
{
	ADSBLongBitset Parity = maskPI.Apply(rawMessageBits);
	long int parity = Parity.to_ulong();

	BYTE* buffer = bitsToBin(rawMessageBits);
	uint32_t parityBODY = checkParity(buffer, rawMessageBits.size());
	delete[] buffer;

	if (parityBODY == parity)
		state = ADSBMessageState::OK;
	else
		state = ADSBMessageState::Parity;
}

ADSBMessageTypeCode CombineChunks::typeCode(ADSBLongBitset TC)
{
	int tc = TC.to_ulong();

	ADSBMessageTypeCode typeCode {};

	if (tc <= 4) typeCode = ADSBMessageTypeCode::AircraftID;
	else if (tc <= 8) typeCode = ADSBMessageTypeCode::SurfacePos;
	else if (tc <= 18) typeCode = ADSBMessageTypeCode::AirbornePosBaroAlti;
	else if (tc <= 19) typeCode = ADSBMessageTypeCode::AirborneVelo;
	else if (tc <= 22) typeCode = ADSBMessageTypeCode::AirbornePosGNSSHeight;
	else if (tc <= 27) typeCode = ADSBMessageTypeCode::Reserved;
	else if (tc <= 28) typeCode = ADSBMessageTypeCode::AircraftStatus;
	else if (tc <= 29) typeCode = ADSBMessageTypeCode::TargetState;
	else if (tc <= 31) typeCode = ADSBMessageTypeCode::AircraftOperationStatus;

	return typeCode;
}

string CombineChunks::decodeAircraftID(ADSBLongBitset DATA)
{
	// Jeweils 6 Bits codieren ein Zeichen der ID, Lookup-Tabelle
	const char* lookup = "#ABCDEFGHIJKLMNOPQRSTUVWXYZ#####_###############0123456789######";

	int len = 8;		// fix: 8 Zeichen
	char* buffer = new char[len + 1];
	memset(buffer, 0, len + 1);

	int offset = 48;
	for (int i = 0; i < len; ++i)
	{
		int c = 0;
		c += DATA[offset + 0] * 1;		// pow(2, 0);
		c += DATA[offset + 1] * 2;		// pow(2, 1);
		c += DATA[offset + 2] * 4;		// pow(2, 2);
		c += DATA[offset + 3] * 8;		// pow(2, 3);
		c += DATA[offset + 4] * 16;		// pow(2, 4);
		c += DATA[offset + 5] * 32;		// pow(2, 5);

		buffer[i] = lookup[c];

		offset -= 6;
	}

	string ret = buffer;
	delete[] buffer;

	return ret;
}

void CombineChunks::decodeDF17Message(ADSBLongBitset rawMessageBits, ADSBMessageState& state, 
	ADSBMessageCode code, unsigned long& icao, unsigned char& ca, string& airCraftID)
{
	ADSBLongBitset CA = maskCA.Apply(rawMessageBits);
	ADSBLongBitset ICAO = maskICAO.Apply(rawMessageBits);
	ADSBLongBitset DATA = maskDATA.Apply(rawMessageBits);
	ADSBLongBitset TC = maskTC.Apply(rawMessageBits);

	checkParity(rawMessageBits, state);

	// Auswertung Type Code
	ADSBMessageTypeCode tc = typeCode(TC);
	switch (tc)
	{
	case ADSBMessageTypeCode::AircraftID:
		airCraftID = decodeAircraftID(DATA);
		break;
	case ADSBMessageTypeCode::SurfacePos:
		break;
	case ADSBMessageTypeCode::AirbornePosBaroAlti:
		break;
	case ADSBMessageTypeCode::AirborneVelo:
		break;
	case ADSBMessageTypeCode::AirbornePosGNSSHeight:
		break;
	case ADSBMessageTypeCode::Reserved:
		break;
	case ADSBMessageTypeCode::AircraftStatus:
		break;
	case ADSBMessageTypeCode::TargetState:
		break;
	case ADSBMessageTypeCode::AircraftOperationStatus:
		break;
	}

	ca = (unsigned char)CA.to_ulong();
	icao = ICAO.to_ulong();
}

void CombineChunks::decodeDF20Message(ADSBLongBitset rawMessageBits, ADSBMessageState& state, ADSBMessageCode code)
{
	ADSBLongBitset MB = maskMB.Apply(rawMessageBits);
	ADSBLongBitset AP = maskAP.Apply(rawMessageBits);
}

void CombineChunks::decodeMessage(string raw, ADSBMessageState& state, unsigned long& df,
	ADSBMessageCode& code, unsigned long& icao, unsigned char& ca, string& airCraftID)
{
	state = ADSBMessageState::Unknown;

	// Nibble-String in binäre Bits wandeln
	ADSBLongBitset rawMessageBits = characterToBits(raw);

	ADSBLongBitset DF = maskDF.Apply(rawMessageBits);
	df = DF.to_ulong();
	switch (df)
	{
	case 0:			// ACAS
		code = ADSBMessageCode::DF0;
		break;
	case 11:		// All-Call Reply
		code = ADSBMessageCode::DF11;
		break;
	case 16:		// ACAS
		code = ADSBMessageCode::DF16;
		break;
	case 17:		// Extended Squitter
		code = ADSBMessageCode::DF17;
		decodeDF17Message(rawMessageBits, state, code, icao, ca, airCraftID);
		break;
	case 18:		// Extended Squitter
		code = ADSBMessageCode::DF18;
		decodeDF17Message(rawMessageBits, state, code, icao, ca, airCraftID);
		break;
	case 20:		// Comm-B, Mode-S EHS, Altitude
		code = ADSBMessageCode::DF20;
		decodeDF20Message(rawMessageBits, state, code);
		break;
	case 21:		// Comm-B, Mode-S EHS, Ident
		code = ADSBMessageCode::DF21;
		decodeDF20Message(rawMessageBits, state, code);
		break;
	case 24:		// Comm-D Extended Length Message (ELM)
		code = ADSBMessageCode::DF24;
		break;
	default:		// Unknown
		code = ADSBMessageCode::DFUnknown;
		break;
	}
}

string* CombineChunks::decodedString(ADSBMessageState state, unsigned long df, ADSBMessageCode code,
	unsigned long icao, unsigned char ca, string airCraftID)
{
	string ret;

	char itoabuffer[ITOA_BUFFER_SIZE];

	switch (code)
	{
	case ADSBMessageCode::DF0:			// ACAS
		ret = "DF00 ";
		break;
	case ADSBMessageCode::DF11:			// All-Call Reply
		ret = "DF11 ";
		break;
	case ADSBMessageCode::DF16:			// ACAS
		ret = "DF16 ";
		break;
	case ADSBMessageCode::DF17:			// Extended Squitter
		ret = "DF17 ";
		break;
	case ADSBMessageCode::DF18:			// Extended Squitter
		ret = "DF18 ";
		break;
	case ADSBMessageCode::DF20:			// Comm-B, Mode-S EHS, Altitude
		ret = "DF20 ";
		break;
	case ADSBMessageCode::DF21:			// Comm-B, Mode-S EHS, Ident
		ret = "DF21 ";
		break;
	case ADSBMessageCode::DF24:			// Comm-D Extended Length Message (ELM)
		ret = "DF24 ";
		break;
	case ADSBMessageCode::DFUnknown:	// Unknown
		ret = "DF";
		memset(itoabuffer, 0, ITOA_BUFFER_SIZE);
		_itoa_s((int)df, itoabuffer, 10);	// 10: dezimal
		ret += itoabuffer;
		ret += "?";
		break;
	}

	switch (state)
	{
	case ADSBMessageState::Unknown:
		ret += "  UK";
		break;
	case ADSBMessageState::OK:
		ret += "  OK";
		break;
	case ADSBMessageState::Parity:
		ret += "  PA";
		break;
	}

	// Auswertung Capability
	if (ca != 0)
	{
		ret += "  CA:";
		memset(itoabuffer, 0, ITOA_BUFFER_SIZE);
		_itoa_s((int)ca, itoabuffer, 10);	// 10: dezimal
		ret += itoabuffer;
	}

	// Auswertung ICA0
	if (icao != 0)
	{
		ret += "  ICAO:";
		memset(itoabuffer, 0, ITOA_BUFFER_SIZE);
		_itoa_s((int)icao, itoabuffer, 16);	// 16: hexadezimal
		ret += itoabuffer;
	}

	if (airCraftID.size() != 0)
	{
		ret += "  ID: ";
		ret += airCraftID;
	}

	return new string(ret);
}

int CombineChunks::processMessage(char* s1, char* s2)
{
	// Eine komplette Rawmessage Syntax-checken, parsen und in Message-Liste eintragen.
	// Returnwert: 0: alles OK, -1: Syntaxfehler

	// s1: Pointer auf erstes Zeichen der Rawmessage
	// s2: Pointer auf letztes Zeichen der Rawmessage

	// Struktur der Daten: !Zeitstempel*ADS-B-Message\r\n
	//                     ^           ^               ^
	//                     |           |               |
	//                     s1          str1            s2

	// Syntax-Check
	if (*s1 != '!') return -1;

	char* str1 = strpbrk(s1, "*");
	if (str1 == NULL) return -1;

	if (*s2 != '\n') return -1;
	if (*(s2 - 1) != '\r') return -1;

	// Parse
	++s1;	// "!" überspringen
	int lenTimeStamp = str1 - s1;
	char* timeStamp = new char[lenTimeStamp + 1];
	strncpy_s(timeStamp, lenTimeStamp + 1, s1, lenTimeStamp);
	*(timeStamp + lenTimeStamp) = 0;

	++str1;							// "*" überspringen
	s2 = s2 - 2;					// "\r\n" überspringen
	int lenRawMsg = s2 - str1;		// ";" nicht mitkopieren
	char* rawMsg = new char[lenRawMsg + 1];
	strncpy_s(rawMsg, lenRawMsg + 1, str1, lenRawMsg);
	*(rawMsg + lenRawMsg) = 0;

	// Eintrag in Message-Liste
	ADSBMessage row;
	unsigned char ca = 0;
	unsigned long icao = 0;
	unsigned long df = 0;
	string airCraftID;
	ADSBMessageCode code;
	ADSBMessageState state;

	// Raw-Message decodieren
	decodeMessage(rawMsg, state, df, code, icao, ca, airCraftID);

	row.timeMark = new string(timeStamp);
	row.rawMessage = new string(rawMsg);
	row.decodedMessage = decodedString(state, df, code, icao, ca, airCraftID);
	row.code = new ADSBMessageCode(code);
	row.icao = new unsigned long(icao);
	row.state = new ADSBMessageState(state);

	rawQueue->push_back(row);
	if (icao != 0) icaoData->insert(pair(icao, rawMsg));	// hier noch alle bekannten Daten sammeln

	delete[] timeStamp;
	delete[] rawMsg;

	return 0;
}

char* CombineChunks::duplicateBuffer(char* buffer)
{
	int len = strlen(buffer);
	char* cpy = new char[len + 1];
	*(cpy + len) = 0;	// 0-Terminierung
	memcpy_s(cpy, len + 1, buffer, len);
	return cpy;
}

char* CombineChunks::detectRawMessages(char* buffer)
{
	// Erkennung und Listeneintrag aller Rawmessages in buffer.
	// Rekursiv, aber keine Heap-Nutzung ausser in der Message-Liste.
	// Es wird ein evtl. verbleibender Reststring in der Variable rest gespeichert.
	if (buffer == NULL) return buffer;
	if (strlen(buffer) == 0) return buffer;

	char* str1 = strpbrk(buffer, "!");
	if (str1 != NULL)
	{
		// Ende 1. ADS-B Message suchen
		char* str2 = strpbrk(str1 + 1, "\n");
		if (str2 != NULL)
		{
			// vollständige ADS-B Message gefunden -> in Message-Liste eintragen
			if (processMessage(str1, str2) == -1) 
				throw(exception("CombineChunks SyntaxError Exception!!!"));

			++str2;	// "\n" überspringen
			char* buff = NULL;
			if (*str2 != NULL) buff = detectRawMessages(str2);
			return buff;
		}
		return buffer;
	}
	return buffer;
}

void CombineChunks::nextChunk(char* buffer)
{
	// Struktur der Daten in buffer: .....!Zeitstempel*ADS-B-Message\r\n!Zeitstempel*ADS-B-Message\r\n.....

	if (buffer == NULL) return;	// nichts zu tun!

	int lenBuffer = strlen(buffer);
	if (lenBuffer == 0) return;	// nichts zu tun!

	++counter;	// Aufrufzähler

	if (counter == 1)
	{
		// 1. Aufruf
		char* str1 = strpbrk(buffer, "!");
		if (str1 != NULL)
		{
			char* r = detectRawMessages(buffer);	// "!" gefunden, Chunk vorher weglassen
			if (r != NULL && strlen(r) != 0) rest = duplicateBuffer(r);
		}
	}
	else
	{
		// Folgeaufruf
		if (rest != NULL)
		{
			// Reststring vorhanden
			int len1 = strlen(rest);
			if (len1 != 0)
			{
				// Reststring vorne an buffer anhängen 
				int len2 = strlen(buffer);	// Länge buffer
				char* buff = new char[len1 + len2 + 1];
				strcpy_s(buff, len1 + len2 + 1, rest);
				strncpy_s(buff + len1, len2 + 1, buffer, len2);
				*(buff + len1 + len2) = 0;	// 0-Terminierung

				delete rest;
				rest = NULL;

				char* r = detectRawMessages(buff);
				if (r != NULL && strlen(r) != 0) rest = duplicateBuffer(r);

				delete[] buff;
			}
		}
		else
		{
			char* r = detectRawMessages(buffer);	// kein Reststring vorhanden
			if (r != NULL && strlen(r) != 0) rest = duplicateBuffer(r);
		}
	}
}
