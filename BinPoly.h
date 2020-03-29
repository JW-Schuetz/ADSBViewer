#pragma once

#include <vector>
#include "CommonData.h"


class BinPoly
{
private:
	int rangData;
	int sizeData;
	vector<bool> bitsData;
	void convert(ADSBLongBitset, vector<bool>&, int&, int&);

public:
	BinPoly(ADSBLongBitset);
	ADSBLongBitset Divide(ADSBLongBitset);
};
