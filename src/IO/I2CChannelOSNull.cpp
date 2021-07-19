#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ByteTool.h"
#include "IO/I2CChannelOS.h"
#include "Text/MyString.h"

IO::I2CChannelOS::I2CChannelOS(Int32 busNo, UInt8 slaveAddr)
{
	this->hand = 0;
}

IO::I2CChannelOS::~I2CChannelOS()
{
}

Bool IO::I2CChannelOS::IsError()
{
	return this->hand == 0;
}

UOSInt IO::I2CChannelOS::I2CRead(UInt8 *buff, UOSInt buffSize)
{
	return 0;
}

UOSInt IO::I2CChannelOS::I2CWrite(const UInt8 *buff, UOSInt buffSize)
{
	return 0;
}
