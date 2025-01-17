#include "Stdafx.h"
#include "MyMemory.h"
#include "IO/OneWireGPIO.h"
#include "Sync/Thread.h"

IO::OneWireGPIO::OneWireGPIO(IO::IOPin *pin)
{
	NEW_CLASS(this->clk, Manage::HiResClock());
	this->pin = pin;
}

IO::OneWireGPIO::~OneWireGPIO()
{
	DEL_CLASS(this->clk);
}

Bool IO::OneWireGPIO::Init()
{
	this->pin->SetPinOutput(true);
	this->pin->SetPinState(false);
	Sync::Thread::Sleepus(480);
	this->pin->SetPinOutput(false);
	Sync::Thread::Sleepus(1);
	if (!this->pin->IsPinHigh())
	{
		return false;
	}
	Sync::Thread::Sleepus(60);
	if (this->pin->IsPinHigh())
	{
		return false;
	}
	Sync::Thread::Sleepus(420);
	if (!this->pin->IsPinHigh())
	{
		return false;
	}
	return true;
}

void IO::OneWireGPIO::SendBits(const UInt8 *buff, OSInt nBits)
{
	UInt8 mask = 1;
	while (nBits-- > 0)
	{
		if (*buff & mask)
		{
			this->pin->SetPinOutput(true);
			this->pin->SetPinState(false);
			Sync::Thread::Sleepus(6);
			this->pin->SetPinOutput(false);
			this->clk->Start();
			while (this->clk->GetTimeDiff() < 0.000064)
			{
				this->pin->IsPinHigh();
			}
//			Sync::Thread::Sleepus(64);
		}
		else
		{
			this->pin->SetPinOutput(true);
			this->pin->SetPinState(false);
			Sync::Thread::Sleepus(60);
			this->pin->SetPinOutput(false);
			this->clk->Start();
			while (this->clk->GetTimeDiff() < 0.000010)
			{
				this->pin->IsPinHigh();
			}
//			Sync::Thread::Sleepus(10);
		}
		mask = (UInt8)(mask << 1);
		if (mask == 0)
		{
			mask = 1;
			buff++;
		}
	}
}

OSInt IO::OneWireGPIO::ReadBits(UInt8 *buff, OSInt nBits)
{
	OSInt i = nBits;
	UInt8 v = 0;
	UInt8 mask = 1;
	while (i-- > 0)
	{
		this->pin->SetPinOutput(true);
		this->pin->SetPinState(false);
		Sync::Thread::Sleepus(6);
		this->pin->SetPinOutput(false);
		this->clk->Start();
		while (this->clk->GetTimeDiff() < 0.000004)
		{
			this->pin->IsPinHigh();
		}
//		Sync::Thread::Sleepus(4);
		if (this->pin->IsPinHigh())
		{
			v |= mask;
		}
		mask = (UInt8)((mask << 1) & 255);
		while (this->clk->GetTimeDiff() < 0.000059)
		{
			this->pin->IsPinHigh();
		}
//		Sync::Thread::Sleepus(55);
		if (mask == 0)
		{
			*buff++ = v;
			mask = 1;
			v = 0;
		}
	}
	if (mask != 1)
	{
		*buff = v;
	}
	return nBits;
}

