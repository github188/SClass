#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ByteTool.h"
#include "Data/DateTime.h"
#include "Data/RandomBytesGenerator.h"

Data::RandomBytesGenerator::RandomBytesGenerator()
{
	this->random = 0;
}

Data::RandomBytesGenerator::~RandomBytesGenerator()
{
	SDEL_CLASS(this->random);
}

void Data::RandomBytesGenerator::NextBytes(UInt8 *buff, UOSInt len)
{
	if (this->random == 0)
	{
		Data::DateTime dt;
		dt.SetCurrTimeUTC();
		NEW_CLASS(this->random, Data::RandomMT19937((UInt32)(dt.ToTicks() & 0xffffffff)));
	}
	UInt8 tmpBuff[4];
	while (len >= 4)
	{
		WriteNInt32(buff, this->random->NextInt32());
		len -= 4;
		buff += 4;
	}
	if (len > 0)
	{
		WriteNInt32(tmpBuff, this->random->NextInt32());
		while (len-- > 0)
		{
			buff[len] = tmpBuff[len];
		}
	}
}
