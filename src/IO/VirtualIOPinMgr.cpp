#include "Stdafx.h"
#include "IO/VirtualIOPinMgr.h"
#include "Text/MyString.h"

IO::VirtualIOPinMgr::VirtualIOPinMgr(OSInt pinCnt)
{
	this->pinCnt = pinCnt;
	this->pins = MemAlloc(PinStatus*, this->pinCnt);
	PinStatus *status;
	OSInt i;
	i = 0;
	while (i < pinCnt)
	{
		status = MemAlloc(PinStatus, 1);
		status->pinNum = (Int32)i;
		NEW_CLASS(status->mut, Sync::Mutex());
		status->useCnt = 1;
		status->pullHigh = false;
		status->outputCnt = 0;
		status->outputHigh = false;
		this->pins[i] = status;
		i++;
	}
}

IO::VirtualIOPinMgr::~VirtualIOPinMgr()
{
	PinStatus *status;
	OSInt i = this->pinCnt;
	Bool toRel;
	while (i-- > 0)
	{
		status = this->pins[i];
		status->mut->Lock();
		toRel = (status->useCnt-- <= 1);
		status->mut->Unlock();

		if (toRel)
		{
			DEL_CLASS(status->mut);
			MemFree(status);
		}
	}
	MemFree(this->pins);
}

IO::IOPin *IO::VirtualIOPinMgr::CreatePin(Int32 pinNum)
{
	if (pinNum < 0 || pinNum >= this->pinCnt)
		return 0;
	IO::IOPin *pin;
	NEW_CLASS(pin, IO::VirtualIOPin(this->pins[pinNum]));
	return pin;
}

OSInt IO::VirtualIOPinMgr::GetAvailablePins(Data::ArrayList<Int32> *pinList)
{
	OSInt i = 0;
	OSInt j = this->pinCnt;
	while (i < j)
	{
		pinList->Add((Int32)i);
		i++;
	}
	return j;
}

IO::VirtualIOPin::VirtualIOPin(IO::VirtualIOPinMgr::PinStatus *pinStatus)
{
	this->pinStatus = pinStatus;
	this->isOutput = false;
	this->pinStatus->mut->Lock();
	this->pinStatus->useCnt++;
	this->pinStatus->mut->Unlock();
}

IO::VirtualIOPin::~VirtualIOPin()
{
	Bool isRel;
	this->pinStatus->mut->Lock();
	isRel = (this->pinStatus->useCnt-- <= 1);
	if (this->isOutput)
	{
		this->pinStatus->outputCnt--;
		this->isOutput = false;
	}
	this->pinStatus->mut->Unlock();
	if (isRel)
	{
		DEL_CLASS(this->pinStatus->mut);
		MemFree(this->pinStatus);
	}
}

Bool IO::VirtualIOPin::IsPinHigh()
{
	Bool ret;
	this->pinStatus->mut->Lock();
	if (this->pinStatus->outputCnt > 0)
	{
		ret = this->pinStatus->outputHigh;
	}
	else
	{
		ret = this->pinStatus->pullHigh;
	}
	this->pinStatus->mut->Unlock();
	return ret;
}

Bool IO::VirtualIOPin::IsPinOutput()
{
	return this->isOutput;
}

void IO::VirtualIOPin::SetPinOutput(Bool isOutput)
{
	if (this->isOutput != isOutput)
	{
		if (isOutput)
		{
			this->isOutput = true;
			this->pinStatus->mut->Lock();
			this->pinStatus->outputCnt++;
			this->pinStatus->mut->Unlock();
		}
		else
		{
			this->isOutput = false;
			this->pinStatus->mut->Lock();
			this->pinStatus->outputCnt--;
			this->pinStatus->mut->Unlock();
		}
	}
}

void IO::VirtualIOPin::SetPinState(Bool isHigh)
{
	if (this->isOutput)
	{
		this->pinStatus->mut->Lock();
		this->pinStatus->outputHigh = isHigh;
		this->pinStatus->mut->Unlock();
	}
}

Bool IO::VirtualIOPin::SetPullType(PullType pt)
{
	if (pt == IO::IOPin::PT_UP)
	{
		this->pinStatus->pullHigh = true;
	}
	else if (pt == IO::IOPin::PT_DOWN)
	{
		this->pinStatus->pullHigh = false;
	}
	return true;
}

UTF8Char *IO::VirtualIOPin::GetName(UTF8Char *buff)
{
	return Text::StrInt32(Text::StrConcat(buff, (const UTF8Char*)"VirtualIOPin"), this->pinStatus->pinNum);
}