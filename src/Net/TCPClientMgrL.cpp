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
#include <poll.h>
#include <unistd.h>
#include <stdio.h>

/*
TODO:
http://man7.org/linux/man-pages/man2/pipe.2.html
http://man7.org/linux/man-pages/man2/poll.2.html
*/

#define TCP_BUFF_SIZE 2048

typedef struct
{
	int piperdfd;
	int pipewrfd;
	Bool hasData;
} ClassData;

void TCPClientMgr_RemoveCliStat(Data::ArrayList<Net::TCPClientMgr::TCPClientStatus*> *cliArr, Data::ArrayListInt64 *cliIdArr, Net::TCPClientMgr::TCPClientStatus *cliStat)
{
	OSInt ind;
	ind = cliIdArr->SortedIndexOf(cliStat->cli->GetCliId());
	if (ind == -1)
	{
		printf("CliId not found\r\n");
	}
	if (cliArr->GetItem(ind) == cliStat)
	{
		cliArr->RemoveAt(ind);
		cliIdArr->RemoveAt(ind);
	}
	else
	{
		ind = cliArr->IndexOf(cliStat);
		if (ind != -1)
		{
			cliArr->RemoveAt(ind);
			cliIdArr->RemoveAt(ind);
		}
		else
		{
			printf("CliId not found2\r\n");
		}
	}
}

