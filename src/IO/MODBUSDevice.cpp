#include "Stdafx.h"
#include "Data/ByteTool.h"
#include "IO/MODBUSDevice.h"
#include "Math/Math.h"
#include "Sync/MutexUsage.h"
#include "Sync/Thread.h"

void __stdcall IO::MODBUSDevice::ReadResult(void *userObj, UInt8 funcCode, const UInt8 *result, UOSInt resultSize)
{
	IO::MODBUSDevice *me = (IO::MODBUSDevice*)userObj;
	if (funcCode == 4)
	{
		if (me->reqBResult)
		{
			MemCopyNO(me->reqBResult, result, resultSize);
			me->reqHasResult = true;
			me->cbEvt->Set();
		}
		else if (resultSize == 4)
		{
			if (me->reqDResult)
			{
				me->reqDResult[0] = ReadMFloat(result);
				me->reqHasResult = true;
				me->cbEvt->Set();
			}
			else if (me->reqIResult)
			{
				me->reqIResult[0] = ReadMInt32(result);
				me->reqHasResult = true;
				me->cbEvt->Set();
			}
		}
		else if (resultSize == 2)
		{
			if (me->reqIResult)
			{
				me->reqIResult[0] = ReadMUInt16(result);
				me->reqHasResult = true;
				me->cbEvt->Set();
			}
		}
	}
	else if (funcCode == 3)
	{
		if (me->reqBResult)
		{
			MemCopyNO(me->reqBResult, result, resultSize);
			me->reqHasResult = true;
			me->cbEvt->Set();
		}
		else if (resultSize == 4)
		{
			if (me->reqDResult)
			{
				me->reqDResult[0] = ReadMFloat(result);
				me->reqHasResult = true;
				me->cbEvt->Set();
			}
			else if (me->reqIResult)
			{
				me->reqIResult[0] = ReadMInt32(result);
				me->reqHasResult = true;
				me->cbEvt->Set();
			}
		}
		else if (resultSize == 2)
		{
			if (me->reqIResult)
			{
				me->reqIResult[0] = ReadMUInt16(result);
				me->reqHasResult = true;
				me->cbEvt->Set();
			}
		}
	}
	else if (funcCode == 2)
	{
		if (me->reqIResult)
		{
			if (resultSize == 1)
			{
				me->reqIResult[0] = result[0];
				me->reqHasResult = true;
				me->cbEvt->Set();
			}
			else if (resultSize == 2)
			{
				me->reqIResult[0] = ReadUInt16(result);
				me->reqHasResult = true;
				me->cbEvt->Set();
			}
			else if (resultSize == 3)
			{
				me->reqIResult[0] = (Int32)ReadUInt24(result);
				me->reqHasResult = true;
				me->cbEvt->Set();
			}
			else if (resultSize == 4)
			{
				me->reqIResult[0] = ReadInt32(result);
				me->reqHasResult = true;
				me->cbEvt->Set();
			}
		}
	}
}

void __stdcall IO::MODBUSDevice::SetResult(void *userObj, UInt8 funcCode, UInt16 startAddr, UInt16 cnt)
{
	IO::MODBUSDevice *me = (IO::MODBUSDevice*)userObj;
	if (funcCode == 5 || funcCode == 6 || funcCode == 15 || funcCode == 16)
	{
		if (me->reqSetStartAddr == startAddr)
		{
			me->reqHasResult = true;
			me->cbEvt->Set();
		}
	}
}

void IO::MODBUSDevice::SetTimeout(UOSInt timeout)
{
	this->timeout = timeout;
}

Bool IO::MODBUSDevice::ReadInputI16(UInt16 addr, Int32 *outVal)
{
	Bool succ;
	Sync::MutexUsage mutUsage(this->reqMut);
	this->reqHasResult = false;
	this->reqIResult = outVal;
	this->cbEvt->Clear();
	this->modbus->ReadInputRegisters(this->addr, addr, 1);
	this->cbEvt->Wait(this->timeout);
	this->reqIResult = 0;
	succ = this->reqHasResult;
	mutUsage.EndUse();
	return succ;
}

Bool IO::MODBUSDevice::ReadInputFloat(UInt16 addr, Double *outVal)
{
	Sync::MutexUsage mutUsage(this->reqMut);
	this->reqHasResult = false;
	this->reqDResult = outVal;
	this->cbEvt->Clear();
	this->modbus->ReadInputRegisters(this->addr, addr, 2);
	this->cbEvt->Wait(this->timeout);
	this->reqDResult = 0;
	return this->reqHasResult;
}

Bool IO::MODBUSDevice::ReadInputBuff(UInt16 addr, UInt16 regCnt, UInt8 *buff)
{
	Sync::MutexUsage mutUsage(this->reqMut);
	this->reqHasResult = false;
	this->reqBResult = buff;
	this->cbEvt->Clear();
	this->modbus->ReadInputRegisters(this->addr, addr, regCnt);
	this->cbEvt->Wait(this->timeout);
	this->reqBResult = 0;
	return this->reqHasResult;
}


Bool IO::MODBUSDevice::ReadHoldingI16(UInt16 addr, Int32 *outVal)
{
	Sync::MutexUsage mutUsage(this->reqMut);
	this->reqHasResult = false;
	this->reqIResult = outVal;
	this->cbEvt->Clear();
	this->modbus->ReadHoldingRegisters(this->addr, addr, 1);
	this->cbEvt->Wait(this->timeout);
	this->reqIResult = 0;
	return this->reqHasResult;
}

