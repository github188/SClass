#include "Stdafx.h"
#include "MyMemory.h"
#include "Math/Math.h"
#include "Media/CodeImageGen/EAN2CodeImageGen.h"
#include "Text/MyString.h"

Media::CodeImageGen::EAN2CodeImageGen::EAN2CodeImageGen()
{
}

Media::CodeImageGen::EAN2CodeImageGen::~EAN2CodeImageGen()
{
}

Media::CodeImageGen::CodeImageGen::CodeType Media::CodeImageGen::EAN2CodeImageGen::GetCodeType()
{
	return Media::CodeImageGen::CodeImageGen::CT_EAN2;
}

UOSInt Media::CodeImageGen::EAN2CodeImageGen::GetMinLength()
{
	return 2;
}

UOSInt Media::CodeImageGen::EAN2CodeImageGen::GetMaxLength()
{
	return 2;
}

Media::DrawImage *Media::CodeImageGen::EAN2CodeImageGen::GenCode(const UTF8Char *code, UOSInt codeWidth, Media::DrawEngine *eng)
{
	UTF8Char sbuff[2];
	if (code == 0)
		return 0;

	UOSInt i = 2;
	UOSInt j;
	UOSInt k;
	const UTF8Char *tmpStr = code;
	UTF8Char c;
	while (i-- > 0)
	{
		c = *tmpStr++;
		if (c < '0' || c > '9')
			return 0;
	}
	if (*tmpStr != 0)
		return 0;
	j = Text::StrToUInt32(code);

	UInt8 bitCode[21];
	bitCode[0] = 0;
	bitCode[1] = 1;
	bitCode[2] = 0;
	bitCode[3] = 1;
	bitCode[4] = 1;
	switch (j & 3)
	{
	case 0:
		tmpStr = (const UTF8Char*)"LL";
		break;
	case 1:
		tmpStr = (const UTF8Char*)"LG";
		break;
	case 2:
		tmpStr = (const UTF8Char*)"GL";
		break;
	case 3:
		tmpStr = (const UTF8Char*)"GG";
		break;
	}
	j = 5;
	i = 2;
	while (i-- > 0)
	{
		if (*tmpStr++ == 'L') //L-code
		{
			switch (*code++)
			{
			case '0':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 0;
				bitCode[j + 2] = 0;
				bitCode[j + 3] = 1;
				bitCode[j + 4] = 1;
				bitCode[j + 5] = 0;
				bitCode[j + 6] = 1;
				break;
			case '1':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 0;
				bitCode[j + 2] = 1;
				bitCode[j + 3] = 1;
				bitCode[j + 4] = 0;
				bitCode[j + 5] = 0;
				bitCode[j + 6] = 1;
				break;
			case '2':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 0;
				bitCode[j + 2] = 1;
				bitCode[j + 3] = 0;
				bitCode[j + 4] = 0;
				bitCode[j + 5] = 1;
				bitCode[j + 6] = 1;
				break;
			case '3':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 1;
				bitCode[j + 2] = 1;
				bitCode[j + 3] = 1;
				bitCode[j + 4] = 1;
				bitCode[j + 5] = 0;
				bitCode[j + 6] = 1;
				break;
			case '4':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 1;
				bitCode[j + 2] = 0;
				bitCode[j + 3] = 0;
				bitCode[j + 4] = 0;
				bitCode[j + 5] = 1;
				bitCode[j + 6] = 1;
				break;
			case '5':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 1;
				bitCode[j + 2] = 1;
				bitCode[j + 3] = 0;
				bitCode[j + 4] = 0;
				bitCode[j + 5] = 0;
				bitCode[j + 6] = 1;
				break;
			case '6':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 1;
				bitCode[j + 2] = 0;
				bitCode[j + 3] = 1;
				bitCode[j + 4] = 1;
				bitCode[j + 5] = 1;
				bitCode[j + 6] = 1;
				break;
			case '7':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 1;
				bitCode[j + 2] = 1;
				bitCode[j + 3] = 1;
				bitCode[j + 4] = 0;
				bitCode[j + 5] = 1;
				bitCode[j + 6] = 1;
				break;
			case '8':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 1;
				bitCode[j + 2] = 1;
				bitCode[j + 3] = 0;
				bitCode[j + 4] = 1;
				bitCode[j + 5] = 1;
				bitCode[j + 6] = 1;
				break;
			case '9':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 0;
				bitCode[j + 2] = 0;
				bitCode[j + 3] = 1;
				bitCode[j + 4] = 0;
				bitCode[j + 5] = 1;
				bitCode[j + 6] = 1;
				break;
			}
			j += 7;
		}
		else //G-code
		{
			switch (*code++)
			{
			case '0':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 1;
				bitCode[j + 2] = 0;
				bitCode[j + 3] = 0;
				bitCode[j + 4] = 1;
				bitCode[j + 5] = 1;
				bitCode[j + 6] = 1;
				break;
			case '1':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 1;
				bitCode[j + 2] = 1;
				bitCode[j + 3] = 0;
				bitCode[j + 4] = 0;
				bitCode[j + 5] = 1;
				bitCode[j + 6] = 1;
				break;
			case '2':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 0;
				bitCode[j + 2] = 1;
				bitCode[j + 3] = 1;
				bitCode[j + 4] = 0;
				bitCode[j + 5] = 1;
				bitCode[j + 6] = 1;
				break;
			case '3':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 1;
				bitCode[j + 2] = 0;
				bitCode[j + 3] = 0;
				bitCode[j + 4] = 0;
				bitCode[j + 5] = 0;
				bitCode[j + 6] = 1;
				break;
			case '4':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 0;
				bitCode[j + 2] = 1;
				bitCode[j + 3] = 1;
				bitCode[j + 4] = 1;
				bitCode[j + 5] = 0;
				bitCode[j + 6] = 1;
				break;
			case '5':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 1;
				bitCode[j + 2] = 1;
				bitCode[j + 3] = 1;
				bitCode[j + 4] = 0;
				bitCode[j + 5] = 0;
				bitCode[j + 6] = 1;
				break;
			case '6':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 0;
				bitCode[j + 2] = 0;
				bitCode[j + 3] = 0;
				bitCode[j + 4] = 1;
				bitCode[j + 5] = 0;
				bitCode[j + 6] = 1;
				break;
			case '7':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 0;
				bitCode[j + 2] = 1;
				bitCode[j + 3] = 0;
				bitCode[j + 4] = 0;
				bitCode[j + 5] = 0;
				bitCode[j + 6] = 1;
				break;
			case '8':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 0;
				bitCode[j + 2] = 0;
				bitCode[j + 3] = 1;
				bitCode[j + 4] = 0;
				bitCode[j + 5] = 0;
				bitCode[j + 6] = 1;
				break;
			case '9':
				bitCode[j + 0] = 0;
				bitCode[j + 1] = 0;
				bitCode[j + 2] = 1;
				bitCode[j + 3] = 0;
				bitCode[j + 4] = 1;
				bitCode[j + 5] = 1;
				bitCode[j + 6] = 1;
				break;
			}
			j += 7;
		}
		if (i > 0)
		{
			bitCode[j + 0] = 0;
			bitCode[j + 1] = 1;
			j += 2;
		}
		else
		{
			break;
		}
	}
	code = code - 2;

	UOSInt h = codeWidth * 70;
	UOSInt y = h - codeWidth;
	Double fh = 12.0 * Math::UOSInt2Double(codeWidth);

	Media::DrawImage *dimg = eng->CreateImage32((4 + 21) * codeWidth, h, Media::AT_NO_ALPHA);
	Media::DrawBrush *b;
	Media::DrawPen *p;
	Media::DrawFont *f;
	b = dimg->NewBrushARGB(0xffffffff);
	dimg->DrawRect(0, 0, (Double)dimg->GetWidth(), (Double)dimg->GetHeight(), 0, b);
	dimg->DelBrush(b);

	p = dimg->NewPenARGB(0xff000000, 1, 0, 0);
	i = 0;
	j = 2 * codeWidth;
	while (i < 21)
	{
		if (bitCode[i])
		{
			k = codeWidth;
			while (k-- > 0)
			{
				dimg->DrawLine((Double)j, (Double)codeWidth + fh, (Double)j, (Double)y, p);
				j++;
			}
		}
		else
		{
			j += codeWidth;
		}
		i++;
	}
	dimg->DelPen(p);

	f = dimg->NewFontPx((const UTF8Char*)"Arial", fh, Media::DrawEngine::DFS_NORMAL, 0);
	b = dimg->NewBrushARGB(0xff000000);
	sbuff[0] = *code++;
	sbuff[1] = 0;
	dimg->DrawString((Double)(2 + 5) * Math::UOSInt2Double(codeWidth), (Double)codeWidth, sbuff, f, b);
	sbuff[0] = *code++;
	dimg->DrawString((Double)(2 + 5 + 7 + 2) * Math::UOSInt2Double(codeWidth), (Double)codeWidth, sbuff, f, b);

	dimg->DelBrush(b);
	dimg->DelFont(f);

	return dimg;
}
