#include "Stdafx.h"
#include "MyMemory.h"
#include "Net/WebServer/WebListener.h"
#include "Net/WebServer/WebConnection.h"
#include "Sync/MutexUsage.h"
#include "Text/MyString.h"
#include "Text/MyStringFloat.h"
#include "Text/TextEnc/FormEncoding.h"
#include "Text/TextEnc/URIEncoding.h"

void __stdcall Net::WebServer::WebListener::ClientReady(Net::TCPClient *cli, void *userObj)
{
	Net::WebServer::WebListener *me = (Net::WebServer::WebListener*)userObj;
	Net::WebServer::WebConnection *conn;
	NEW_CLASS(conn, Net::WebServer::WebConnection(me->sockf, me->ssl, cli, me, me->hdlr, me->allowProxy, me->allowKA));
	conn->SetSendLogger(OnDataSent, me);
	me->cliMgr->AddClient(cli, conn);
}

void __stdcall Net::WebServer::WebListener::ConnHdlr(UInt32 *s, void *userObj)
{
	Net::WebServer::WebListener *me = (Net::WebServer::WebListener*)userObj;
	Net::TCPClient *cli;
	if (me->ssl)
	{
		me->ssl->ServerInit(s, ClientReady, me);
	}
	else
	{
		NEW_CLASS(cli, Net::TCPClient(me->sockf, s));
		ClientReady(cli, me);
	}
	Sync::MutexUsage mutUsage(me->statusMut);
	me->status.connCnt++;
	mutUsage.EndUse();
}

void __stdcall Net::WebServer::WebListener::ClientEvent(Net::TCPClient *cli, void *userObj, void *cliData, Net::TCPClientMgr::TCPEventType evtType)
{
//	Net::WebServer::WebListener *me = (Net::WebServer::WebListener*)userObj;
	if (evtType == Net::TCPClientMgr::TCP_EVENT_DISCONNECT)
	{
		Net::WebServer::WebConnection *conn = (Net::WebServer::WebConnection*)cliData;
		DEL_CLASS(conn);
	}
	else if (evtType == Net::TCPClientMgr::TCP_EVENT_SHUTDOWN)
	{
		Net::WebServer::WebConnection *conn = (Net::WebServer::WebConnection*)cliData;
		conn->ProxyShutdown();
	}
}

void __stdcall Net::WebServer::WebListener::ClientData(Net::TCPClient *cli, void *userObj, void *cliData, const UInt8 *buff, UOSInt size)
{
	Net::WebServer::WebListener *me = (Net::WebServer::WebListener*)userObj;
	Net::WebServer::WebConnection *conn = (Net::WebServer::WebConnection*)cliData;
	conn->ReceivedData(buff, size);
	Sync::MutexUsage mutUsage(me->statusMut);
	me->status.totalRead += size;
	mutUsage.EndUse();
}

void __stdcall Net::WebServer::WebListener::ClientTimeout(Net::TCPClient *cli, void *userObj, void *cliData)
{
	Net::WebServer::WebListener *me = (Net::WebServer::WebListener *)userObj;
	Net::WebServer::WebConnection *conn = (Net::WebServer::WebConnection*)cliData;
	conn->ProcessTimeout();
	if (me->timeoutHdlr)
	{
		me->timeoutHdlr(me->timeoutObj, conn->GetRequestURL());
	}
}

void __stdcall Net::WebServer::WebListener::ProxyClientEvent(Net::TCPClient *cli, void *userObj, void *cliData, Net::TCPClientMgr::TCPEventType evtType)
{
//	Net::WebServer::WebListener *me = (Net::WebServer::WebListener*)userObj;
	if (evtType == Net::TCPClientMgr::TCP_EVENT_DISCONNECT)
	{
		Net::WebServer::WebConnection *conn = (Net::WebServer::WebConnection*)cliData;
		conn->EndProxyConn();
	}
	else if (evtType == Net::TCPClientMgr::TCP_EVENT_SHUTDOWN)
	{
		Net::WebServer::WebConnection *conn = (Net::WebServer::WebConnection*)cliData;
		conn->ShutdownSend();
	}
}

