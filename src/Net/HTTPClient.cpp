#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ArrayList.h"
#include "Data/DateTime.h"
#include "IO/Stream.h"
#include "Net/HTTPClient.h"
#include "Net/HTTPMyClient.h"
#include "Net/HTTPOSClient.h"
#include "Net/SocketFactory.h"
#include "Net/TCPClient.h"
#include "Sync/Event.h"
#include "Text/Encoding.h"
#include "Text/EncodingFactory.h"
#include "Text/MyString.h"
#include "Text/StringBuilderUTF8.h"
#include "Text/TextEnc/FormEncoding.h"
#include "Text/TextEnc/URIEncoding.h"

Net::HTTPClient::HTTPClient(Net::SocketFactory *sockf, Bool kaConn) : IO::Stream((const UTF8Char*)"HTTPClient")
{
	this->sockf = sockf;
	this->canWrite = false;
	this->contLeng = 0;
	this->respStatus = Net::WebStatus::SC_UNKNOWN;
	this->url = 0;
	this->hasForm = false;
	this->formSb = 0;
	this->kaConn = kaConn;
	this->svrAddr.addrType = Net::SocketUtil::AT_UNKNOWN;
	NEW_CLASS(this->headers, Data::ArrayList<UTF8Char*>());
	NEW_CLASS(this->clk, Manage::HiResClock());
}

Net::HTTPClient::~HTTPClient()
{
	if (this->headers)
	{
		UOSInt i = this->headers->GetCount();
		while (i-- > 0)
		{
			MemFree(this->headers->RemoveAt(i));
		}
		DEL_CLASS(this->headers);
	}
	SDEL_CLASS(this->formSb);
	SDEL_TEXT(this->url);
	DEL_CLASS(this->clk);
}

Bool Net::HTTPClient::FormBegin()
{
	if (this->canWrite && !this->hasForm)
	{
		this->hasForm = true;
		this->AddContentType((const UTF8Char*)"application/x-www-form-urlencoded");
		NEW_CLASS(this->formSb, Text::StringBuilderUTF8());
		return true;
	}
	return false;
}

Bool Net::HTTPClient::FormAdd(const UTF8Char *name, const UTF8Char *value)
{
	if (!this->hasForm)
	{
		return false;
	}
	UTF8Char sbuff[256];
	Text::TextEnc::URIEncoding::URIEncode(sbuff, name);
	if (this->formSb->GetLength() > 0)
	{
		this->formSb->AppendChar('&', 1);
	}
	this->formSb->Append(sbuff);
	Text::TextEnc::URIEncoding::URIEncode(sbuff, value);
	this->formSb->AppendChar('=', 1);
	this->formSb->Append(sbuff);
	return true;
}

void Net::HTTPClient::AddTimeHeader(const UTF8Char *name, Data::DateTime *dt)
{
	UTF8Char sbuff[64];
	Date2Str(sbuff, dt);
	this->AddHeader(name, sbuff);
}

void Net::HTTPClient::AddContentType(const UTF8Char *contType)
{
	this->AddHeader((const UTF8Char*)"Content-Type", contType);
}

void Net::HTTPClient::AddContentLength(UOSInt leng)
{
	UTF8Char sbuff[32];
	Text::StrUOSInt(sbuff, leng);
	this->AddHeader((const UTF8Char*)"Content-Length", sbuff);
}

UOSInt Net::HTTPClient::GetRespHeaderCnt()
{
	return this->headers->GetCount();
}

UTF8Char *Net::HTTPClient::GetRespHeader(UOSInt index, UTF8Char *buff)
{
	return Text::StrConcat(buff, this->headers->GetItem(index));
}

UTF8Char *Net::HTTPClient::GetRespHeader(const UTF8Char *name, UTF8Char *valueBuff)
{
	UTF8Char buff[256];
	UTF8Char *s2;
	UTF8Char *s;
	UOSInt i;
	s2 = Text::StrConcat(Text::StrConcat(buff, name), (const UTF8Char*)": ");
	i = this->headers->GetCount();
	while (i-- > 0)
	{
		s = (UTF8Char*)this->headers->GetItem(i);
		if (Text::StrStartsWith(s, buff))
		{
			return Text::StrConcat(valueBuff, &s[s2 - buff]);
		}
	}
	return 0;
}

Bool Net::HTTPClient::GetRespHeader(const UTF8Char *name, Text::StringBuilderUTF *sb)
{
	UTF8Char buff[256];
	UTF8Char *s2;
	UTF8Char *s;
	UOSInt i;
	s2 = Text::StrConcat(Text::StrConcat(buff, name), (const UTF8Char*)": ");
	i = this->headers->GetCount();
	while (i-- > 0)
	{
		s = (UTF8Char*)this->headers->GetItem(i);
		if (Text::StrStartsWithICase(s, buff))
		{
			sb->Append(&s[s2-buff]);
			return true;
		}
	}
	return false;
}

UInt64 Net::HTTPClient::GetContentLength()
{
	this->EndRequest(0, 0);
	return this->contLeng;
}

UInt32 Net::HTTPClient::GetContentCodePage()
{
	UTF8Char sbuff[256];
	UTF8Char *sarr[2];
	UOSInt arrCnt;
	this->EndRequest(0, 0);
	if (this->GetRespHeader((const UTF8Char*)"Content-Type", sbuff))
	{
		sarr[1] = sbuff;
		arrCnt = 2;
		while (arrCnt > 1)
		{
			arrCnt = Text::StrSplit(sarr, 2, sarr[1], ';');
			if (Text::StrStartsWith(sarr[0], (const UTF8Char*)"charset="))
			{
				Text::EncodingFactory encFact;
				return encFact.GetCodePage(&sarr[0][8]);
			}
		}
	}
	return 0;
}

