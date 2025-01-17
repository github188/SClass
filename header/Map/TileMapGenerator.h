#ifndef _SM_MAP_TILEMAPGENERATOR
#define _SM_MAP_TILEMAPGENERATOR
#include "Map/MapConfig2TGen.h"


namespace Map
{
	class TileMapGenerator
	{
	private:
		UInt32 imgSize;
		const UTF8Char *tileDir;
		Map::MapConfig2TGen *mcfg;
		Media::DrawEngine *geng;
		UOSInt osSize;

		Data::ArrayListInt64 *dbGenList;
		Sync::Mutex *dbMut;
		Sync::Event *dbEvt;
		Media::IImgResizer *resizer;

	private:
		void InitMapView(Map::MapView *view, Int32 x, Int32 y, UInt32 scale);
		UTF8Char *GenFileName(UTF8Char *sbuff, Int32 x, Int32 y, UInt32 scale, const UTF8Char *ext);
		void AppendDBFile(IO::Writer *writer, Int32 x, Int32 y, UInt32 scale, Int32 xOfst, Int32 yOfst);
		Bool GenerateDBFile(Int32 x, Int32 y, UInt32 scale, Map::MapScheduler *mapSch);
	public:
		TileMapGenerator(Map::MapConfig2TGen *mcfg, Media::DrawEngine *geng, const UTF8Char *tileDir, UOSInt osSize);
		~TileMapGenerator();

		static Int64 GetTileID(Double lat, Double lon, UInt32 scale, UInt32 imgSize);

		Bool GenerateTile(Int64 tileId, UInt32 scale, Map::MapScheduler *mapSch);
		Bool GenerateTileArea(Double lat1, Double lon1, Double lat2, Double lon2, UInt32 scale, Map::MapScheduler *mapSch);
	};
}
#endif
