#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/DateTime.h"
#include "Data/ArrayList.h"
#include "IO/Stream.h"
#include "IO/Console.h"
#include "Manage/HiResClock.h"
#include "Net/SocketFactory.h"
#include "Net/TCPClient.h"
#include "Net/TCPClientMgr.h"
#include "Sync/Event.h"
#include "Sync/Mutex.h"
#include "Sync/Thread.h"

#define TCP_BUFF_SIZE 2048

UInt32 __stdcall Net::TCPClientMgr::ClientThread(void *o)
{
	Net::TCPClientMgr *me = (Net::TCPClientMgr*)o;
	Data::DateTime *currTime;
	Manage::HiResClock *clk;
	Double t;
	Int32 waitPeriod = 1;
	Int64 intT;

	NEW_CLASS(currTime, Data::DateTime());
	NEW_CLASS(clk, Manage::HiResClock());
	OSInt i;
	OSInt readSize;
	Bool found;
	Net::TCPClientMgr::TCPClientStatus *cliStat;
	me->clientThreadRunning = true;
	Sync::Thread::SetPriority(Sync::Thread::TP_REALTIME);
	while (!me->toStop)
	{
		found = false;
		((Sync::Event*)me->clsData)->Clear();
		clk->Start();
		currTime->SetCurrTimeUTC();
		intT = currTime->ToTicks();
		i = me->cliArr->GetCount();
		while (i-- > 0)
		{
			me->cliMut->Lock();
			cliStat = (Net::TCPClientMgr::TCPClientStatus*)me->cliArr->GetItem(i);
			if (cliStat)
			{
				if (cliStat->reading)
				{
					cliStat->readMut->Lock();
					if (cliStat->readReq == 0)
					{
						if (cliStat->cli->IsClosed())
						{
							readSize = 0;
						}
						else
						{
							readSize = -1;
						}
					}
					else
					{
						if (cliStat->cli->IsClosed())
						{
							Sync::Thread::Sleep(1);
						}
						readSize = cliStat->cli->EndRead(cliStat->readReq, false);
					}
					cliStat->readMut->Unlock();
					if (readSize == -1)
					{
						currTime->SetCurrTimeUTC();
						if (currTime->DiffMS(cliStat->lastDataTime) > me->timeOutSeconds * 1000)
						{
							cliStat->cli->ShutdownSend();
							cliStat->cli->Close();
							Sync::Thread::Sleep(1);
							cliStat->cli->EndRead(cliStat->readReq, true);
							me->cliArr->RemoveAt(i);
							me->cliIdArr->RemoveAt(i);
							me->cliMut->Unlock();

							DEL_CLASS(cliStat->lastDataTime);
							DEL_CLASS(cliStat->readMut);
							MemFree(cliStat->buff);
							me->evtHdlr(cliStat->cli, me->userObj, cliStat->cliData, Net::TCPClientMgr::TCP_EVENT_DISCONNECT);
							MemFree(cliStat);
						}
						else
						{
							me->cliMut->Unlock();
						}
					}
					else if (readSize)
					{
						cliStat->reading = false;
						me->cliMut->Unlock();
						cliStat->lastDataTime->SetCurrTimeUTC();
						me->evtHdlr(cliStat->cli, me->userObj, cliStat->cliData, Net::TCPClientMgr::TCP_EVENT_HASDATA);
						cliStat->buffSize = readSize;
						me->ProcessClient(cliStat);
						found = true;
					}
					else
					{
						me->cliArr->RemoveAt(i);
						me->cliIdArr->RemoveAt(i);
						me->cliMut->Unlock();

						DEL_CLASS(cliStat->lastDataTime);
						DEL_CLASS(cliStat->readMut);
						MemFree(cliStat->buff);
						me->evtHdlr(cliStat->cli, me->userObj, cliStat->cliData, Net::TCPClientMgr::TCP_EVENT_DISCONNECT);
						MemFree(cliStat);
					}
				}
				else
				{
					if (cliStat->processing && !cliStat->timeAlerted && (intT - cliStat->timeStart) >= 2000)
					{
						cliStat->timeAlerted = true;
						if (me->toHdlr)
						{
							me->toHdlr(cliStat->cli, me->userObj, cliStat->cliData);
						}
					}
					me->cliMut->Unlock();
				}
			}
			else
			{
				me->cliMut->Unlock();
			}
		}
		t = clk->GetTimeDiff();
		if (t > 0.1)
		{
			clk->Start();
		}

		if (!found)
		{
			clk->Start();
			((Sync::Event*)me->clsData)->Wait(waitPeriod);
			t = clk->GetTimeDiff();
			if (t > 0.5 && ((Sync::Event*)me->clsData)->IsSet())
			{
				clk->Start();
			}
			waitPeriod = waitPeriod << 1;
			if (waitPeriod >= 800)
			{
				waitPeriod = 800;
			}
		}
		else
		{
			waitPeriod = 1;
		}
	}
	DEL_CLASS(currTime);
	DEL_CLASS(clk);
	me->clientThreadRunning = false;
	return 0;
}

