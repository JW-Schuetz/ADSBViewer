#include "pch.h"
#include "BitMask.h"


BitMask::BitMask(int width, int shift)
{
	this->width = width;
	this->shift = shift;

	// '1'-er Array initialisieren
	char* ones = new char[width + 1];
	memset(ones, '1', width);
	ones[width] = 0;

	ADSBLongBitset tmp(ones);
	mask = tmp << shift;

	// aufräumen
	delete[] ones;
}

ADSBLongBitset BitMask::Apply(ADSBLongBitset raw)
{
	return((raw & mask) >> shift);
}

ADSBLongBitset BitMask::GetMask()
{
	return mask;
}
