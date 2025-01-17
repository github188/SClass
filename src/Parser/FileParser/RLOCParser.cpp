#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ByteTool.h"
#include "IO/Path.h"
#include "Map/GPSTrack.h"
#include "Net/SocketUtil.h"
#include "Parser/FileParser/RLOCParser.h"
#include "Text/MyString.h"
#include "Text/MyStringFloat.h"

typedef struct
{
	Int32 devId;
	Int32 devType;
	Int32 funcs;
	UInt32 status;
	UInt32 status2;
	Double other0;
	Double other1;
	Double other2;
	Double other3;
	UInt32 locTimeTS;
	UInt32 recvTimeTS;
	Int64 connId;
	Int32 gtime;
	Double gpsLat;
	Double gpsLon;
} ExtraInfo;

class RLOCExtraParser : public Map::GPSTrack::GPSExtraParser
{
public:
	RLOCExtraParser()
	{
	}

	virtual ~RLOCExtraParser()
	{
	}

	virtual UOSInt GetExtraCount(const UInt8 *buff, UOSInt buffSize)
	{
		return 15;
	}

	virtual Bool GetExtraName(const UInt8 *buff, UOSInt buffSize, UOSInt extIndex, Text::StringBuilderUTF *sb)
	{
		switch (extIndex)
		{
		case 0:
			sb->Append((const UTF8Char*)"DevID");
			return true;
		case 1:
			sb->Append((const UTF8Char*)"DevType");
			return true;
		case 2:
			sb->Append((const UTF8Char*)"Funcs");
			return true;
		case 3:
			sb->Append((const UTF8Char*)"Status");
			return true;
		case 4:
			sb->Append((const UTF8Char*)"Status2");
			return true;
		case 5:
			sb->Append((const UTF8Char*)"Other0");
			return true;
		case 6:
			sb->Append((const UTF8Char*)"Other1");
			return true;
		case 7:
			sb->Append((const UTF8Char*)"Other2");
			return true;
		case 8:
			sb->Append((const UTF8Char*)"Other3");
			return true;
		case 9:
			sb->Append((const UTF8Char*)"LocTime");
			return true;
		case 10:
			sb->Append((const UTF8Char*)"RecvTime");
			return true;
		case 11:
			sb->Append((const UTF8Char*)"ConnId");
			return true;
		case 12:
			sb->Append((const UTF8Char*)"GTime");
			return true;
		case 13:
			sb->Append((const UTF8Char*)"GPS Lat");
			return true;
		case 14:
			sb->Append((const UTF8Char*)"GPS Lon");
			return true;
		}
		return false;
	}

	virtual Bool GetExtraValueStr(const UInt8 *buff, UOSInt buffSize, UOSInt extIndex, Text::StringBuilderUTF *sb)
	{
		if (buffSize != sizeof(ExtraInfo))
			return false;
		const ExtraInfo *extInfo = (const ExtraInfo*)buff;

		switch (extIndex)
		{
		case 0:
			sb->AppendI32(extInfo->devId);
			return true;
		case 1:
			sb->AppendI32(extInfo->devType);
			return true;
		case 2:
			sb->AppendI32(extInfo->funcs);
			return true;
		case 3:
			sb->Append((const UTF8Char*)"0x");
			sb->AppendHex32(extInfo->status);
			return true;
		case 4:
			sb->Append((const UTF8Char*)"0x");
			sb->AppendHex32(extInfo->status2);
			return true;
		case 5:
			Text::SBAppendF64(sb, extInfo->other0);
			return true;
		case 6:
			Text::SBAppendF64(sb, extInfo->other1);
			return true;
		case 7:
			Text::SBAppendF64(sb, extInfo->other2);
			return true;
		case 8:
			Text::SBAppendF64(sb, extInfo->other3);
			return true;
		case 9:
			{
				Data::DateTime dt;
				dt.SetUnixTimestamp(extInfo->locTimeTS);
				dt.ToLocalTime();
				sb->AppendDate(&dt);
			}
			return true;
		case 10:
			{
				Data::DateTime dt;
				dt.SetUnixTimestamp(extInfo->recvTimeTS);
				dt.ToLocalTime();
				sb->AppendDate(&dt);
			}
			return true;
		case 11:
			{
				UTF8Char sbuff[32];
				Net::SocketUtil::GetIPv4Name(sbuff, (UInt32)(extInfo->connId >> 16), (UInt16)(extInfo->connId & 0xffff));
				sb->Append(sbuff);
			}
			return true;
		case 12:
			sb->AppendI32(extInfo->gtime);
			return true;
		case 13:
			Text::SBAppendF64(sb, extInfo->gpsLat);
			return true;
		case 14:
			Text::SBAppendF64(sb, extInfo->gpsLon);
			return true;
		}
		return false;
	}
};


