#ifndef _SM_MEDIA_CS_CSRGB16_LRGB
#define _SM_MEDIA_CS_CSRGB16_LRGB
#include "Media/CS/CSConverter.h"
#include "Sync/Event.h"

namespace Media
{
	namespace CS
	{
		class CSRGB16_LRGB : public Media::CS::CSConverter
		{
		private:
			UOSInt srcNBits;
			Media::ColorProfile *srcProfile;
			Media::CS::TransferType currRGBType;
			Bool invert;

			Media::IColorHandler::RGBPARAM2 *rgbParam;
			UInt8 *rgbTable;
			Bool rgbUpdated;
		private:
			void UpdateRGBTable();
		public:
			CSRGB16_LRGB(UOSInt srcNBits, Bool invert, Media::ColorProfile *srcProfile, Media::ColorManagerSess *colorSess);
			virtual ~CSRGB16_LRGB();
			virtual void ConvertV2(UInt8 **srcPtr, UInt8 *destPtr, UOSInt dispWidth, UOSInt dispHeight, UOSInt srcStoreWidth, UOSInt srcStoreHeight, OSInt destRGBBpl, Media::FrameType ftype, Media::YCOffset ycOfst);
			virtual UOSInt GetSrcFrameSize(UOSInt width, UOSInt height);
			virtual UOSInt GetDestFrameSize(UOSInt width, UOSInt height);
			virtual void SetPalette(UInt8 *pal);

			virtual void YUVParamChanged(const YUVPARAM *yuv);
			virtual void RGBParamChanged(const RGBPARAM2 *rgb);
		};
	}
}
#endif