Bool Net::HTTPClient::GetLastModified(Data::DateTime *dt)
{
	UTF8Char sbuff[64];
	this->EndRequest(0, 0);
	if (this->GetRespHeader((const UTF8Char*)"Last-Modified", sbuff))
	{
		ParseDateStr(dt, sbuff);
		return true;
	}
	return false;
}

const UTF8Char *Net::HTTPClient::GetURL()
{
	return this->url;
}

Net::WebStatus::StatusCode Net::HTTPClient::GetRespStatus()
{
	this->EndRequest(0, 0);
	return this->respStatus;
}

Double Net::HTTPClient::GetTotalTime()
{
	return this->clk->GetTimeDiff();
}

const Net::SocketUtil::AddressInfo *Net::HTTPClient::GetSvrAddr()
{
	return &this->svrAddr;
}

void Net::HTTPClient::ParseDateStr(Data::DateTime *dt, const UTF8Char *dateStr)
{
	UTF8Char *tmps;
	UTF8Char *ptrs[6];
	UTF8Char *ptrs2[3];
	UTF8Char *ptrs3[3];
	UTF8Char sbuff[64];
	UOSInt i;
	UOSInt j;
	if ((i = Text::StrIndexOf(dateStr, (const UTF8Char*)", ")) != INVALID_INDEX)
	{
		Text::StrConcat(sbuff, &dateStr[i + 2]);
		tmps = sbuff;
		if (Text::StrIndexOf(tmps, '-') == INVALID_INDEX)
		{
			i = Text::StrSplit(ptrs, 6, tmps, ' ');
			if (i >= 4)
			{
				j = Text::StrSplit(ptrs2, 3, ptrs[3], ':');
				if (j == 3)
				{
					dt->SetValue((UInt16)Text::StrToUInt32(ptrs[2]), Data::DateTime::ParseMonthStr(ptrs[1]), Text::StrToInt32(ptrs[0]), Text::StrToInt32(ptrs2[0]), Text::StrToInt32(ptrs2[1]), Text::StrToInt32(ptrs2[2]), 0);
				}
			}
		}
		else
		{
			i = Text::StrSplit(ptrs, 6, tmps, ' ');
			if (i >= 2)
			{
				Text::StrSplit(ptrs2, 3, ptrs[1], ':');
				Text::StrSplit(ptrs3, 3, ptrs[0], '-');
				dt->SetValue((UInt16)(Text::StrToUInt32(ptrs3[2]) + (UInt32)((dt->GetYear() / 100) * 100)), Data::DateTime::ParseMonthStr(ptrs3[1]), Text::StrToInt32(ptrs3[0]), Text::StrToInt32(ptrs2[0]), Text::StrToInt32(ptrs2[1]), Text::StrToInt32(ptrs2[2]), 0);
			}
		}
	}
	else
	{
		Text::StrConcat(sbuff, dateStr);
		i = Text::StrSplit(ptrs, 6, sbuff, ' ');
		if (i > 3)
		{
			j = Text::StrSplit(ptrs2, 3, ptrs[i - 2], ':');
			if (j == 3)
			{
				dt->SetValue((UInt16)Text::StrToUInt32(ptrs[i - 1]), Data::DateTime::ParseMonthStr(ptrs[1]), Text::StrToInt32(ptrs[i - 3]), Text::StrToInt32(ptrs2[0]), Text::StrToInt32(ptrs2[1]), Text::StrToInt32(ptrs2[2]), 0);
			}
		}
	}
}

UTF8Char *Net::HTTPClient::Date2Str(UTF8Char *sbuff, Data::DateTime *dt)
{
	const Char *wds[] = {"Sun, ", "Mon, ", "Tue, ", "Wed, ", "Thu, ", "Fri, ", "Sat, "};
	Data::DateTime t;
	t.SetValue(dt);
	t.ToUTCTime();
	Int32 wd = t.GetWeekday();
	return Text::StrConcat(t.ToString(Text::StrConcat(sbuff, (const UTF8Char*)wds[wd]), "dd MMM yyyy HH:mm:ss"), (const UTF8Char*)" GMT");
}

Net::HTTPClient *Net::HTTPClient::CreateClient(Net::SocketFactory *sockf, Net::SSLEngine *ssl, const UTF8Char *userAgent, Bool kaConn, Bool isSecure)
{
	Net::HTTPClient *cli;
	if (isSecure && ssl == 0)
	{
		NEW_CLASS(cli, Net::HTTPOSClient(sockf, userAgent, kaConn));
	}
	else
	{
		NEW_CLASS(cli, Net::HTTPMyClient(sockf, ssl, userAgent, kaConn));
	}
	return cli;
}

Net::HTTPClient *Net::HTTPClient::CreateConnect(Net::SocketFactory *sockf, Net::SSLEngine *ssl, const UTF8Char *url, const Char *method, Bool kaConn)
{
	if (method == 0)
	{
		method = "GET";
	}
	Net::HTTPClient *cli = Net::HTTPClient::CreateClient(sockf, ssl, 0, kaConn, Text::StrStartsWithICase(url, (const UTF8Char*)"HTTPS://"));
	cli->Connect(url, method, 0, 0, true);
	return cli;
}
