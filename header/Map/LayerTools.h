#ifndef _SM_MAP_LAYERTOOLS
#define _SM_MAP_LAYERTOOLS
#include "Map/VectorLayer.h"
#include "Data/ArrayList.h"

namespace Map
{
	class LayerTools
	{
	public:
		static Map::VectorLayer *CombineLayers(Data::ArrayList<Map::IMapDrawLayer*> *layers, const UTF8Char *name);
	};
};
#endif
