#include "Stdafx.h"
#include "MyMemory.h"
#include "Math/TSPFile.h"
#include "Parser/FileParser/TSPParser.h"

Parser::FileParser::TSPParser::TSPParser()
{
}

Parser::FileParser::TSPParser::~TSPParser()
{
}

Int32 Parser::FileParser::TSPParser::GetName()
{
	return *(Int32*)"TSPP";
}

void Parser::FileParser::TSPParser::PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t)
{
	if (t == IO::ParsedObject::PT_UNKNOWN || t == IO::ParsedObject::PT_READINGDB_PARSER)
	{
		selector->AddFilter((const UTF8Char*)"*.tsp", (const UTF8Char*)"Total Station Point File");
	}
}

IO::ParsedObject::ParserType Parser::FileParser::TSPParser::GetParserType()
{
	return IO::ParsedObject::PT_READINGDB_PARSER;
}

IO::ParsedObject *Parser::FileParser::TSPParser::ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType)
{
	UInt8 hdr[8];
	fd->GetRealData(0, 8, hdr);

	if (*(Int64*)hdr != *(Int64*)"SmTS____" && *(Int64*)hdr != *(Int64*)"SmTS___A" && *(Int64*)hdr != *(Int64*)"SmTS___B")
	{
		return 0;
	}
	Math::TSPFile *pf;
	NEW_CLASS(pf, Math::TSPFile(fd));

	return pf;
}