void __stdcall Net::WebServer::WebListener::ProxyClientData(Net::TCPClient *cli, void *userObj, void *cliData, const UInt8 *buff, UOSInt size)
{
//	Net::WebServer::WebListener *me = (Net::WebServer::WebListener*)userObj;
	Net::WebServer::WebConnection *conn = (Net::WebServer::WebConnection*)cliData;
	conn->ProxyData(buff, size);
}

void __stdcall Net::WebServer::WebListener::ProxyTimeout(Net::TCPClient *cli, void *userObj, void *cliData)
{

}

void __stdcall Net::WebServer::WebListener::OnDataSent(void *userObj, UOSInt buffSize)
{
	Net::WebServer::WebListener *me = (Net::WebServer::WebListener*)userObj;
	Sync::MutexUsage mutUsage(me->statusMut);
	me->status.totalWrite += buffSize;
	mutUsage.EndUse();
}

Net::WebServer::WebListener::WebListener(Net::SocketFactory *sockf, Net::SSLEngine *ssl, IWebHandler *hdlr, UInt16 port, Int32 timeoutSeconds, UOSInt workerCnt, const UTF8Char *svrName, Bool allowProxy, Bool allowKA)
{
	this->hdlr = hdlr;

	this->sockf = sockf;
	this->accLog = 0;
	this->reqLog = 0;
	this->ssl = ssl;
	this->allowProxy = allowProxy;
	this->allowKA = allowKA;
	this->timeoutHdlr = 0;
	this->timeoutObj = 0;
	this->proxyCliMgr = 0;
	if (svrName)
	{
		this->svrName = Text::StrCopyNew(svrName);
	}
	else
	{
		this->svrName = Text::StrCopyNew((const UTF8Char*)"sswr/1.0");
	}
	this->status.connCnt = 0;
	this->status.currConn = 0;
	this->status.reqCnt = 0;
	this->status.totalRead = 0;
	this->status.totalWrite = 0;
	NEW_CLASS(this->statusMut, Sync::Mutex());
	NEW_CLASS(this->accLogMut, Sync::Mutex());
	NEW_CLASS(this->log, IO::LogTool());
	NEW_CLASS(this->cliMgr, Net::TCPClientMgr(timeoutSeconds, ClientEvent, ClientData, this, workerCnt, ClientTimeout));
	NEW_CLASS(this->svr, Net::TCPServer(sockf, port, log, ConnHdlr, this, (const UTF8Char*)"Web: "));
	if (this->allowProxy)
	{
		NEW_CLASS(this->proxyCliMgr, Net::TCPClientMgr(240, ProxyClientEvent, ProxyClientData, this, workerCnt, ProxyTimeout));
	}
}

Net::WebServer::WebListener::~WebListener()
{
	DEL_CLASS(this->svr);
	DEL_CLASS(this->cliMgr);
	SDEL_CLASS(this->proxyCliMgr);
	DEL_CLASS(this->log);
	DEL_CLASS(this->accLogMut);
	DEL_CLASS(this->statusMut);
	Text::StrDelNew(this->svrName);
}

Bool Net::WebServer::WebListener::IsError()
{
	Bool err = this->svr->IsV4Error();
	return err;
}

const UTF8Char *Net::WebServer::WebListener::GetServerName()
{
	return this->svrName;
}

void Net::WebServer::WebListener::SetAccessLog(IO::LogTool *accLog, IO::ILogHandler::LogLevel accLogLev)
{
	Sync::MutexUsage mutUsage(this->accLogMut);
	this->accLog = accLog;
	this->accLogLev = accLogLev;
}

void Net::WebServer::WebListener::SetRequestLog(Net::WebServer::IReqLogger *reqLog)
{
	Sync::MutexUsage mutUsage(this->accLogMut);
	this->reqLog = reqLog;
}