UInt32 __stdcall Net::TCPClientMgr::ClientThread(void *o)
{
	Net::TCPClientMgr *me = (Net::TCPClientMgr*)o;
	Data::DateTime *currTime;
	Manage::HiResClock *clk;
	ClassData *clsData = (ClassData*)me->clsData;
	NEW_CLASS(currTime, Data::DateTime());
	NEW_CLASS(clk, Manage::HiResClock());
	OSInt i;
	OSInt readSize;
	UInt8 tmpBuff[16];
	Net::TCPClientMgr::TCPClientStatus *cliStat;
	OSInt pollfdCap = 16;
	struct pollfd *pollfds = MemAlloc(struct pollfd, pollfdCap);
	Net::TCPClientMgr::TCPClientStatus **pollCli = MemAlloc(Net::TCPClientMgr::TCPClientStatus*, pollfdCap);
	OSInt pollCliCnt = -1;
	OSInt pollReqCnt;
	Bool pollPreData = false;

	me->clientThreadRunning = true;
	Sync::Thread::SetPriority(Sync::Thread::TP_REALTIME);
	while (!me->toStop)
	{
		pollPreData = false;
		me->cliMut->Lock();
		pollCliCnt = me->cliArr->GetCount();
		if (pollCliCnt >= pollfdCap)
		{
			while (pollCliCnt >= pollfdCap)
			{
				pollfdCap = pollfdCap << 1;
			}
			MemFree(pollfds);
			MemFree(pollCli);
			pollfds = MemAlloc(struct pollfd, pollfdCap);
			pollCli = MemAlloc(Net::TCPClientMgr::TCPClientStatus*, pollfdCap);
		}
		pollReqCnt = 0;
		pollfds[0].fd = clsData->piperdfd;
		pollfds[0].events = POLLIN;
		pollfds[0].revents = 0;
		if (clsData->hasData)
		{
			pollfds[0].revents = POLLIN;
			pollPreData = true;
		}
		i = 0;
		while (i < pollCliCnt)
		{
			cliStat = me->cliArr->GetItem(i);
			if (cliStat->reading)
			{
				pollCli[pollReqCnt] = cliStat;
				pollReqCnt++;
				pollfds[pollReqCnt].fd = -1 + (int)(OSInt)cliStat->cli->GetSocket();
				pollfds[pollReqCnt].events = POLLIN;
				pollfds[pollReqCnt].revents = 0;
				if (cliStat->recvDataExist)
				{
					pollfds[pollReqCnt].revents = POLLIN;
					pollPreData = true;
				}
			}
			i++;
		}
		me->cliMut->Unlock();
		if (pollPreData)
		{
			i = 1;
		}
		else
		{
//			printf("Poll Begin\r\n");
			i = poll(pollfds, pollReqCnt + 1, -1);
//			printf("Poll End %d\r\n", i);
		}
		if (i > 0)
		{
			if (pollfds[0].revents != 0)
			{
				i = read(clsData->piperdfd, tmpBuff, 16);
				pollfds[0].revents = 0;
				clsData->hasData = false;
			}
			i = pollReqCnt;
			while (i-- > 0)
			{
				if (pollfds[i + 1].revents != 0)
				{
//					printf("Cli %d revents %d\r\n", i, pollfds[i + 1].revents);
					pollfds[i + 1].revents = 0;
					me->cliMut->Lock();
					cliStat = pollCli[i];
					if (cliStat && cliStat->reading)
					{
						Bool closed = false;
						cliStat->readMut->Lock();
						if (cliStat->cli->IsClosed())
						{
							Sync::Thread::Sleep(1);
							closed = true;
						}
						readSize = cliStat->cli->GetRecvBuffSize();
						if (!closed && readSize <= 0)
						{
//							printf("Cli Empty data found\r\n");
							cliStat->recvDataExist = false;
							cliStat->readMut->Unlock();
							readSize = 0;
							cliStat->cli->ShutdownSend();
							cliStat->cli->Close();
						}
						else
						{
//							printf("Cli Read Begin %d\r\n", readSize);
							readSize = cliStat->cli->Read(cliStat->buff, TCP_BUFF_SIZE);
//							printf("Cli Read End %d\r\n", readSize);
							cliStat->readMut->Unlock();
						}

						if (readSize == -1)
						{
//							printf("Cli readSize = -1\r\n");
							currTime->SetCurrTimeUTC();
							if (currTime->DiffMS(cliStat->lastDataTime) > me->timeOutSeconds * 1000)
							{
//								printf("Cli disconnect\r\n");
								cliStat->cli->ShutdownSend();
								cliStat->cli->Close();
								Sync::Thread::Sleep(1);
								TCPClientMgr_RemoveCliStat(me->cliArr, me->cliIdArr, cliStat);
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
//							printf("Cli read data\r\n");
							cliStat->reading = false;
							cliStat->lastDataTime->SetCurrTimeUTC();
							me->cliMut->Unlock();
							me->evtHdlr(cliStat->cli, me->userObj, cliStat->cliData, Net::TCPClientMgr::TCP_EVENT_HASDATA);
							cliStat->buffSize = readSize;
							me->ProcessClient(cliStat);
						}
						else
						{
//							printf("Cli end conn\r\n");
							TCPClientMgr_RemoveCliStat(me->cliArr, me->cliIdArr, cliStat);
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
						me->cliMut->Unlock();
					}
				}
			}
		}
	}
	DEL_CLASS(currTime);
	DEL_CLASS(clk);
	MemFree(pollfds);
	MemFree(pollCli);
	me->clientThreadRunning = false;
	return 0;
}

UInt32 __stdcall Net::TCPClientMgr::WorkerThread(void *o)
{
	Data::DateTime *dt;
	Net::TCPClientMgr::WorkerStatus *stat = (Net::TCPClientMgr::WorkerStatus*)o;
	Net::TCPClientMgr *me = stat->me;
	ClassData *clsData = (ClassData*)me->clsData;
	Net::TCPClientMgr::TCPClientStatus *cliStat;
	OSInt i;
	stat->running = true;
	NEW_CLASS(dt, Data::DateTime());
	while (!stat->toStop)
	{
		while ((stat->cliStat = (Net::TCPClientMgr::TCPClientStatus*)me->workerTasks->Get()) != 0)
		{
			cliStat = stat->cliStat;
			dt->SetCurrTimeUTC();
			cliStat->timeStart = dt->ToTicks();
			cliStat->timeAlerted = false;
			cliStat->processing = true;
			me->dataHdlr(cliStat->cli, me->userObj, cliStat->cliData, cliStat->buff, cliStat->buffSize);
			cliStat->processing = false;
			cliStat->readMut->Lock();
			cliStat->reading = true;
			cliStat->recvDataExist = cliStat->cli->GetRecvBuffSize() > 0;
			cliStat->readMut->Unlock();
			clsData->hasData = true;
			if (write(clsData->pipewrfd, "", 1) == -1)
			{
				printf("TCPClientMgr: Error in writing to pipe\r\n");
			}
		}
		me->cliMut->Lock();
		dt->SetCurrTimeUTC();
		i = me->cliArr->GetCount();
		while (i-- > 0)
		{
			cliStat = me->cliArr->GetItem(i);
			if (dt->DiffMS(cliStat->lastDataTime) > me->timeOutSeconds * 1000)
			{
//				printf("Client data timeout\r\n");
				cliStat->cli->Close();
			}
		}
		me->cliMut->Unlock();
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
	ClassData *clsData = 0;
	int pipefd[2];
	if (pipe(pipefd) != -1)
	{
		clsData = MemAlloc(ClassData, 1);
		clsData->piperdfd = pipefd[0];
		clsData->pipewrfd = pipefd[1];
		clsData->hasData = false;
	}
	NEW_CLASS(cliArr, Data::ArrayList<Net::TCPClientMgr::TCPClientStatus*>());
	NEW_CLASS(cliIdArr, Data::ArrayListInt64());
	NEW_CLASS(cliMut, Sync::Mutex());
	NEW_CLASS(this->workerTasks, Data::LinkedList());
	this->clsData = clsData;
	if (clsData == 0)
	{
		this->workers = 0;
	}
	else
	{
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
}

Net::TCPClientMgr::~TCPClientMgr()
{
	ClassData *clsData = (ClassData*)this->clsData;
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
	if (clsData)
	{
		clsData->hasData = true;
		i = write(clsData->pipewrfd, "", 1);
	}
	while (clientThreadRunning)
	{
		Sync::Thread::Sleep(10);
	}
	if (this->workers)
	{
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
	}

	DEL_CLASS(cliMut);
	DEL_CLASS(cliIdArr);
	DEL_CLASS(cliArr);
	if (clsData)
	{
		close(clsData->pipewrfd);
		close(clsData->piperdfd);
		MemFree(clsData);
	}
	DEL_CLASS(this->workerTasks);
}

void Net::TCPClientMgr::AddClient(TCPClient *cli, void *cliData)
{
//	printf("Client added and set timeout to %d\r\n", this->timeOutSeconds);
	cli->SetNoDelay(true);
	cli->SetTimeout(this->timeOutSeconds * 1000);
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
	cliStat->reading = true;
	cliStat->recvDataExist = false;
	OSInt i = this->cliIdArr->SortedIndexOf(cli->GetCliId());
	if (i >= 0)
	{
		printf("TCPClientMgr: Duplicate Client Id %llx\r\n", cli->GetCliId());
	}
	this->cliArr->Insert(this->cliIdArr->SortedInsert(cli->GetCliId()), cliStat);
	cliStat->readMut->Unlock();
	this->cliMut->Unlock();
	if (write(((ClassData*)this->clsData)->pipewrfd, "", 1) == -1)
	{
		printf("TCPClientMgr: Error in writing to pipe\r\n");
	}
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