UInt32 __stdcall Net::TCPClientMgr::WorkerThread(void *o)
{
	Data::DateTime *dt;
	Net::TCPClientMgr::WorkerStatus *stat = (Net::TCPClientMgr::WorkerStatus*)o;
	Net::TCPClientMgr *me = stat->me;
	stat->running = true;
	NEW_CLASS(dt, Data::DateTime());
	while (!stat->toStop)
	{
		while ((stat->cliStat = (Net::TCPClientMgr::TCPClientStatus*)me->workerTasks->Get()) != 0)
		{
			Net::TCPClientMgr::TCPClientStatus *cliStat = stat->cliStat;
			dt->SetCurrTimeUTC();
			cliStat->timeStart = dt->ToTicks();
			cliStat->timeAlerted = false;
			cliStat->processing = true;
			me->dataHdlr(cliStat->cli, me->userObj, cliStat->cliData, cliStat->buff, cliStat->buffSize);
			cliStat->processing = false;
			Bool setEvt = false;
			if (cliStat->cli->GetRecvBuffSize() > 0)
			{
				setEvt = true;
			}

			cliStat->readMut->Lock();
			cliStat->readReq = cliStat->cli->BeginRead(cliStat->buff, TCP_BUFF_SIZE, ((Sync::Event*)me->clsData));
			cliStat->reading = true;
			cliStat->readMut->Unlock();
			if (setEvt)
			{
				((Sync::Event*)me->clsData)->Set();
			}
		}
		stat->evt->Wait(700);
	}
	DEL_CLASS(dt);
	stat->running = false;
	return 0;
}

void Net::TCPClientMgr::ProcessClient(Net::TCPClientMgr::TCPClientStatus *cliStat)
{
	this->workerTasks->Put(cliStat);
	OSInt i = this->workerCnt;
	while (i-- > 0)
	{
		if (this->workers[i].cliStat == 0)
		{
			this->workers[i].evt->Set();
			return;
		}
	}
}

Net::TCPClientMgr::TCPClientMgr(Int32 timeOutSeconds, TCPClientEvent evtHdlr, TCPClientData dataHdlr, void *userObj, UOSInt workerCnt, TCPClientTimeout toHdlr)
{
	this->timeOutSeconds = timeOutSeconds;
	this->evtHdlr = evtHdlr;
	this->dataHdlr = dataHdlr;
	this->toHdlr = toHdlr;
	this->userObj = userObj;
	this->toStop = false;
	this->clientThreadRunning = false;
	if (workerCnt <= 0)
		workerCnt = 1;
	this->workerCnt = workerCnt;
	Sync::Event *recvEvt;
	NEW_CLASS(recvEvt, Sync::Event(false, (const UTF8Char*)"Net.TCPClientMgr.RecvEvt"));
	NEW_CLASS(cliArr, Data::ArrayList<Net::TCPClientMgr::TCPClientStatus*>());
	NEW_CLASS(cliIdArr, Data::ArrayListInt64());
	NEW_CLASS(cliMut, Sync::Mutex());
	NEW_CLASS(this->workerTasks, Data::LinkedList());
	this->clsData = recvEvt;
	Sync::Thread::Create(ClientThread, this);
	this->workers = MemAlloc(WorkerStatus, workerCnt);
	while (workerCnt-- > 0)
	{
		this->workers[workerCnt].running = false;
		this->workers[workerCnt].toStop = false;
		this->workers[workerCnt].cliStat = 0;
		this->workers[workerCnt].me = this;
		NEW_CLASS(this->workers[workerCnt].evt, Sync::Event(true, (const UTF8Char*)"Net.TCPClientMgr.workers.evt"));
		
		Sync::Thread::Create(WorkerThread, &this->workers[workerCnt]);
	}
}

