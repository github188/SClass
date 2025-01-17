#include "Stdafx.h"
#include "MyMemory.h"
#include "Net/ProtoClient.h"
#include "Sync/Thread.h"
#include "Text/MyString.h"

UInt32 __stdcall Net::ProtoClient::ProtoThread(void *userObj)
{
	Net::ProtoClient *me = (Net::ProtoClient *)userObj;
	UInt8 buff[4096];
	OSInt buffSize;
	OSInt readSize;


	me->threadRunning = true;
	while (!me->threadToStop)
	{
		if (me->started && me->cli == 0)
		{
			me->cliMut->Lock();
			NEW_CLASS(me->cli, Net::TCPClient(me->sockf, me->cliAddr, me->cliPort));
			if (me->cli->GetLastError())
			{
				DEL_CLASS(me->cli);
				me->cli = 0;
				me->cliMut->Unlock();
			}
			else
			{
				me->cliData = me->proto->CreateStreamData(me->cli);
				buffSize = 0;
				me->connected = true;
				me->cliMut->Unlock();
				me->cliHdlr->ClientConn();
			}
		}
		if (me->cli)
		{
			readSize = me->cli->Read(&buff[buffSize], 2048);
			if (readSize == 0)
			{
				me->cliMut->Lock();
				me->proto->DeleteStreamData(me->cli, me->cliData);
				DEL_CLASS(me->cli);
				me->cli = 0;
				me->connected = false;
				me->cliMut->Unlock();
				me->cliHdlr->ClientDisconn();
			}
			else
			{
				buffSize += readSize;
				readSize = me->proto->ParseProtocol(me->cli, 0, me->cliData, buff, buffSize);
				if (readSize == 0)
				{
					buffSize = 0;
				}
				else if (buffSize != readSize)
				{
					MemCopy(buff, &buff[buffSize - readSize], readSize);
					buffSize = readSize;
				}
				if (buffSize > 2048)
				{
					MemCopy(buff, &buff[buffSize - 2048], 2048);
					buffSize = 2048;
				}
			}
		}

		if (me->cli == 0)
		{
			me->threadEvt->Wait(10000);
		}
	}
	if (me->cli)
	{
		me->cliMut->Lock();
		me->proto->DeleteStreamData(me->cli, me->cliData);
		DEL_CLASS(me->cli);
		me->cli = 0;
		me->connected = false;
		me->cliMut->Unlock();
		me->cliHdlr->ClientDisconn();
	}
	me->threadRunning = false;
	return 0;
}

Net::ProtoClient::ProtoClient(Net::SocketFactory *sockf, const WChar *cliAddr, UInt16 cliPort, IO::IProtocolHandler *proto, Net::ProtoClient::IProtoClientHandler *cliHdlr)
{
	this->sockf = sockf;
	this->cli = 0;
    this->cliAddr = Text::StrCopyNew(cliAddr);
	this->cliPort = cliPort;
	this->proto = proto;
	this->cliHdlr = cliHdlr;
	this->started = false;
	this->connected = false;
	NEW_CLASS(this->cliMut, Sync::Mutex());

	NEW_CLASS(this->threadEvt, Sync::Event(true, L"Net.ProtoClient.threadEvt"));
	this->threadRunning = false;
	this->threadToStop = false;
	Sync::Thread::Create(ProtoThread, this);
	while (!this->threadRunning)
	{
		Sync::Thread::Sleep(10);
	}
}

Net::ProtoClient::~ProtoClient()
{
	this->threadToStop = true;
	this->cliMut->Lock();
	if (this->cli)
	{
		this->cli->Close();
	}
	this->cliMut->Unlock();
	this->threadEvt->Set();
	while (this->threadRunning)
	{
		Sync::Thread::Sleep(10);
	}
	DEL_CLASS(this->cliMut);
	DEL_CLASS(this->threadEvt);
	Text::StrDelNew(this->cliAddr);
}

void Net::ProtoClient::Start()
{
	if (this->started)
		return;
	this->started = true;
	this->threadEvt->Set();
}
void Net::ProtoClient::Reconnect()
{
	this->cliMut->Lock();
	if (this->cli)
	{
		this->cli->Close();
	}
	this->cliMut->Unlock();
}

Bool Net::ProtoClient::IsConnected()
{
	return this->connected;//this->cli != 0;
}

Bool Net::ProtoClient::SendPacket(UInt8 *buff, OSInt buffSize, Int32 cmdType, Int32 seqId)
{
	UInt8 sendBuff[2048];
	OSInt sendSize;
	Bool succ = true;
	this->cliMut->Lock();
	if (this->cli)
	{
		sendSize = this->proto->BuildPacket(sendBuff, cmdType, seqId, buff, buffSize);
		if (this->cli->Write(sendBuff, sendSize) != sendSize)
		{
			succ = false;
		}
	}
	else
	{
		succ = false;
	}
	this->cliMut->Unlock();
	return succ;
}

Bool Net::ProtoClient::SendPacket(UInt8 *buff, OSInt buffSize)
{
	Bool succ = true;
	this->cliMut->Lock();
	if (this->cli)
	{
		if (this->cli->Write(buff, buffSize) != buffSize)
		{
			succ = false;
		}
	}
	else
	{
		succ = false;
	}
	this->cliMut->Unlock();
	return succ;
}
