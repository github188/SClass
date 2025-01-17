#include "Stdafx.h"
#include "MyMemory.h"
#include "Text/TextBinEnc/QuotedPrintableEnc.h"

Text::TextBinEnc::QuotedPrintableEnc::QuotedPrintableEnc()
{
}

Text::TextBinEnc::QuotedPrintableEnc::~QuotedPrintableEnc()
{
}

UOSInt Text::TextBinEnc::QuotedPrintableEnc::EncodeBin(Text::StringBuilderUTF *sb, const UInt8 *dataBuff, UOSInt buffSize)
{
	UTF8Char sbuff[78];
	UOSInt lineCnt = 0;
	UInt8 b;
	UOSInt retCnt = 0;
	while (buffSize-- > 0)
	{
		b = *dataBuff++;
		if (b == 13 || b == 10)
		{
			sbuff[lineCnt] = b;
			sbuff[lineCnt + 1] = 0;
			sb->AppendC(sbuff, lineCnt + 1);
			retCnt += lineCnt + 1;
			lineCnt = 0;
		}
		else if (b >= 32 && b <= 126)
		{
			if (lineCnt < 75)
			{
				sbuff[lineCnt] = b;
				lineCnt++;
			}
			else
			{
				if (dataBuff[0] == 13 || dataBuff[0] == 10)
				{
					sbuff[lineCnt] = b;
					lineCnt++;
				}
				else
				{
					sbuff[lineCnt] = '=';
					sbuff[lineCnt + 1] = 13;
					sbuff[lineCnt + 2] = 10;
					sbuff[lineCnt + 3] = 0;
					sb->AppendC(sbuff, lineCnt + 2);
					retCnt += lineCnt + 2;
					sbuff[0] = b;
					lineCnt = 1;
				}
			}
		}
		else
		{
			if (lineCnt < 73)
			{
				sbuff[lineCnt] = '=';
				Text::StrHexByte(&sbuff[lineCnt + 1], b);
				lineCnt += 3;
			}
			else
			{
				if (dataBuff[0] == 13 || dataBuff[0] == 10)
				{
					sbuff[lineCnt] = '=';
					Text::StrHexByte(&sbuff[lineCnt + 1], b);
					lineCnt += 3;
				}
				else
				{
					sbuff[lineCnt] = '=';
					sbuff[lineCnt + 1] = 13;
					sbuff[lineCnt + 2] = 10;
					sbuff[lineCnt + 3] = 0;
					sb->AppendC(sbuff, lineCnt + 2);
					retCnt += lineCnt + 2;
					sbuff[0] = '=';
					Text::StrHexByte(&sbuff[1], b);
					lineCnt = 3;
				}
			}
		}
	}
	if (lineCnt != 0)
	{
		sbuff[lineCnt] = 0;
		sb->AppendC(sbuff, lineCnt);
		retCnt += lineCnt;
	}
	return retCnt;
}

UOSInt Text::TextBinEnc::QuotedPrintableEnc::CalcBinSize(const UTF8Char *sbuff)
{
	UOSInt cnt = 0;
	UTF8Char c;
	while ((c = *sbuff++) != 0)
	{
		if (c == '=')
		{
			if (sbuff[0] == '\r' && sbuff[1] == '\n')
			{
				sbuff += 2;
			}
			else if (sbuff[0] == '\r' || sbuff[0] == '\n')
			{
				sbuff += 1;
			}
			else
			{
				sbuff += 2;
				cnt++;
			}
		}
		else
		{
			cnt++;
		}
	}
	return cnt;
}

UOSInt Text::TextBinEnc::QuotedPrintableEnc::DecodeBin(const UTF8Char *b64Str, UInt8 *dataBuff)
{
	UOSInt cnt = 0;
	UTF8Char c;
	while ((c = *b64Str++) != 0)
	{
		if (c == '=')
		{
			if (b64Str[0] == '\r' && b64Str[1] == '\n')
			{
				b64Str += 2;
			}
			else if (b64Str[0] == '\r' || b64Str[0] == '\n')
			{
				b64Str += 1;
			}
			else
			{
				*dataBuff++ = Text::StrHex2UInt8C(b64Str);
				b64Str += 2;
				cnt++;
			}
		}
		else
		{
			*dataBuff++ = c;
			cnt++;
		}
	}
	return cnt;
}

UOSInt Text::TextBinEnc::QuotedPrintableEnc::DecodeBin(const UTF8Char *b64Str, UOSInt len, UInt8 *dataBuff)
{
	UOSInt cnt = 0;
	UTF8Char c;
	while (len-- > 0)
	{
		c = *b64Str++;
		if (c == '=')
		{
			if (len >= 2 && b64Str[0] == '\r' && b64Str[1] == '\n')
			{
				b64Str += 2;
				len -= 2;
			}
			else if (len >= 1 && (b64Str[0] == '\r' || b64Str[0] == '\n'))
			{
				b64Str += 1;
				len -= 1;
			}
			else if (len >= 2)
			{
				*dataBuff++ = Text::StrHex2UInt8C(b64Str);
				b64Str += 2;
				len -= 2;
				cnt++;
			}
		}
		else
		{
			*dataBuff++ = c;
			cnt++;
		}
	}
	return cnt;
}

const UTF8Char *Text::TextBinEnc::QuotedPrintableEnc::GetName()
{
	return (const UTF8Char*)"QuotedPrintable";
}