Net::TCPClientMgr::~TCPClientMgr()
{
	OSInt i = cliArr->GetCount();
	Net::TCPClientMgr::TCPClientStatus *cliStat;
	if (i)
	{
		while (i-- > 0)
		{
			cliMut->Lock();
			cliStat = cliArr->GetItem(i);
			cliMut->Unlock();
			if (cliStat)
			{
				cliStat->cli->Close();
			}
		}
	}
	while (cliArr->GetCount() > 0)
	{
		Sync::Thread::Sleep(10);
	}
	this->toStop = true;
	((Sync::Event*)this->clsData)->Set();
	while (clientThreadRunning)
	{
		Sync::Thread::Sleep(10);
	}
	i = this->workerCnt;
	while (i-- > 0)
	{
		this->workers[i].toStop = true;
		this->workers[i].evt->Set();
	}
	Bool exist = true;
	while (exist)
	{
		exist = false;
		i = this->workerCnt;
		while (i-- > 0)
		{
			if (this->workers[i].running)
			{
				exist = true;
				break;
			}
		}
		if (!exist)
			break;
		Sync::Thread::Sleep(10);
	}
	i = this->workerCnt;
	while (i-- > 0)
	{
		DEL_CLASS(this->workers[i].evt);
	}
	MemFree(this->workers);

	DEL_CLASS(cliMut);
	DEL_CLASS(cliIdArr);
	DEL_CLASS(cliArr);
	Sync::Event *recvEvt = ((Sync::Event*)this->clsData);
	DEL_CLASS(recvEvt);
	DEL_CLASS(this->workerTasks);
}

void Net::TCPClientMgr::AddClient(TCPClient *cli, void *cliData)
{
	cli->SetNoDelay(true);
	this->cliMut->Lock();
	Net::TCPClientMgr::TCPClientStatus *cliStat = MemAlloc(Net::TCPClientMgr::TCPClientStatus, 1);
	cliStat->cli = cli;
	cliStat->cliData = cliData;
	cliStat->buff = MemAlloc(UInt8, TCP_BUFF_SIZE);
	NEW_CLASS(cliStat->lastDataTime, Data::DateTime());
	NEW_CLASS(cliStat->readMut, Sync::Mutex());
	cliStat->lastDataTime->SetCurrTimeUTC();
	cliStat->reading = false;
	cliStat->processing = false;
	cliStat->timeAlerted = false;
	cliStat->timeStart = 0;
	cliStat->readMut->Lock();
	cliStat->readReq = cliStat->cli->BeginRead(cliStat->buff, TCP_BUFF_SIZE, ((Sync::Event*)this->clsData));
	cliStat->reading = true;
	this->cliArr->Insert(this->cliIdArr->SortedInsert(cli->GetCliId()), cliStat);
	cliStat->readMut->Unlock();
	this->cliMut->Unlock();
}

Bool Net::TCPClientMgr::SendClientData(Int64 cliId, const UInt8 *buff, UOSInt buffSize)
{
	OSInt i;
	Net::TCPClientMgr::TCPClientStatus *cliStat = 0;
	this->cliMut->Lock();
	i = this->cliIdArr->SortedIndexOf(cliId);
	if (i >= 0)
	{
		cliStat = this->cliArr->GetItem(i);
	}
	this->cliMut->Unlock();
	if (cliStat)
	{
		return cliStat->cli->Write(buff, buffSize) == buffSize;
	}
	else
	{
		return false;
	}
}

Bool Net::TCPClientMgr::IsError()
{
	return false;
}

void Net::TCPClientMgr::CloseAll()
{
	OSInt i = this->cliArr->GetCount();
	this->cliMut->Lock();
	while (i-- > 0)
	{
		Net::TCPClientMgr::TCPClientStatus *cliStat;
		cliStat = (Net::TCPClientMgr::TCPClientStatus*)this->cliArr->GetItem(i);
		if (cliStat)
		{
			cliStat->cli->Close();
		}
	}
	this->cliMut->Unlock();
}

void Net::TCPClientMgr::BeginGetClient()
{
	this->cliMut->Lock();
}

void Net::TCPClientMgr::EndGetClient()
{
	this->cliMut->Unlock();
}

OSInt Net::TCPClientMgr::GetClientCount()
{
	return this->cliArr->GetCount();
}

Net::TCPClient *Net::TCPClientMgr::GetClient(OSInt index, void **cliData)
{
	Net::TCPClientMgr::TCPClientStatus *cliStat = this->cliArr->GetItem(index);
	if (cliStat)
	{
		*cliData = cliStat->cliData;
		return cliStat->cli;
	}
	return 0;
}