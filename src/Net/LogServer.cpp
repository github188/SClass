#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ByteTool.h"
#include "IO/Path.h"
#include "Net/LogServer.h"
#include "Sync/MutexUsage.h"
#include "Text/MyString.h"
#include "Text/StringBuilderUTF8.h"

#define MAXSIZE 4096
#define MTU 2048
#define BUFFSIZE (MAXSIZE + MTU)

void __stdcall Net::LogServer::ConnHdlr(UInt32 *s, void *userObj)
{
	Net::LogServer *me = (Net::LogServer*)userObj;
	Net::TCPClient *cli;
	ClientStatus *cliStatus;
	Net::SocketUtil::AddressInfo addr;
	NEW_CLASS(cli, Net::TCPClient(me->sockf, s));
	cliStatus = MemAlloc(ClientStatus, 1);
	cliStatus->buff = MemAlloc(UInt8, BUFFSIZE);
	cliStatus->buffSize = 0;
	cli->GetRemoteAddr(&addr);
	cliStatus->status = me->GetIPStatus(&addr);
	me->cliMgr->AddClient(cli, cliStatus);
}

void __stdcall Net::LogServer::ClientEvent(Net::TCPClient *cli, void *userObj, void *cliData, Net::TCPClientMgr::TCPEventType evtType)
{
	Net::LogServer *me = (Net::LogServer*)userObj;
	if (evtType == Net::TCPClientMgr::TCP_EVENT_DISCONNECT)
	{
		ClientStatus *cliStatus;
		if (me->log)
		{
			me->log->LogMessage((const UTF8Char*)"Client Disconnected", IO::ILogHandler::LOG_LEVEL_COMMAND);
		}
		cliStatus = (ClientStatus*)cliData;
		MemFree(cliStatus->buff);
		MemFree(cliStatus);
		DEL_CLASS(cli);
	}
	else if (evtType == Net::TCPClientMgr::TCP_EVENT_HASDATA)
	{
	}
}

void __stdcall Net::LogServer::ClientData(Net::TCPClient *cli, void *userObj, void *cliData, const UInt8 *buff, UOSInt size)
{
	Net::LogServer *me = (Net::LogServer*)userObj;
	ClientStatus *cliStatus;
	cliStatus = (ClientStatus*)cliData;
	if (size > BUFFSIZE)
	{
		MemCopyNO(cliStatus->buff, &buff[size - BUFFSIZE], BUFFSIZE);
		cliStatus->buffSize = BUFFSIZE;
	}
	else if (cliStatus->buffSize + size > BUFFSIZE)
	{
		MemCopyO(cliStatus->buff, &cliStatus->buff[cliStatus->buffSize - BUFFSIZE + size], BUFFSIZE - size);
		MemCopyNO(&cliStatus->buff[BUFFSIZE - size], buff, size);
		cliStatus->buffSize = BUFFSIZE;
	}
	else
	{
		MemCopyNO(&cliStatus->buff[cliStatus->buffSize], buff, size);
		cliStatus->buffSize += size;
	}

	UOSInt sizeLeft = me->protoHdlr->ParseProtocol(cli, cliStatus, 0, cliStatus->buff, cliStatus->buffSize);
	if (sizeLeft <= 0)
	{
		cliStatus->buffSize = 0;
	}
	else if (sizeLeft < cliStatus->buffSize)
	{
		MemCopyO(cliStatus->buff, &cliStatus->buff[cliStatus->buffSize - sizeLeft], sizeLeft);
		cliStatus->buffSize = sizeLeft;
	}
}

void __stdcall Net::LogServer::ClientTimeout(Net::TCPClient *cli, void *userObj, void *cliData)
{
}

Net::LogServer::IPStatus *Net::LogServer::GetIPStatus(const Net::SocketUtil::AddressInfo *addr)
{
	UTF8Char sbuff[512];
	UTF8Char *sptr;

	if (addr->addrType == Net::SocketUtil::AT_IPV4)
	{
		UInt32 ip = ReadMUInt32(addr->addr);
		IPStatus *status;
		Sync::MutexUsage mutUsage(this->ipMut);
		status = this->ipMap->Get(ip);
		if (status)
		{
			mutUsage.EndUse();
			return status;
		}
		status = MemAlloc(IPStatus, 1);
		status->ip = ReadNUInt32(addr->addr);
		sptr = Text::StrConcat(sbuff, this->logPath);
		if (sptr[-1] != IO::Path::PATH_SEPERATOR)
		{
			*sptr++ = IO::Path::PATH_SEPERATOR;
		}
		sptr = Net::SocketUtil::GetAddrName(sptr, addr);
		*sptr++ = IO::Path::PATH_SEPERATOR;
		sptr = Text::StrConcat(sptr, (const UTF8Char*)"Log");
		NEW_CLASS(status->log, IO::LogTool());
		status->log->AddFileLog(sbuff, IO::ILogHandler::LOG_TYPE_PER_DAY, IO::ILogHandler::LOG_GROUP_TYPE_PER_MONTH, IO::ILogHandler::LOG_LEVEL_RAW, "yyyy-MM-dd HH:mm:ss.fff", false);
		this->ipMap->Put(ip, status);
		mutUsage.EndUse();
		return status;
	}
	return 0;
}

