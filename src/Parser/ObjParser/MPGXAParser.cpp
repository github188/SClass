#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ByteTool.h"
#include "IO/ISectorData.h"
#include "Media/VCDMPGFile.h"
#include "Parser/ObjParser/MPGXAParser.h"

Parser::ObjParser::MPGXAParser::MPGXAParser()
{
}

Parser::ObjParser::MPGXAParser::~MPGXAParser()
{
}

Int32 Parser::ObjParser::MPGXAParser::GetName()
{
	return *(Int32*)"MPGX";
}

void Parser::ObjParser::MPGXAParser::PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t)
{
	if (t == IO::ParsedObject::PT_VIDEO_PARSER)
	{
		selector->AddFilter((const UTF8Char*)"*.dat", (const UTF8Char*)"VCD File");
	}
}

IO::ParsedObject::ParserType Parser::ObjParser::MPGXAParser::GetParserType()
{
	return IO::ParsedObject::PT_VIDEO_PARSER;
}

IO::ParsedObject *Parser::ObjParser::MPGXAParser::ParseObject(IO::ParsedObject *pobj, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType)
{
	IO::ISectorData *data;
	if (pobj->GetParserType() != IO::ParsedObject::PT_SECTOR_DATA)
		return 0;
	data = (IO::ISectorData*)pobj;
	if (data->GetBytesPerSector() != 2352)
		return 0;

	UInt8 buff[2352];
	Bool found = false;
	OSInt i = 0;
	Int32 v;
	while (i < 75)
	{
		if (!data->ReadSector(i, buff))
			return 0;
		v = ReadMInt32(&buff[24]);
		if (v == 0x1ba)
		{
			if ((buff[28] & 0xf0) != 0x20)
				return 0;
			if (ReadMInt32(&buff[36]) != 0x000001bb)
				return 0;

			found = true;
			break;
		}
		else if (v == 0x01434430 && ReadMInt16(&buff[28]) == 0x3031)
		{
		}
		else if (v != 0)
		{
			return 0;
		}
		i++;
	}
	if (!found)
		return 0;

	Media::VCDMPGFile *file;
	NEW_CLASS(file, Media::VCDMPGFile(data, i, data->GetSectorCount()));
	return file;
}