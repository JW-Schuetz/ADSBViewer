#pragma once

#include "CommonData.h"

class BitMask
{
	int width;			// Anzahl der 1-en
	int shift;			// Linksverschiebung
	ADSBLongBitset mask;	// Maske

public:
	BitMask() { width = shift = 0; }
	BitMask(int, int);						// Konstruktor
	ADSBLongBitset Apply(ADSBLongBitset);			// Anwenden der Bitmaske auf ein Bitset
	ADSBLongBitset GetMask();
	int Width() { return width; }
};