void Net::WebServer::WebListener::LogAccess(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, Double time)
{
	UTF8Char sbuff[128];
	const UTF8Char *csptr;
	Sync::MutexUsage mutUsage(this->statusMut);
	this->status.reqCnt++;
	mutUsage.EndUse();
	Sync::MutexUsage accLogMutUsage(this->accLogMut);
	if (this->reqLog)
	{
		this->reqLog->LogRequest(req);
	}
	if (this->accLog)
	{
		Text::StringBuilderUTF8 sb;
		Net::SocketUtil::GetAddrName(sbuff, req->GetClientAddr(), req->GetClientPort());
		sb.Append(sbuff);
		sb.Append((const UTF8Char*)" ");
		sb.Append(req->GetReqMethodStr());
		sb.Append((const UTF8Char*)" ");
		sb.Append(req->GetRequestURI());
		sb.Append((const UTF8Char*)" \"");
		if (req->GetBrowser() == Net::BrowserInfo::BT_UNKNOWN)
		{
			if (!req->GetHeader(&sb, (const UTF8Char*)"User-Agent"))
			{
				sb.Append((const UTF8Char*)"Unk Browser");
			}
		}
		else
		{
			sb.Append(Net::BrowserInfo::GetName(req->GetBrowser()));
		}
		csptr = req->GetBrowserVer();
		if (csptr)
		{
			sb.Append((const UTF8Char*)" ");
			sb.Append(csptr);
		}
		sb.Append((const UTF8Char*)"\"");

		sb.Append((const UTF8Char*)" \"");
		Manage::OSInfo::GetCommonName(&sb, req->GetOS(), req->GetOSVer());
		sb.Append((const UTF8Char*)"\"");

		sb.Append((const UTF8Char*)" ");
		sb.AppendI32(resp->GetStatusCode());
		sb.Append((const UTF8Char*)" ");
		sb.AppendU64(resp->GetRespLength());
		sb.Append((const UTF8Char*)" ");
		Text::SBAppendF64(&sb, time);

		this->accLog->LogMessage(sb.ToString(), this->accLogLev);
	}
	accLogMutUsage.EndUse();
}

void Net::WebServer::WebListener::LogMessage(Net::WebServer::IWebRequest *req, const UTF8Char *msg)
{
	UTF8Char sbuff[32];
	Sync::MutexUsage mutUsage(this->accLogMut);
	if (this->accLog)
	{
		if (req)
		{
			Text::StringBuilderUTF8 sb;
			Net::SocketUtil::GetAddrName(sbuff, req->GetClientAddr(), req->GetClientPort());
			sb.Append(sbuff);
			sb.Append((const UTF8Char*)" ");
			sb.Append(msg);
			this->accLog->LogMessage(sb.ToString(), this->accLogLev);
		}
		else
		{
			this->accLog->LogMessage(msg, this->accLogLev);
		}
	}
}

void Net::WebServer::WebListener::AddProxyConn(Net::WebServer::WebConnection *conn, Net::TCPClient *proxyCli)
{
	if (this->proxyCliMgr)
	{
		this->proxyCliMgr->AddClient(proxyCli, conn);
	}
	else
	{
		conn->EndProxyConn();
	}
}

void Net::WebServer::WebListener::HandleTimeout(TimeoutHandler hdlr, void *userObj)
{
	this->timeoutObj = userObj;
	this->timeoutHdlr = hdlr;
}

void Net::WebServer::WebListener::ExtendTimeout(Net::TCPClient *cli)
{
	this->cliMgr->ExtendTimeout(cli);
}

void Net::WebServer::WebListener::GetStatus(SERVER_STATUS *status)
{
	Sync::MutexUsage mutUsage(this->statusMut);
	MemCopyNO(status, &this->status, sizeof(SERVER_STATUS));
	mutUsage.EndUse();
	status->currConn = (UInt32)this->cliMgr->GetClientCount();
}
