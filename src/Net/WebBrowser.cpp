#include "Stdafx.h"
#include "MyMemory.h"
#include "Crypto/Hash/CRC32R.h"
#include "IO/Path.h"
#include "IO/StmData/FileData.h"
#include "IO/StmData/MemoryData2.h"
#include "Net/HTTPData.h"
#include "Net/MIME.h"
#include "Net/WebBrowser.h"
#include "Text/MyString.h"
#include "Text/URLString.h"
#include "Text/TextBinEnc/Base64Enc.h"
#include "Text/TextEnc/URIEncoding.h"

UTF8Char *Net::WebBrowser::GetLocalFileName(UTF8Char *sbuff, const UTF8Char *url)
{
	UTF8Char buff[512];
	UTF8Char *sptr;
	UTF8Char *sptr2;
	UInt8 hashResult[4];
	sptr = Text::StrConcat(buff, this->cacheDir);
	if (sptr != buff && sptr[-1] != IO::Path::PATH_SEPERATOR)
	{
		*sptr++ = IO::Path::PATH_SEPERATOR;
	}
	sptr2 = Text::URLString::GetURIScheme(sptr, url);
	if (Text::StrCompareICase(sptr, (const UTF8Char*)"HTTP") == 0 || Text::StrCompareICase(sptr, (const UTF8Char*)"HTTPS") == 0)
	{
		sptr = sptr2;
		*sptr++ = IO::Path::PATH_SEPERATOR;
		sptr2 = Text::URLString::GetURLHost(sptr, url);
		Text::StrReplace(sptr, ':', '+');
		IO::Path::CreateDirectory(buff);
		sptr = sptr2;
		*sptr++ = IO::Path::PATH_SEPERATOR;
		url = &url[Text::StrIndexOf(url, ':') + 3];
		url = &url[Text::StrIndexOf(url, '/')];
		this->hash->Clear();
		this->hash->Calc((UInt8*)url, Text::StrCharCnt(url));
		this->hash->GetValue(hashResult);
		Text::StrHexBytes(sptr, hashResult, 4, 0);
		return Text::StrConcat(sbuff, buff);
	}
	else
	{
		return 0;
	}
}

Net::WebBrowser::WebBrowser(Net::SocketFactory *sockf, Net::SSLEngine *ssl, const UTF8Char *cacheDir)
{
	this->sockf = sockf;
	this->ssl = ssl;
	this->cacheDir = Text::StrCopyNew(cacheDir);
	NEW_CLASS(this->hash, Crypto::Hash::CRC32R(Crypto::Hash::CRC32R::GetPolynormialIEEE()));
	NEW_CLASS(this->queue, Net::HTTPQueue(sockf, ssl));
}

Net::WebBrowser::~WebBrowser()
{
	DEL_CLASS(this->queue);
	Text::StrDelNew(this->cacheDir);
	DEL_CLASS(this->hash);
}

IO::IStreamData *Net::WebBrowser::GetData(const UTF8Char *url, Bool forceReload, UTF8Char *contentType)
{
	UTF8Char sbuff[512];
	IO::Path::PathType pt = IO::Path::GetPathType(url);
	/////////////////////////////////////////////
	if (pt == IO::Path::PT_FILE)
	{
		IO::StmData::FileData *fd;
		NEW_CLASS(fd, IO::StmData::FileData(url, false));
		if (contentType)
		{
			Text::StrConcat(contentType, Net::MIME::GetMIMEFromExt(url));
		}
		return fd;
	}
	if (Text::URLString::GetURIScheme(sbuff, url) == 0)
		return 0;
	if (Text::StrCompareICase(sbuff, (const UTF8Char*)"FILE") == 0)
	{
		Text::URLString::GetURLFilePath(sbuff, url);
		IO::StmData::FileData *fd;
		NEW_CLASS(fd, IO::StmData::FileData(sbuff, false));
		if (contentType)
		{
			Text::StrConcat(contentType, Net::MIME::GetMIMEFromExt(url));
		}
		return fd;
	}
	else if (Text::StrCompareICase(sbuff, (const UTF8Char*)"HTTP") == 0)
	{
		Net::HTTPData *data;
		GetLocalFileName(sbuff, url);
		NEW_CLASS(data, Net::HTTPData(this->sockf, this->ssl, this->queue, url, sbuff, forceReload));
		return data;
	}
	else if (Text::StrCompareICase(sbuff, (const UTF8Char*)"HTTPS") == 0)
	{
		Net::HTTPData *data;
		GetLocalFileName(sbuff, url);
		NEW_CLASS(data, Net::HTTPData(this->sockf, this->ssl, this->queue, url, sbuff, forceReload));
		return data;
	}
	else if (Text::StrCompareICase(sbuff, (const UTF8Char*)"FTP") == 0)
	{
/*		IO::Stream *stm = Net::URL::OpenStream(url, this->sockf);
		IO::StmData::StreamDataStream *data;
		if (stm == 0)
			return 0;
		NEW_CLASS(data, IO::StmData::*/
		return 0;
	}
	else if (Text::StrCompareICase(sbuff, (const UTF8Char*)"DATA") == 0)
	{
		IO::StmData::MemoryData2 *fd;
		WChar c;
		url = &url[5];
		if (contentType)
		{
			while (true)
			{
				c = *url++;
				if (c == 0)
				{
					*contentType = 0;
					return 0;
				}
				else if (c == ';')
				{
					break;
				}
				else
				{
					*contentType++ = (UTF8Char)c;
				}
			}
		}
		else
		{
			while (true)
			{
				c = *url++;
				if (c == 0)
				{
					return 0;
				}
				else if (c == ';')
				{
					break;
				}
			}
		}
		if (Text::StrStartsWith(url, (const UTF8Char*)"base64,"))
		{
			Text::TextBinEnc::Base64Enc b64;
			UOSInt textSize;
			UOSInt binSize;
			UTF8Char *strTemp;
			UInt8 *binTemp;
			textSize = Text::StrCharCnt(url + 7);
			strTemp = MemAlloc(UTF8Char, textSize + 1);
			Text::TextEnc::URIEncoding::URIDecode(strTemp, url + 7);
			binSize = b64.CalcBinSize(strTemp);
			binTemp = MemAlloc(UInt8, binSize);
			b64.DecodeBin(strTemp, binTemp);
			NEW_CLASS(fd, IO::StmData::MemoryData2(binTemp, binSize));
			MemFree(binTemp);
			MemFree(strTemp);
			return fd;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}
