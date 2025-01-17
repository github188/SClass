#include "Stdafx.h"
#include "MyMemory.h"
#include "IO/Path.h"
#include "Text/MyString.h"
#include "Text/MyStringW.h"
#include "Text/TextEnc/URIEncoding.h"
#include "Text/URLString.h"

UTF8Char *Text::URLString::GetURLFilePath(UTF8Char *sbuff, const UTF8Char *url)
{
	if (!Text::StrStartsWithICase(url, (const UTF8Char*)"FILE:///"))
		return 0;
	if (IO::Path::PATH_SEPERATOR == '\\')
	{
		UTF8Char *sptr = Text::TextEnc::URIEncoding::URIDecode(sbuff, &url[8]);
		Text::StrReplace(sbuff, '/', '\\');
		return sptr;
	}
	else
	{
		return Text::TextEnc::URIEncoding::URIDecode(sbuff, &url[7]);
	}
}

UTF8Char *Text::URLString::GetURLDomain(UTF8Char *sbuff, const UTF8Char *url, UInt16 *port)
{
	UOSInt i;
	UOSInt j;
	UOSInt k;
	i = Text::StrIndexOf(url, (const UTF8Char*)"://");
	if (i != INVALID_INDEX)
	{
		url = &url[i + 3];
	}
	k = Text::StrIndexOf(url, '@');
	i = Text::StrIndexOf(url, '/');
	if (k != INVALID_INDEX && i != INVALID_INDEX && k < i)
	{
		url = &url[k + 1];
		i -= k + 1;
	}
	j = Text::StrIndexOf(url, ':');
	if (i != INVALID_INDEX && j != INVALID_INDEX && j < i)
	{
		if (port)
		{
			MemCopyNO(sbuff, &url[j + 1], (i - j - 1) * sizeof(UTF8Char));
			sbuff[i - j - 1] = 0;
			Text::StrToUInt16S(sbuff, port, 0);
		}
		if (i < j)
		{
			MemCopyNO(sbuff, url, sizeof(UTF8Char) * i);
			sbuff[i] = 0;
			return &sbuff[i];
		}
		else
		{
			MemCopyNO(sbuff, url, sizeof(UTF8Char) * j);
			sbuff[j] = 0;
			return &sbuff[j];
		}
	}
	else if (i != INVALID_INDEX)
	{
		if (port)
		{
			*port = 0;
		}
		MemCopyNO(sbuff, url, sizeof(UTF8Char) * i);
		sbuff[i] = 0;
		return &sbuff[i];
	}
	else if (j != INVALID_INDEX)
	{
		if (port)
		{
			Text::StrToUInt16S(&url[j + 1], port, 0);
		}
		MemCopyNO(sbuff, url, sizeof(UTF8Char) * j);
		sbuff[j] = 0;
		return &sbuff[j];
	}
	else
	{
		if (port)
		{
			*port = 0;
		}
		return Text::StrConcat(sbuff, url);
	}
}

UTF8Char *Text::URLString::GetURIScheme(UTF8Char *sbuff, const UTF8Char *url)
{
	UOSInt i = Text::StrIndexOf(url, ':');
	if (i == INVALID_INDEX)
	{
		return 0;
	}
	MemCopyNO(sbuff, url, (UOSInt)i * sizeof(UTF8Char));
	sbuff[i] = 0;
	return &sbuff[i];
}

UTF8Char *Text::URLString::GetURLHost(UTF8Char *sbuff, const UTF8Char *url)
{
	UOSInt i;
	i = Text::StrIndexOf(url, (const UTF8Char*)"://");
	if (i != INVALID_INDEX)
	{
		url = &url[i + 3];
	}
	i = Text::StrIndexOf(url, '/');
	if (i != INVALID_INDEX)
	{
		MemCopyNO(sbuff, url, sizeof(UTF8Char) * i);
		sbuff[i] = 0;
		return &sbuff[i];
	}
	else
	{
		return Text::StrConcat(sbuff, url);
	}
}