Bool IO::MODBUSDevice::ReadHoldingI32(UInt16 addr, Int32 *outVal)
{
	Sync::MutexUsage mutUsage(this->reqMut);
	this->reqHasResult = false;
	this->reqIResult = outVal;
	this->cbEvt->Clear();
	this->modbus->ReadHoldingRegisters(this->addr, addr, 2);
	this->cbEvt->Wait(this->timeout);
	this->reqIResult = 0;
	return this->reqHasResult;
}

Bool IO::MODBUSDevice::ReadHoldingFloat(UInt16 addr, Double *outVal)
{
	Sync::MutexUsage mutUsage(this->reqMut);
	this->reqHasResult = false;
	this->reqDResult = outVal;
	this->cbEvt->Clear();
	this->modbus->ReadHoldingRegisters(this->addr, addr, 2);
	this->cbEvt->Wait(this->timeout);
	this->reqDResult = 0;
	return this->reqHasResult;
}

Bool IO::MODBUSDevice::WriteHoldingU16(UInt16 addr, UInt16 val)
{
	Sync::MutexUsage mutUsage(this->reqMut);
	this->reqHasResult = false;
	this->reqSetStartAddr = addr;
	this->cbEvt->Clear();
	this->modbus->WriteHoldingRegister(this->addr, addr, val);
	this->cbEvt->Wait(this->timeout);
	this->reqSetStartAddr = 0;
	return this->reqHasResult;
}

Bool IO::MODBUSDevice::WriteHoldingsU16(UInt16 addr, UInt16 cnt, UInt16 *val)
{
	UInt8 buff[256];
	OSInt i = 0;
	while (i < cnt)
	{
		WriteMInt16(&buff[i * 2], val[i]);
	}
	Sync::MutexUsage mutUsage(this->reqMut);
	this->reqHasResult = false;
	this->reqSetStartAddr = addr;
	this->cbEvt->Clear();
	this->modbus->WriteHoldingRegisters(this->addr, addr, cnt, buff);
	this->cbEvt->Wait(this->timeout);
	this->reqSetStartAddr = 0;
	return this->reqHasResult;
}

Bool IO::MODBUSDevice::WriteHoldingI32(UInt16 addr, Int32 val)
{
	UInt8 buff[4];
	WriteMInt32(buff, val);
	Sync::MutexUsage mutUsage(this->reqMut);
	this->reqHasResult = false;
	this->reqSetStartAddr = addr;
	this->cbEvt->Clear();
	this->modbus->WriteHoldingRegisters(this->addr, addr, 2, buff);
	this->cbEvt->Wait(this->timeout);
	this->reqSetStartAddr = 0;
	return this->reqHasResult;
}

Bool IO::MODBUSDevice::WriteHoldingF32(UInt16 addr, Single val)
{
	UInt8 buff[4];
	WriteMFloat(buff, val);
	Sync::MutexUsage mutUsage(this->reqMut);
	this->reqHasResult = false;
	this->reqSetStartAddr = addr;
	this->cbEvt->Clear();
	this->modbus->WriteHoldingRegisters(this->addr, addr, 2, buff);
	this->cbEvt->Wait(this->timeout);
	this->reqSetStartAddr = 0;
	return this->reqHasResult;
}

Bool IO::MODBUSDevice::ReadDInputs(UInt16 addr, UInt16 cnt, Int32 *outVal)
{
	Sync::MutexUsage mutUsage(this->reqMut);
	this->reqHasResult = false;
	this->reqIResult = outVal;
	this->cbEvt->Clear();
	this->modbus->ReadInputs(this->addr, addr, cnt);
	this->cbEvt->Wait(this->timeout);
	this->reqIResult = 0;
	return this->reqHasResult;
}

Bool IO::MODBUSDevice::WriteDOutput(UInt16 addr, Bool isHigh)
{
	Sync::MutexUsage mutUsage(this->reqMut);
	this->reqHasResult = false;
	this->reqSetStartAddr = addr;
	this->cbEvt->Clear();
	this->modbus->WriteCoil(this->addr, addr, isHigh);
	this->cbEvt->Wait(this->timeout);
	this->reqSetStartAddr = 0;
	return this->reqHasResult;
}

IO::MODBUSDevice::MODBUSDevice(IO::MODBUSMaster *modbus, UInt8 addr)
{
	this->modbus = modbus;
	this->addr = addr;
	this->timeout = 200;
	NEW_CLASS(this->cbEvt, Sync::Event(true, (const UTF8Char*)"IO.MODBUSDevice.cbEvt"));
	NEW_CLASS(this->reqMut, Sync::Mutex());
	this->reqDResult = 0;
	this->reqIResult = 0;
	this->reqBResult = 0;
	this->reqSetStartAddr = 0;
	this->reqSetCount = 0;
	this->modbus->HandleReadResult(this->addr, ReadResult, SetResult, this);
}

IO::MODBUSDevice::~MODBUSDevice()
{
	this->modbus->HandleReadResult(this->addr, 0, 0, 0);
	DEL_CLASS(this->cbEvt);
	DEL_CLASS(this->reqMut);
}
