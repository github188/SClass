#include "Stdafx.h"
#include "MyMemory.h"
#include "IO/PackageFile.h"
#include "Parser/FileParser/TARParser.h"
#include "Text/Encoding.h"
#include "Text/MyString.h"
#include "Text/StringBuilderUTF8.h"

Parser::FileParser::TARParser::TARParser()
{
	this->codePage = 65001;
}

Parser::FileParser::TARParser::~TARParser()
{
}

Int32 Parser::FileParser::TARParser::GetName()
{
	return *(Int32*)"TARP";
}

void Parser::FileParser::TARParser::SetCodePage(Int32 codePage)
{
	this->codePage = codePage;
}

void Parser::FileParser::TARParser::PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t)
{
	if (t == IO::ParsedObject::PT_UNKNOWN || t == IO::ParsedObject::PT_PACKAGE_PARSER)
	{
		selector->AddFilter((const UTF8Char*)"*.tar", (const UTF8Char*)"Tar File");
	}
}

IO::ParsedObject::ParserType Parser::FileParser::TARParser::GetParserType()
{
	return IO::ParsedObject::PT_PACKAGE_PARSER;
}

IO::ParsedObject *Parser::FileParser::TARParser::ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType)
{
	UTF8Char sbuff[256];
	UTF8Char *sptr;
	UInt8 buff[512];
	OSInt i;
	Int64 currOfst;
	Int64 fileSize;
	Int64 itemSize;
	Int64 t;
	Text::StrConcat(sbuff, fd->GetFullName());
	i = Text::StrLastIndexOf(sbuff, '.');
	if (Text::StrCompareICase(&sbuff[i], (const UTF8Char*)".TAR") != 0)
	{
		return 0;
	}
	fileSize = fd->GetDataSize();
	if (fileSize & 511)
	{
		return 0;
	}

	currOfst = 0;
	IO::PackageFile *pf;
	IO::PackageFile *pf2;
	IO::PackageFile *pf3;
	Text::StringBuilderUTF8 sb;
	Text::Encoding enc(this->codePage);
	NEW_CLASS(pf, IO::PackageFile(fd->GetFullName()));

	while (true)
	{
		if (currOfst >= fileSize)
		{
			DEL_CLASS(pf);
			return 0;
		}
		fd->GetRealData(currOfst, 512, buff);
		currOfst += 512;

		itemSize = Text::StrOct2Int64((Char*)&buff[124]);
		t = Text::StrOct2Int64((Char*)&buff[136]);
		enc.UTF8FromBytes(sbuff, buff, 100, 0);
		if (itemSize == 0)
		{
			if (buff[0] == 0)
				return pf;
			if (buff[156] == '5')
			{
				sptr = sbuff;
				pf2 = pf;
				sb.ClearStr();
				sb.Append(fd->GetFullName());
				while (true)
				{
					i = Text::StrIndexOf(sptr, '/');
					if (i >= 0)
					{
						sptr[i] = 0;
						sb.Append((const UTF8Char*)"\\");
						sb.Append(sptr);
						pf3 = pf2->GetPackFile(sptr);
						if (pf3 == 0)
						{
							NEW_CLASS(pf3, IO::PackageFile(sb.ToString()));
							pf2->AddPack(pf3, sptr, t * 1000LL);
						}
						pf2 = pf3;
						sptr = &sptr[i + 1];
					}
					else 
					{
						if (sptr[0] != 0)
						{
							sb.Append((const UTF8Char*)"\\");
							sb.Append(sptr);
							pf3 = pf2->GetPackFile(sptr);
							if (pf3 == 0)
							{
								NEW_CLASS(pf3, IO::PackageFile(sb.ToString()));
								pf2->AddPack(pf3, sptr, t * 1000LL);
							}
						}
						break;
					}
				}
				continue;
			}
			DEL_CLASS(pf);
			return 0;
		}
		pf2 = pf;
		sptr = sbuff;
		sb.ClearStr();
		sb.Append(fd->GetFullName());
		while (true)
		{
			i = Text::StrIndexOf(sptr, '/');
			if (i >= 0)
			{
				sptr[i] = 0;
				sb.Append((const UTF8Char*)"\\");
				sb.Append(sptr);
				pf3 = pf2->GetPackFile(sptr);
				if (pf3 == 0)
				{
					NEW_CLASS(pf3, IO::PackageFile(sb.ToString()));
					pf2->AddPack(pf3, sptr, t * 1000LL);
				}
				pf2 = pf3;
				sptr = &sptr[i + 1];
			}
			else
			{
				break;
			}
		}
		pf2->AddData(fd, currOfst, itemSize, sptr, t * 1000LL);
		if (itemSize & 511)
		{
			currOfst += itemSize + 512 - (itemSize & 511);
		}
		else
		{
			currOfst += itemSize;
		}
	}
	return 0;
}