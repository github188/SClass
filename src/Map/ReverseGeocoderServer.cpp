#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ByteTool.h"
#include "Map/ReverseGeocoderServer.h"
#include "Math/Math.h"
#include "Sync/MutexUsage.h"
#include "Sync/Thread.h"
#include "Text/Encoding.h"
#include "Text/MyString.h"

Net::TCPClient *Map::ReverseGeocoderServer::GetLatestClient(UOSInt retryCnt)
{
	Int64 currTime;
	Data::DateTime dt;
	Int64 maxTime = -1;
	UOSInt maxIndex = 0;
	Net::TCPClient *cli;
	void *cliObj;
	ClientStatus *stat;

	dt.SetCurrTimeUTC();
	currTime = dt.ToTicks();

	Sync::MutexUsage mutUsage;
	this->ctrl->UseGetCli(&mutUsage);
	UOSInt i = this->ctrl->GetCliCount();
	if (i <= retryCnt)
	{
		return 0;
	}

	while (i-- > 0)
	{
		cli = this->ctrl->GetClient(i, &cliObj);
		if (cli)
		{
			stat = (ClientStatus*)cliObj;
			if (currTime - stat->lastReqTime > maxTime)
			{
				maxTime = currTime - stat->lastReqTime;
				maxIndex = i;
			}
		}
	}
	cli = 0;
	if (maxTime >= 0)
	{
		cli = this->ctrl->GetClient(maxIndex, &cliObj);
		stat = (ClientStatus*)cliObj;
		stat->lastReqTime = currTime;
	}
	mutUsage.EndUse();
	return cli;
}

Map::ReverseGeocoderServer::ReverseGeocoderServer(Net::SocketFactory *sockf, IO::LogTool *log, UInt16 port)
{
	NEW_CLASS(this->reqMut, Sync::Mutex());
	NEW_CLASS(this->reqEvt, Sync::Event((const UTF8Char*)"Map.ReverseGeocoderServer.reqEvt"));
	NEW_CLASS(this->protocol, IO::ProtoHdlr::ProtoRevGeoHandler(this));
	NEW_CLASS(this->ctrl, Net::TCPServerController(sockf, log, port, (const UTF8Char*)"RGS: ", 4096, this, 8, 240));
}

Map::ReverseGeocoderServer::~ReverseGeocoderServer()
{
	DEL_CLASS(this->ctrl);
	DEL_CLASS(this->protocol);
	DEL_CLASS(this->reqEvt);
	DEL_CLASS(this->reqMut);
}

UTF8Char *Map::ReverseGeocoderServer::SearchName(UTF8Char *buff, UOSInt buffSize, Double lat, Double lon, UInt32 lcid)
{
	UInt8 dataBuff[16];
	UInt8 dataBuff2[256];
	Bool sent;
	UOSInt retryCnt = 0;
	UTF8Char *sptr = 0;
	Sync::MutexUsage mutUsage(this->reqMut);
	this->reqBuff = buff;
	this->reqBuffSize = buffSize;
	this->reqLat = Math::Double2Int32(lat * 200000.0);
	this->reqLon = Math::Double2Int32(lon * 200000.0);
	this->reqLCID = lcid;
	this->reqResult = false;
	buff[0] = 0;

	Net::TCPClient *cli;
	while ((cli = GetLatestClient(retryCnt)) != 0)
	{
		sent = false;
		if (cli)
		{
			WriteInt32(&dataBuff[0], this->reqLat);
			WriteInt32(&dataBuff[4], this->reqLon);
			WriteUInt32(&dataBuff[8], this->reqLCID);
			cli->Write(dataBuff2, this->protocol->BuildPacket(dataBuff2, 0, 0, dataBuff, 12, 0));
			sent = true;
		}
		sptr = 0;
		if (sent)
		{
			OSInt i = 10;
			while (!this->reqResult)
			{
				this->reqEvt->Wait(1000);
				if (i-- <= 0)
					break;
			}
			if (this->reqResult)
			{
				sptr = &buff[Text::StrCharCnt(buff)];
			}
		}
		if (buff[0] != 0)
			break;
		this->reqResult = false;
		retryCnt++;
	}
	this->reqBuff = 0;
	mutUsage.EndUse();
	return sptr;
}

