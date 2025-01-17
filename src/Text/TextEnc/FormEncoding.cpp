#include "Stdafx.h"
#include "MyMemory.h"
#include "Text/Encoding.h"
#include "Text/MyString.h"
#include "Text/TextEnc/FormEncoding.h"

static UInt8 URIAllow[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void Text::TextEnc::FormEncoding::FormEncode(Text::StringBuilderUTF *sb, const UTF8Char *uri)
{
	UInt8 b;
	while ((b = *uri++) != 0)
	{
		if (URIAllow[b])
		{
			sb->AppendChar(b, 1);
		}
		else
		{
			if (b == ' ')
			{
				sb->AppendChar('+', 1);
			}
			else
			{
				sb->AppendChar('%', 1);
				sb->AppendHex8(b);
			}
		}
	}
}

UTF8Char *Text::TextEnc::FormEncoding::FormEncode(UTF8Char *buff, const UTF8Char *uri)
{
	const UTF8Char *src;
	UInt8 b;
	UTF8Char *dest;

	src = uri;
	dest = buff;
	while (*src)
	{
		if (URIAllow[*src])
		{
			*dest++ = *src++;
		}
		else
		{
			b = *src++;
			if (b == ' ')
			{
				*dest++ = '+';
			}
			else
			{
				*dest++ = '%';
				*dest++ = (UTF8Char)MyString_STRHEXARR[b >> 4];
				*dest++ = (UTF8Char)MyString_STRHEXARR[b & 15];
			}
		}
	}
	*dest = 0;
	return dest;
}

UTF8Char *Text::TextEnc::FormEncoding::FormDecode(UTF8Char *buff, const UTF8Char *uri)
{
	UInt8 *dest;
	UInt8 v;
	UTF8Char c;
	dest = buff;

	while ((c = *uri++) != 0)
	{
		if (c == '+')
		{
			*dest++ = ' ';
		}
		else if (c == '%')
		{
			if (uri[0] == 0 || uri[1] == 0)
			{
				*dest++ = (UInt8)c;
			}
			else
			{
				c = *uri++;
				if (c >= 0x30 && c <= 0x39)
				{
					v = (UInt8)(c - 0x30);
				}
				else if (c >= 0x41 && c <= 0x46)
				{
					v = (UInt8)(c - 0x37);
				}
				else if (c >= 0x61 && c <= 0x66)
				{
					v = (UInt8)(c - 0x57);
				}
				else
				{
					*dest++ = '%';
					*dest++ = (UInt8)c;
					c = 0;
				}
				if (c)
				{
					c = *uri++;
					if (c >= 0x30 && c <= 0x39)
					{
						*dest++ = (UInt8)((v << 4) + (c - 0x30));
					}
					else if (c >= 0x41 && c <= 0x46)
					{
						*dest++ = (UInt8)((v << 4) + (c - 0x37));
					}
					else if (c >= 0x61 && c <= 0x66)
					{
						*dest++ = (UInt8)((v << 4) + (c - 0x57));
					}
					else
					{
						*dest++ = '%';
						*dest++ = (UInt8)uri[-2];
						*dest++ = (UInt8)c;
					}
				}
			}
		}
		else
		{
			*dest++ = (UInt8)c;
		}
	}
	*dest = 0;
	return dest;
}

Text::TextEnc::FormEncoding::FormEncoding()
{
}

Text::TextEnc::FormEncoding::~FormEncoding()
{
}

UTF8Char *Text::TextEnc::FormEncoding::EncodeString(UTF8Char *buff, const UTF8Char *strToEnc)
{
	return FormEncode(buff, strToEnc);
}

UTF8Char *Text::TextEnc::FormEncoding::DecodeString(UTF8Char *buff, const UTF8Char *strToDec)
{
	return FormDecode(buff, strToDec);
}
