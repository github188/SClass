#ifndef _SM_MEDIA_GRFILTER
#define _SM_MEDIA_GRFILTER
#include "Data/ArrayList.h"

namespace Media
{
	class GRFilter
	{
	private:
		typedef struct
		{
			OSInt hOfst;
			OSInt vOfst;
			OSInt level;
			Int32 status;
		} LayerSetting;

	private:
		Data::ArrayList<LayerSetting*> *layers;

	public:
		GRFilter();
		~GRFilter();

		UOSInt GetLayerCount();
		UOSInt AddLayer();
		Bool RemoveLayer(UOSInt layer);
		void SetParameter(UOSInt layer, OSInt hOfst, OSInt vOfst, OSInt level, Int32 status);
		Bool GetParameter(UOSInt layer, OSInt *hOfst, OSInt *vOfst, OSInt *level, Int32 *status);

		void ProcessImage32(UInt8 *srcPtr, UInt8 *destPtr, UOSInt width, UOSInt height, OSInt sbpl, OSInt dbpl);
		void ProcessImage64(UInt8 *srcPtr, UInt8 *destPtr, UOSInt width, UOSInt height, OSInt sbpl, OSInt dbpl);
	};
}
#endif
