#ifndef _SM_MAP_REVGEODIR
#define _SM_MAP_REVGEODIR
#include "Data/ArrayList.h"
#include "IO/IWriter.h"
#include "Map/IReverseGeocoder.h"
#include "Map/RevGeoCfg.h"

namespace Map
{
	class RevGeoDir : public Map::IReverseGeocoder
	{
	private:
		typedef struct
		{
			Map::RevGeoCfg *cfg;
			Int32 lcid;
		} RevGeoFile;

		Data::ArrayList<RevGeoFile*> *files;
		Map::MapSearchManager *mapSrchMgr;
		Int32 defLCID;

	public:
		RevGeoDir(const UTF8Char *cfgDir, Int32 defLCID, IO::IWriter *errWriter);
		virtual ~RevGeoDir();

		virtual UTF8Char *SearchName(UTF8Char *buff, UOSInt buffSize, Double lat, Double lon, Int32 lcid);
		virtual UTF8Char *CacheName(UTF8Char *buff, UOSInt buffSize, Double lat, Double lon, Int32 lcid);
	};
}
#endif