Net::LogServer::LogServer(Net::SocketFactory *sockf, UInt16 port, const UTF8Char *logPath, IO::LogTool *svrLog, Bool redirLog)
{
	this->sockf = sockf;
	this->logPath = Text::StrCopyNew(logPath);
	this->log = svrLog;
	this->redirLog = redirLog;
	this->logHdlr = 0;
	this->logHdlrObj = 0;
	NEW_CLASS(this->ipMut, Sync::Mutex());
	NEW_CLASS(this->ipMap, Data::UInt32Map<IPStatus*>());
	NEW_CLASS(this->protoHdlr, IO::ProtoHdlr::ProtoLogCliHandler(this));
	NEW_CLASS(this->cliMgr, Net::TCPClientMgr(240, ClientEvent, ClientData, this, 4, ClientTimeout));
	NEW_CLASS(this->svr, Net::TCPServer(this->sockf, port, log, ConnHdlr, this, 0));
}

Net::LogServer::~LogServer()
{
	DEL_CLASS(this->svr);
	DEL_CLASS(this->cliMgr);
	DEL_CLASS(this->protoHdlr);
	Text::StrDelNew(this->logPath);
	UOSInt i;
	Data::ArrayList<IPStatus*> *ipList = this->ipMap->GetValues();
	IPStatus *status;
	i = ipList->GetCount();
	while (i-- > 0)
	{
		status = ipList->GetItem(i);
		DEL_CLASS(status->log);
		MemFree(status);
	}
	DEL_CLASS(this->ipMap);
	DEL_CLASS(this->ipMut);
}

Bool Net::LogServer::IsError()
{
	return this->svr->IsV4Error();
}

void Net::LogServer::HandleClientLog(ClientLogHandler hdlr, void *userObj)
{
	this->logHdlrObj = userObj;
	this->logHdlr = hdlr;
}

void Net::LogServer::DataParsed(IO::Stream *stm, void *stmObj, Int32 cmdType, Int32 seqId, const UInt8 *cmd, UOSInt cmdSize)
{
	UInt8 reply[18];
	UOSInt replySize;
	UTF8Char sbuff[256];
	ClientStatus *cliStatus;
	cliStatus = (ClientStatus*)stmObj;
	switch (cmdType)
	{
	case 0: //KA
		replySize = this->protoHdlr->BuildPacket(reply, 1, seqId, 0, 0, 0);
		stm->Write(reply, replySize);
		break;
	case 2: //Log Message
		replySize = this->protoHdlr->BuildPacket(reply, 3, seqId, cmd, 8, 0);
		stm->Write(reply, replySize);
		if (this->redirLog)
		{
			Text::StringBuilderUTF8 sb;
			if (this->log)
			{
				((Net::TCPClient *)stm)->GetRemoteName(sbuff);
				sb.Append(sbuff);
				sb.Append((const UTF8Char*)"> ");
				sb.AppendC(&cmd[8], cmdSize - 8);
				this->log->LogMessage(sb.ToString(), IO::ILogHandler::LOG_LEVEL_COMMAND);
			}

			if (cliStatus->status)
			{
				sb.ClearStr();
				sb.AppendC(&cmd[8], cmdSize - 8);
				cliStatus->status->log->LogMessage(sb.ToString(), IO::ILogHandler::LOG_LEVEL_COMMAND);

				if (this->logHdlr)
				{
					this->logHdlr(this->logHdlrObj, cliStatus->status->ip, sb.ToString());
				}
			}
		}
		else if (cmdSize - 8 < 256)
		{
			if (cliStatus->status)
			{
				MemCopyNO(sbuff, &cmd[8], cmdSize - 8);
				sbuff[cmdSize - 8] = 0;
				cliStatus->status->log->LogMessage(sbuff, IO::ILogHandler::LOG_LEVEL_COMMAND);

				if (this->logHdlr)
				{
					this->logHdlr(this->logHdlrObj, cliStatus->status->ip, sbuff);
				}
			}
		}
		else
		{
			if (cliStatus->status)
			{
				UInt8 *tmpPtr = MemAlloc(UInt8, cmdSize - 8 + 1);
				MemCopyNO(tmpPtr, &cmd[8], cmdSize - 8);
				tmpPtr[cmdSize - 8] = 0;
				cliStatus->status->log->LogMessage(tmpPtr, IO::ILogHandler::LOG_LEVEL_COMMAND);

				if (this->logHdlr)
				{
					this->logHdlr(this->logHdlrObj, cliStatus->status->ip, tmpPtr);
				}
				MemFree(tmpPtr);
			}
		}
		break;
	}
}

void Net::LogServer::DataSkipped(IO::Stream *stm, void *stmObj, const UInt8 *buff, UOSInt buffSize)
{

}
