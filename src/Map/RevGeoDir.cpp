#include "Stdafx.h"
#include "MyMemory.h"
#include "Text/MyString.h"
#include "IO/Path.h"
#include "Map/RevGeoDir.h"

Map::RevGeoDir::RevGeoDir(const UTF8Char *cfgDir, Int32 defLCID, IO::IWriter *errWriter)
{
	UTF8Char sbuff[256];
	UTF8Char *sptr;
	Data::DateTime modTime;
	void *sess;
	IO::Path::PathType pt;


	NEW_CLASS(files, Data::ArrayList<RevGeoFile*>());
	NEW_CLASS(mapSrchMgr, Map::MapSearchManager());
	this->defLCID = defLCID;

	sptr = Text::StrConcat(sbuff, cfgDir);
	if (sptr[-1] != IO::Path::PATH_SEPERATOR)
	{
		*sptr++ = IO::Path::PATH_SEPERATOR;
	}
	Text::StrConcat(sptr, (const UTF8Char*)"RevGeo_*.*");
	sess = IO::Path::FindFile(sbuff);
	if (sess == 0)
	{
		return;
	}
	while (IO::Path::FindNextFile(sptr, sess, &modTime, &pt, 0))
	{
		if (Text::StrStartsWithICase(sptr, (const UTF8Char*)"REVGEO_"))
		{
			errWriter->Write((const UTF8Char*)"Loading search file ");
			errWriter->Write(sptr);
			errWriter->Write((const UTF8Char*)"...");
			
			Map::RevGeoCfg *revGeo;
			RevGeoFile *file;
			OSInt i;
			NEW_CLASS(revGeo, Map::RevGeoCfg(sbuff, this->mapSrchMgr));
			file = MemAlloc(RevGeoFile, 1);
			file->cfg = revGeo;
			i = Text::StrIndexOf(sptr, (const UTF8Char*)".");
			if (i >= 7)
			{
				sptr[i] = 0;
				file->lcid = Text::StrToInt32(&sptr[7]);
			}
			else
			{
				file->lcid = 0;
			}
			this->files->Add(file);
			errWriter->WriteLine((const UTF8Char*)"Success");

		}
	}
	IO::Path::FindFileClose(sess);

}

Map::RevGeoDir::~RevGeoDir()
{
	RevGeoFile *file;
	OSInt i = this->files->GetCount();
	while (i-- > 0)
	{
		file = this->files->GetItem(i);
		DEL_CLASS(file->cfg);
		MemFree(file);
	}
	DEL_CLASS(files);
	DEL_CLASS(mapSrchMgr);
}

UTF8Char *Map::RevGeoDir::SearchName(UTF8Char *buff, UOSInt buffSize, Double lat, Double lon, Int32 lcid)
{
	OSInt i;
	RevGeoFile *file;
	RevGeoFile *tmpFile;
	file = this->files->GetItem(0);
	i = this->files->GetCount();
	while (i-- > 0)
	{
		tmpFile = this->files->GetItem(i);
		if (tmpFile->lcid == lcid)
		{
			file = tmpFile;
			break;
		}
		else if (tmpFile->lcid == this->defLCID)
		{
			file = tmpFile;
		}
	}
	if (file == 0)
		return 0;
	return file->cfg->GetStreetName(buff, buffSize, lat, lon);
}

UTF8Char *Map::RevGeoDir::CacheName(UTF8Char *buff, UOSInt buffSize, Double lat, Double lon, Int32 lcid)
{
	return SearchName(buff, buffSize, lat, lon, lcid);
}