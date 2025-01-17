#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ByteTool.h"
#include "Media/FileVideoSource.h"
#include "Media/MediaFile.h"
#include "Media/MPEGVideoParser.h"
#include "Media/Decoder/MP2GDecoder.h"
#include "Parser/FileParser/M2VParser.h"
#include <windows.h>

Parser::FileParser::M2VParser::M2VParser()
{
}

Parser::FileParser::M2VParser::~M2VParser()
{
}

Int32 Parser::FileParser::M2VParser::GetName()
{
	return *(Int32*)"M2VP";
}

void Parser::FileParser::M2VParser::PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t)
{
	if (t == IO::ParsedObject::PT_UNKNOWN || t == IO::ParsedObject::PT_VIDEO_PARSER)
	{
		selector->AddFilter((const UTF8Char*)"*.m2v", (const UTF8Char*)"MPEG-2 Video Elemental File");
	}
}

IO::ParsedObject::ParserType Parser::FileParser::M2VParser::GetParserType()
{
	return IO::ParsedObject::PT_VIDEO_PARSER;
}

IO::ParsedObject *Parser::FileParser::M2VParser::ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType)
{
	UInt8 tmpBuff[1024];
	OSInt readSize = fd->GetRealData(0, 1024, tmpBuff);
	Media::FrameInfo info;
	UInt32 frameRateNorm;
	UInt32 frameRateDenorm;
	if (!Media::MPEGVideoParser::GetFrameInfo(tmpBuff, readSize, &info, &frameRateNorm, &frameRateDenorm, 0, false))
		return 0;

	OSInt i;
	OSInt j;
	Int64 fleng = fd->GetDataSize();
	Int64 gopStart;
	OSInt gopFrameCnt = 0;
	Int64 thisGop;
	OSInt totalFrameCnt = 0;
	Int64 currOfst;
	Media::FileVideoSource *vSource;
	Int32 hdr;
	Int32 pictureHdr;
	WriteMInt32((UInt8*)&hdr, 0x000001b3);
	WriteMInt32((UInt8*)&pictureHdr, 0x00000100);
	info.fourcc = *(Int32*)"MP2G";
	NEW_CLASS(vSource, Media::FileVideoSource(fd, &info, frameRateNorm, frameRateDenorm, true));

	gopStart = 0;
	currOfst = 3;
	while (currOfst < fleng)
	{
		readSize = fd->GetRealData(currOfst, 1021, &tmpBuff[3]);
		if (readSize == 0)
		{
			DEL_CLASS(vSource);
			return 0;
		}

		i = 0;
		j = readSize - 3;
		while (i < j)
		{
			if (*(Int32*)&tmpBuff[i] == hdr)
			{
				thisGop = currOfst + i - 3;
				if (thisGop != 0)
				{
					vSource->AddNewFrame(gopStart, (Int32)(thisGop - gopStart), true, MulDiv((Int32)gopFrameCnt, frameRateDenorm * 1000, frameRateNorm));
				}
				gopStart = thisGop;
				gopFrameCnt = totalFrameCnt;
			}
			else if (*(Int32*)&tmpBuff[i] == pictureHdr)
			{
				totalFrameCnt++;
			}
			i++;
		}
		currOfst += readSize;
		tmpBuff[0] = tmpBuff[readSize - 3];
		tmpBuff[1] = tmpBuff[readSize - 2];
		tmpBuff[2] = tmpBuff[readSize - 1];
	}
	vSource->AddNewFrame(gopStart, (Int32)(fleng - gopStart), true, MulDiv((Int32)gopFrameCnt, frameRateDenorm * 1000, frameRateNorm));
	Media::Decoder::MP2GDecoder *mp2g;
	NEW_CLASS(mp2g, Media::Decoder::MP2GDecoder(vSource, true));
	Media::MediaFile *file;
	NEW_CLASS(file, Media::MediaFile(fd->GetFullName()));
	file->AddSource(mp2g, 0);
	return file;
}
