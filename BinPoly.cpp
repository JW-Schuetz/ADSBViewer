#include "pch.h"
#include "BinPoly.h"


void BinPoly::convert(ADSBLongBitset in, vector<bool>& out, int&sizeOut, int& rangOut )
{
	// Konversion ADSBLongBitset -> vector<bool>
	rangOut = 0;
	sizeOut = in.size();

	for (int i = 0; i < sizeOut; ++i)
	{
		bool elem = (in[i] == 1 ? true : false);
		out.push_back(elem);

		if (elem == true) rangOut = i;
	}
}

BinPoly::BinPoly(ADSBLongBitset data)
{
	convert(data, bitsData, sizeData, rangData);
}

ADSBLongBitset BinPoly::Divide(ADSBLongBitset divisor)
{
	int rangDivisor;
	int sizeDivisor;
	vector<bool> bitsDivisor;

	convert(divisor, bitsDivisor, sizeDivisor, rangDivisor);

	return divisor;
}
