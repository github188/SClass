#include "Stdafx.h"
#include "Data/ByteTool.h"

extern "C" Int32 ImageTo8Bit_CalDiff(Int32 col1, Int32 col2)
{
	UInt8 c1[4];
	UInt8 c2[4];
	WriteInt32(c1, col1);
	WriteInt32(c2, col2);
	Int32 v1 = c1[0] - (Int32)c2[0];
	Int32 sum = 0;
	if (v1 < 0)
		sum += -v1;
	else
		sum += v1;
	v1 = c1[1] - (Int32)c2[1];
	if (v1 < 0)
		sum += -v1;
	else
		sum += v1;
	v1 = c1[2] - (Int32)c2[2];
	if (v1 < 0)
		sum += -v1;
	else
		sum += v1;
	return sum;
}