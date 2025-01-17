#include "Stdafx.h"
#include "MyMemory.h"
#include "DB/ReadingDB.h"
#include "Map/GPSTrack.h"
#include "Map/MapLayerCollection.h"
#include "Parser/ObjParser/DBMapParser.h"

Parser::ObjParser::DBMapParser::DBMapParser()
{
}

Parser::ObjParser::DBMapParser::~DBMapParser()
{
}

Int32 Parser::ObjParser::DBMapParser::GetName()
{
	return *(Int32*)"DBMA";
}

void Parser::ObjParser::DBMapParser::PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t)
{
	if (t == IO::ParsedObject::PT_MAP_LAYER_PARSER)
	{
		selector->AddFilter((const UTF8Char*)"*.mdb", (const UTF8Char*)"MDB map layer");
	}
}

IO::ParsedObject::ParserType Parser::ObjParser::DBMapParser::GetParserType()
{
	return IO::ParsedObject::PT_MAP_LAYER_PARSER;
}

IO::ParsedObject *Parser::ObjParser::DBMapParser::ParseObject(IO::ParsedObject *pobj, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType)
{
//	DB::ReadingDB *db;
//	DB::DBReader *r;
//	WChar sbuff[512];
	Bool valid = false;
	if (pobj->GetParserType() != IO::ParsedObject::PT_READINGDB_PARSER)
		return 0;
	///////////////////////////
/*	Map::MapLayerCollection *layerColl;
	Data::ArrayList<const WChar *> tableNames;
	OSInt i;
	OSInt j;
	OSInt k;
	OSInt xCol;
	OSInt yCol;
	DB::SharedDBConn *sdb;

	//NEW_CLASS(sdb, DB::SharedDBConn(db
	
	NEW_CLASS(layerColl, Map::MapLayerCollection(pobj->GetSourceNameObj()));
	db = (DB::ReadingDB*)pobj;
	db->GetTableNames(&tableNames);
	i = 0;
	j = tableNames.GetCount();
	while (i < j)
	{
		r = db->GetTableData(tableNames.GetItem(i);
		if (r)
		{
			xCol = -1;
			yCol = -1;
			k = r->ColCount();
			while (k-- > 0)
			{
				if (r->GetName(k, sbuff))
				{
					if (Text::StrCompareICase(sbuff, L"EASTING") == 0)
					{
						xCol = k;
					}
					if (Text::StrCompareICase(sbuff, L"NORTHING") == 0)
					{
						yCol = k;
					}
				}
			}

			if (xCol != -1 && yCol != -1)
			{
				////////////////////////////////
			}
			db->CloseReader(r);
		}
	}

	if (layerColl->GetCount() > 0)
	{
		return layerColl;
	}
	DEL_CLASS(layerColl);*/
	return 0;
}