Parser::FileParser::RLOCParser::RLOCParser()
{
}

Parser::FileParser::RLOCParser::~RLOCParser()
{
}

Int32 Parser::FileParser::RLOCParser::GetName()
{
	return *(Int32*)"RLOC";
}

void Parser::FileParser::RLOCParser::PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t)
{
	if (t == IO::ParsedObject::PT_UNKNOWN || t == IO::ParsedObject::PT_MAP_LAYER_PARSER)
	{
		selector->AddFilter((const UTF8Char*)"*.dat", (const UTF8Char*)"Rodsum Location File");
	}
}

IO::ParsedObject::ParserType Parser::FileParser::RLOCParser::GetParserType()
{
	return IO::ParsedObject::PT_MAP_LAYER_PARSER;
}

IO::ParsedObject *Parser::FileParser::RLOCParser::ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType)
{
	Map::GPSTrack::GPSRecord rec;
	UInt8 buff[384];
	UTF8Char u8buff[256];
	const UTF8Char *u8ptr;
	UOSInt i;
	UOSInt currPos;
	UInt64 fileSize;
	Int32 devId;
	u8ptr = fd->GetFullName();
	i = Text::StrLastIndexOf(u8ptr, IO::Path::PATH_SEPERATOR);
	Text::StrConcat(u8buff, &u8ptr[i + 1]);
	if (!Text::StrStartsWithICase(u8buff, (const UTF8Char*)"LOC"))
	{
		return 0;
	}
	i = Text::StrIndexOf(u8buff, (const UTF8Char*)"_");
	if (i == INVALID_INDEX)
	{
		i = Text::StrIndexOf(u8buff, (const UTF8Char*)".");
		if (i == INVALID_INDEX)
			return 0;
		u8buff[i] = 0;
		devId = Text::StrToInt32(&u8buff[3]);
	}
	else
	{
		u8buff[i] = 0;
		devId = Text::StrToInt32(&u8buff[3]);
	}
	if (devId == 0)
		return 0;
	fileSize = fd->GetDataSize();
	if (fileSize & 127)
		return 0;

	fd->GetRealData(0, 384, buff);
	if (*(Int32*)&buff[0] != devId || *(Int32*)&buff[128] != devId || *(Int32*)&buff[256] != devId)
		return 0;

	Map::GPSTrack *track;
	Text::StrInt32(u8buff, devId);
	NEW_CLASS(track, Map::GPSTrack(fd->GetFullName(), true, 0, u8buff));
	track->SetTrackName(u8buff);
	RLOCExtraParser *parser;
	NEW_CLASS(parser, RLOCExtraParser());
	track->SetExtraParser(parser);
	currPos = 0;
	while (currPos < fileSize)
	{
		ExtraInfo extInfo;

		fd->GetRealData(currPos, 128, buff);
		extInfo.devId = ReadInt32(&buff[0]);
		extInfo.devType = ReadInt32(&buff[4]);
		rec.speed = ReadUInt16(&buff[8]) * 0.1;
		rec.heading = ReadUInt16(&buff[10]) * 0.01;
		rec.utcTimeTicks = ReadInt32(&buff[12]) * 1000LL;
		extInfo.funcs = ReadInt24(&buff[16]);
		rec.nSateUsed = buff[19];
		rec.lat = ReadInt32(&buff[20]) / 200000.0;
		rec.lon = ReadInt32(&buff[24]) / 200000.0;
		extInfo.status = ReadUInt32(&buff[28]);
		extInfo.status2 = ReadUInt32(&buff[32]);
		rec.nSateView = buff[36];
		rec.valid = buff[38] & 1;
		extInfo.other0 = ReadUInt32(&buff[40]) * 0.001;
		extInfo.other1 = ReadUInt32(&buff[44]) * 0.001;
		extInfo.other2 = ReadInt16(&buff[48]) * 0.01;
		extInfo.other3 = ReadInt16(&buff[50]) * 0.01;
		extInfo.locTimeTS = ReadUInt32(&buff[52]);
		extInfo.recvTimeTS = ReadUInt32(&buff[56]);
		extInfo.connId = ReadInt64(&buff[64]);
		extInfo.gtime = ReadInt32(&buff[72]);
		extInfo.gpsLat = ReadInt32(&buff[76]) / 200000.0;
		extInfo.gpsLon = ReadInt32(&buff[80]) / 200000.0;
		if ((*(Int16*)&buff[16] & 0x22) == 0x22)
		{
			rec.altitude = (*(Int32*)&buff[44]) * 0.001;
		}
		else
		{
			rec.altitude = 0;
		}
		i = track->AddRecord(&rec);
		track->SetExtraDataIndex(i, (const UInt8*)&extInfo, sizeof(extInfo));

		currPos += 128;
	}
	return track;
}