UTF8Char *Text::URLString::GetURLPath(UTF8Char *sbuff, const UTF8Char *url)
{
	UOSInt i;
	UTF8Char *tmpBuff;
	UOSInt urlLen = Text::StrCharCnt(url);
	i = Text::StrIndexOf(url, (const UTF8Char*)"://");
	if (i != INVALID_INDEX)
	{
		url = &url[i + 3];
	}
	i = Text::StrIndexOf(url, '/');
	if (i != INVALID_INDEX)
	{
		tmpBuff = MemAlloc(UTF8Char, urlLen + 1);
		UTF8Char *sptr = Text::TextEnc::URIEncoding::URIDecode(tmpBuff, &url[i]);
		i = Text::StrIndexOf(tmpBuff, '?');
		if (i != INVALID_INDEX)
		{
			sptr = &tmpBuff[i];
			*sptr = 0;
		}
		i = Text::StrIndexOf(tmpBuff, '#');
		if (i != INVALID_INDEX)
		{
			sptr = &tmpBuff[i];
			*sptr = 0;
		}
		sptr = Text::StrConcat(sbuff, tmpBuff);
		MemFree(tmpBuff);
		return sptr;
	}
	else
	{
		return Text::StrConcat(sbuff, (const UTF8Char*)"/");
	}
}

UTF8Char *Text::URLString::GetURLPathSvr(UTF8Char *sbuff, const UTF8Char *url)
{
	UOSInt i;
	UTF8Char *tmpBuff;
	UOSInt urlLen = Text::StrCharCnt(url);
	i = Text::StrIndexOf(url, (const UTF8Char*)"://");
	if (i != INVALID_INDEX)
	{
		url = &url[i + 3];
	}
	i = Text::StrIndexOf(url, '/');
	if (i != INVALID_INDEX)
	{
		tmpBuff = MemAlloc(UTF8Char, urlLen + 1);
		UTF8Char *sptr = Text::TextEnc::URIEncoding::URIDecode(tmpBuff, &url[i]);
		i = Text::StrIndexOf(tmpBuff, '?');
		if (i != INVALID_INDEX)
		{
			sptr = &tmpBuff[i];
			*sptr = 0;
		}
		sptr = Text::StrConcat(sbuff, tmpBuff);
		MemFree(tmpBuff);
		return sptr;
	}
	else
	{
		return Text::StrConcat(sbuff, (const UTF8Char*)"/");
	}
}

UTF8Char *Text::URLString::AppendURLPath(UTF8Char *sbuff, const UTF8Char *path)
{
	UOSInt i = Text::StrIndexOf(path, (const UTF8Char*)"://");
	if (i != INVALID_INDEX)
	{
		return Text::StrConcat(sbuff, path);
	}
	if (sbuff[0] != 0)
	{
		if (sbuff[1] == ':' && sbuff[2] == '\\')
		{
			UTF8Char *sptr = IO::Path::AppendPath(sbuff, path);
			Text::StrReplace(sbuff, '/', '\\');
			return sptr;
		}
		IO::Path::PathType pt = IO::Path::GetPathType(sbuff);
		if (pt != IO::Path::PT_UNKNOWN)
		{
			return IO::Path::AppendPath(sbuff, path);
		}
	}
	i = Text::StrIndexOf(sbuff, (const UTF8Char*)"://");
	if (i == INVALID_INDEX)
		return 0;
	sbuff = &sbuff[3];
	if (path[0] == '/')
	{
		i = Text::StrIndexOf(sbuff, (const UTF8Char*)"/");
		if (i == INVALID_INDEX)
		{
			return Text::StrConcat(&sbuff[Text::StrCharCnt(sbuff)], path);
		}
		else
		{
			return Text::StrConcat(&sbuff[i], path);
		}
	}
	else
	{
		while (path[0] == '.' && path[1] == '.' && path[2] == '/')
		{
			i = Text::StrLastIndexOf(sbuff, '/');
			if (i != INVALID_INDEX)
			{
				sbuff[i] = 0;
			}
			path = &path[3];
		}
		i = Text::StrLastIndexOf(sbuff, '/');
		if (i == INVALID_INDEX)
		{
			return Text::StrConcat(&sbuff[Text::StrCharCnt(sbuff)], path);
		}
		else
		{
			return Text::StrConcat(&sbuff[i + 1], path);
		}
	}
	////////////////////////////////
}
