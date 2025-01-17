#include "Stdafx.h"
#include "MyMemory.h"
#include "Net/WebServer/EGaugeHandler.h"

Net::WebServer::EGaugeHandler::EGaugeHandler()
{
	this->dataHdlr = 0;
	this->dataHdlrObj = 0;
}

Net::WebServer::EGaugeHandler::~EGaugeHandler()
{
}

Bool Net::WebServer::EGaugeHandler::ProcessRequest(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq)
{
	if (this->DoRequest(req, resp, subReq))
	{
		return true;
	}

	if (req->GetProtocol() != Net::WebServer::IWebRequest::REQPROTO_HTTP1_0 && req->GetProtocol() != Net::WebServer::IWebRequest::REQPROTO_HTTP1_1)
	{
		resp->ResponseError(req, Net::WebStatus::SC_METHOD_NOT_ALLOWED);
		return true;
	}
	if (req->GetReqMethod() != Net::WebServer::IWebRequest::REQMETH_HTTP_POST)
	{
		resp->ResponseError(req, Net::WebStatus::SC_NOT_ACCEPTABLE);
		return true;
	}
	UOSInt buffSize;
	const UTF8Char *buff = req->GetReqData(&buffSize);
	resp->SetStatusCode(Net::WebStatus::SC_OK);
	resp->AddContentLength(2);
	resp->AddContentType((const UTF8Char*)"text/plain");
	resp->Write((const UInt8*)"ok", 2);
	
	if (this->dataHdlr)
	{
		this->dataHdlr(this->dataHdlrObj, buff, buffSize);
	}
	return true;
}

void Net::WebServer::EGaugeHandler::HandleEGaugeData(DataHandler dataHdlr, void *userObj)
{
	this->dataHdlrObj = userObj;
	this->dataHdlr = dataHdlr;
}
