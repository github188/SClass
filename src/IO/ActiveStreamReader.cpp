#include "Stdafx.h"
#include "MyMemory.h"
#include "IO/ActiveStreamReader.h"
#include "Sync/Thread.h"

#define BUFFCNT 2

UInt32 __stdcall IO::ActiveStreamReader::ReadThread(void *obj)
{
	IO::ActiveStreamReader *me = (IO::ActiveStreamReader *)obj;
	Int32 i = 0;
	BottleNeckType *bnt;
	me->running = true;
	while (!me->toStop)
	{
		while (me->buffs[i].buffSize > 0)
		{
			me->hdlr(me->buffs[i].buff, me->buffs[i].buffSize, me->userData);
			me->buffs[i].buffSize = 0;
			me->emptyEvt->Set();
			i = (i + 1) % BUFFCNT;
		}
		bnt = me->bnt;
		if (bnt && me->reading)
		{
			*bnt = IO::ActiveStreamReader::BNT_READ;
		}
		me->fullEvt->Wait(1000);
	}
	me->running = false;
	me->emptyEvt->Set();
	return 0;
}

IO::ActiveStreamReader::ActiveStreamReader(DataHdlr hdlr, void *userData, IO::Stream *stm, UOSInt buffSize)
{
	this->hdlr = hdlr;
	this->stm = stm;
	this->buffSize = buffSize;
	this->currIndex = 0;
	this->userData = userData;
	NEW_CLASS(emptyEvt, Sync::Event((const UTF8Char*)"IO.ActiveStreamReader.emptyEvt"));
	NEW_CLASS(fullEvt, Sync::Event((const UTF8Char*)"IO.ActiveStreamReader.fullEvt"));
	this->running = false;
	this->toStop = false;
	this->reading = false;

	Int32 i = BUFFCNT;
	this->buffs = MemAlloc(ReadBuffer, BUFFCNT);
	while (i-- > 0)
	{
		buffs[i].buff = MemAllocA(UInt8, buffSize);
		buffs[i].buffSize = 0;
	}
	Sync::Thread::Create(ReadThread, this);
}

IO::ActiveStreamReader::~ActiveStreamReader()
{
	this->toStop = true;
	this->fullEvt->Set();
	while (this->running)
	{
		this->emptyEvt->Wait(1000);
	}
	DEL_CLASS(fullEvt);
	DEL_CLASS(emptyEvt);
	Int32 i = BUFFCNT;
	while (i-- > 0)
	{
		MemFreeA(this->buffs[i].buff);
	}
	MemFree(this->buffs);
}

void IO::ActiveStreamReader::ReadStream(IO::ActiveStreamReader::BottleNeckType *bnt)
{
	Int32 i = this->currIndex;
	UOSInt readSize = this->buffSize;
	UOSInt actSize;
	IO::Stream *stm = this->stm;
	this->bnt = bnt;
	this->reading = true;
	while (true)
	{
		while (this->buffs[i].buffSize)
		{
			if (bnt)
			{
				*bnt = IO::ActiveStreamReader::BNT_WRITE;
			}
			this->emptyEvt->Wait(1000);
		}
		actSize = stm->Read(this->buffs[i].buff, readSize);
		if (actSize <= 0)
			break;
		this->buffs[i].buffSize = actSize;
		this->fullEvt->Set();
		i = (i + 1) % BUFFCNT;
	}
	this->reading = false;
	this->bnt = 0;
	
	i = (i + BUFFCNT - 1) % BUFFCNT;
	while (this->buffs[i].buffSize)
	{
		this->emptyEvt->Wait(1000);
	}
	this->currIndex = (i + 1) % BUFFCNT;
}
