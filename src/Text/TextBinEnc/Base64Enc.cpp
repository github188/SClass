#include "Stdafx.h"
#include "MyMemory.h"
#include "Text/TextBinEnc/Base64Enc.h"

const UInt8 Text::TextBinEnc::Base64Enc::decArr[] = {
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3e, 0xff, 0x3e, 0xff, 0x3f,
		0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
		0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xff, 0xff, 0xff, 0xff, 0x3f,
		0xff, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
		0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

const UTF8Char *Text::TextBinEnc::Base64Enc::GetEncArr(Charset cs)
{
	if (cs == CS_URL)
	{
		return (const UTF8Char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
	}
	else
	{
		return (const UTF8Char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	}
}

Text::TextBinEnc::Base64Enc::Base64Enc()
{
	this->cs = CS_NORMAL;
	this->noPadding = false;
}

Text::TextBinEnc::Base64Enc::Base64Enc(Charset cs, Bool noPadding)
{
	this->cs = cs;
	this->noPadding = noPadding;
}

Text::TextBinEnc::Base64Enc::~Base64Enc()
{
}

UOSInt Text::TextBinEnc::Base64Enc::EncodeBin(Text::StringBuilderUTF *sb, const UInt8 *dataBuff, UOSInt buffSize)
{
	const UTF8Char *encArr = GetEncArr(this->cs);
	UOSInt outSize;
	UTF8Char sptr[4];
	UOSInt tmp1 = buffSize % 3;
	UOSInt tmp2 = buffSize / 3;
	if (tmp1)
	{
		outSize = tmp2 * 4 + 4;
	}
	else
	{
		outSize = tmp2 * 4;
	}
	if (outSize == 0)
		return 0;
	sb->AllocLeng(outSize);
	while (tmp2-- > 0)
	{
		sptr[0] = encArr[dataBuff[0] >> 2];
		sptr[1] = encArr[((dataBuff[0] << 4) | (dataBuff[1] >> 4)) & 0x3f];
		sptr[2] = encArr[((dataBuff[1] << 2) | (dataBuff[2] >> 6)) & 0x3f];
		sptr[3] = encArr[dataBuff[2] & 0x3f];
		sb->AppendC(sptr, 4);
		dataBuff += 3;
	}
	if (tmp1 == 1)
	{
		sptr[0] = encArr[dataBuff[0] >> 2];
		sptr[1] = encArr[(dataBuff[0] << 4) & 0x3f];
		if (this->noPadding)
		{
			sb->AppendC(sptr, 2);
		}
		else
		{
			sptr[2] = '=';
			sptr[3] = '=';
			sb->AppendC(sptr, 4);
		}
	}
	else if (tmp1 == 2)
	{
		sptr[0] = encArr[dataBuff[0] >> 2];
		sptr[1] = encArr[((dataBuff[0] << 4) | (dataBuff[1] >> 4)) & 0x3f];
		sptr[2] = encArr[(dataBuff[1] << 2) & 0x3f];
		if (this->noPadding)
		{
			sb->AppendC(sptr, 3);
		}
		else
		{
			sptr[3] = '=';
			sb->AppendC(sptr, 4);
		}
	}
	return outSize;
}

UOSInt Text::TextBinEnc::Base64Enc::CalcBinSize(const UTF8Char *sbuff)
{
	UOSInt cnt = 0;
	UTF8Char c;
	while ((c = *sbuff++) != 0)
	{
		if (c < 0x80 && decArr[c] != 0xff)
		{
			cnt++;
		}
	}
	return cnt * 3 / 4;
}

UOSInt Text::TextBinEnc::Base64Enc::CalcBinSize(const WChar *sbuff)
{
	UOSInt cnt = 0;
	WChar c;
	while ((c = *sbuff++) != 0)
	{
		if (c < 0x80 && decArr[c] != 0xff)
		{
			cnt++;
		}
	}
	return cnt * 3 / 4;
}

UOSInt Text::TextBinEnc::Base64Enc::DecodeBin(const UTF8Char *b64Str, UInt8 *dataBuff)
{
	UOSInt decSize = 0;
	UInt8 b = 0;
	UInt8 b2 = 0;
	UInt8 code;
	UTF8Char c;
	while ((c = *b64Str++) != 0)
	{
		if (c < 0x80)
		{
			code = decArr[c];
			if (code != 0xff)
			{
				switch (b)
				{
				case 0:
					b2 = (UInt8)(code << 2);
					b = 1;
					break;
				case 1:
					*dataBuff = (UInt8)(b2 | (code >> 4));
					b2 = (UInt8)(code << 4);
					dataBuff++;
					decSize++;
					b = 2;
					break;
				case 2:
					*dataBuff = (UInt8)(b2 | (code >> 2));
					b2 = (UInt8)(code << 6);
					dataBuff++;
					decSize++;
					b = 3;
					break;
				case 3:
					*dataBuff = (UInt8)(b2 | code);
					dataBuff++;
					decSize++;
					b = 0;
					break;
				}
			}
		}
	}
	return decSize;
}

UOSInt Text::TextBinEnc::Base64Enc::DecodeBin(const UTF8Char *b64Str, UOSInt len, UInt8 *dataBuff)
{
	UOSInt decSize = 0;
	UInt8 b = 0;
	UInt8 b2 = 0;
	UInt8 code;
	UTF8Char c;
	while (len-- > 0)
	{
		c = *b64Str++;
		if (c < 0x80)
		{
			code = decArr[c];
			if (code != 0xff)
			{
				switch (b)
				{
				case 0:
					b2 = (UInt8)(code << 2);
					b = 1;
					break;
				case 1:
					*dataBuff = (UInt8)(b2 | (code >> 4));
					b2 = (UInt8)(code << 4);
					dataBuff++;
					decSize++;
					b = 2;
					break;
				case 2:
					*dataBuff = (UInt8)(b2 | (code >> 2));
					b2 = (UInt8)(code << 6);
					dataBuff++;
					decSize++;
					b = 3;
					break;
				case 3:
					*dataBuff = (UInt8)(b2 | code);
					dataBuff++;
					decSize++;
					b = 0;
					break;
				}
			}
		}
	}
	return decSize;
}

UOSInt Text::TextBinEnc::Base64Enc::DecodeBin(const WChar *b64Str, UInt8 *dataBuff)
{
	UOSInt decSize = 0;
	UInt8 b = 0;
	UInt8 b2 = 0;
	UInt8 code;
	WChar c;
	while ((c = *b64Str++) != 0)
	{
		if (c < 0x80)
		{
			code = decArr[c];
			if (code != 0xff)
			{
				switch (b)
				{
				case 0:
					b2 = (UInt8)(code << 2);
					b = 1;
					break;
				case 1:
					*dataBuff = (UInt8)(b2 | (code >> 4));
					b2 = (UInt8)(code << 4);
					dataBuff++;
					decSize++;
					b = 2;
					break;
				case 2:
					*dataBuff = (UInt8)(b2 | (code >> 2));
					b2 = (UInt8)(code << 6);
					dataBuff++;
					decSize++;
					b = 3;
					break;
				case 3:
					*dataBuff = (UInt8)(b2 | code);
					dataBuff++;
					decSize++;
					b = 0;
					break;
				}
			}
		}
	}
	return decSize;
}

const UTF8Char *Text::TextBinEnc::Base64Enc::GetName()
{
	return (const UTF8Char*)"Base64";
}