UTF8Char *Map::ReverseGeocoderServer::CacheName(UTF8Char *buff, UOSInt buffSize, Double lat, Double lon, UInt32 lcid)
{
	UInt8 dataBuff[16];
	UInt8 dataBuff2[256];
	Bool sent;
	UOSInt retryCnt = 0;
	UTF8Char *sptr = 0;
	Sync::MutexUsage mutUsage(this->reqMut);
	this->reqBuff = buff;
	this->reqBuffSize = buffSize;
	this->reqLat = Math::Double2Int32(lat * 200000.0);
	this->reqLon = Math::Double2Int32(lon * 200000.0);
	this->reqLCID = lcid;
	this->reqResult = false;
	buff[0] = 0;

	Net::TCPClient *cli;
	while ((cli = GetLatestClient(retryCnt)) != 0)
	{
		sent = false;
		if (cli)
		{
			WriteInt32(&dataBuff[0], this->reqLat);
			WriteInt32(&dataBuff[4], this->reqLon);
			WriteUInt32(&dataBuff[8], this->reqLCID);
			cli->Write(dataBuff2, this->protocol->BuildPacket(dataBuff2, 2, 0, dataBuff, 12, 0));
			sent = true;
		}
		sptr = 0;
		if (sent)
		{
			OSInt i = 10;
			while (!this->reqResult)
			{
				this->reqEvt->Wait(1000);
				if (i-- <= 0)
					break;
			}
			if (this->reqResult)
			{
				sptr = &buff[Text::StrCharCnt(buff)];
			}
		}
		if (buff[0] != 0)
			break;
		this->reqResult = false;
		retryCnt++;
	}
	this->reqBuff = 0;
	mutUsage.EndUse();
	return sptr;}

void *Map::ReverseGeocoderServer::NewConn(Net::TCPClient *cli)
{
	ClientStatus *stat = MemAlloc(ClientStatus, 1);
	stat->cliData = this->protocol->CreateStreamData(cli);
	stat->lastReqTime = 0;
	return stat;
}

void Map::ReverseGeocoderServer::EndConn(Net::TCPClient *cli, void *cliObj)
{
	ClientStatus *stat = (ClientStatus *)cliObj;
	this->protocol->DeleteStreamData(cli, stat->cliData);
	MemFree(stat);
}

UOSInt Map::ReverseGeocoderServer::ReceivedData(Net::TCPClient *cli, void *cliObj, UInt8 *buff, UOSInt buffSize)
{
	ClientStatus *stat = (ClientStatus *)cliObj;
	return this->protocol->ParseProtocol(cli, cliObj, stat->cliData, buff, buffSize);
}

void Map::ReverseGeocoderServer::DataParsed(IO::Stream *stm, void *cliObj, Int32 cmdType, Int32 seqId, const UInt8 *cmd, UOSInt cmdSize)
{
	if (cmdType == 1)
	{
		if (this->reqBuff && ReadInt32(&cmd[0]) == this->reqLat && ReadInt32(&cmd[4]) == this->reqLon && ReadUInt32(&cmd[8]) == this->reqLCID)
		{
			UOSInt strSize;
			if ((cmd[12] & 0x80) != 0)
			{
				strSize = (UOSInt)(((cmd[12] & 0x7f) << 8) | cmd[13]);
				cmd += 14;
			}
			else
			{
				strSize = cmd[12];
				cmd += 13;
			}
			if (strSize > 0)
			{
				if (this->reqBuffSize > strSize)
				{
					Text::StrConcatC(this->reqBuff, cmd, strSize);
				}
				else
				{
					Text::StrConcatC(this->reqBuff, cmd, this->reqBuffSize - 1);
				}
			}
			this->reqResult = true;
			this->reqEvt->Set();
		}
	}
	else if (cmdType == 3)
	{
		if (this->reqBuff && ReadInt32(&cmd[0]) == this->reqLat && ReadInt32(&cmd[4]) == this->reqLon && ReadUInt32(&cmd[8]) == this->reqLCID)
		{
			UOSInt strSize;
			if ((cmd[12] & 0x80) != 0)
			{
				strSize = (UOSInt)(((cmd[12] & 0x7f) << 8) | cmd[13]);
				cmd += 14;
			}
			else
			{
				strSize = cmd[12];
				cmd += 13;
			}
			if (strSize > 0)
			{
				if (this->reqBuffSize > strSize)
				{
					Text::StrConcatC(this->reqBuff, cmd, strSize);
				}
				else
				{
					Text::StrConcatC(this->reqBuff, cmd, this->reqBuffSize - 1);
				}
			}
			this->reqResult = true;
			this->reqEvt->Set();
		}
	}
	else if (cmdType == 4)
	{
		UInt8 buff[16];
		stm->Write(buff, this->protocol->BuildPacket(buff, 4, 0, buff, 0, 0));
	}
}

void Map::ReverseGeocoderServer::DataSkipped(IO::Stream *stm, void *cliObj, const UInt8 *buff, UOSInt buffSize)
{
}

Bool Map::ReverseGeocoderServer::IsError()
{
	return this->ctrl->IsError();
}
