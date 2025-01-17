#ifndef _SM_MEDIA_CS_CSYUV_LRGB
#define _SM_MEDIA_CS_CSYUV_LRGB
#include "Media/CS/CSConverter.h"
#include "Media/CS/TransferFunc.h"

namespace Media
{
	namespace CS
	{
		class CSYUV_LRGB : public Media::CS::CSConverter
		{
		protected:
			YUVPARAM yuvParam;
			RGBPARAM2 *rgbParam;
			TransferFunc *irFunc;
			TransferFunc *igFunc;
			TransferFunc *ibFunc;
			Media::ColorProfile *srcColor;
			Media::ColorProfile::YUVType yuvType;

			Bool yuvUpdated;
			Bool rgbUpdated;
			UInt8 *rgbGammaCorr;
			Int64 *yuv2rgb;

			void SetupRGB13_LR();
			void SetupYUV_RGB13();

		protected:
			CSYUV_LRGB(const Media::ColorProfile *srcColor, Media::ColorProfile::YUVType yuvType, Media::ColorManagerSess *colorSess);
			virtual ~CSYUV_LRGB();
			virtual void UpdateTable();

		public:
			virtual void YUVParamChanged(const Media::IColorHandler::YUVPARAM *yuv);
			virtual void RGBParamChanged(const Media::IColorHandler::RGBPARAM2 *rgb);
			virtual UOSInt GetDestFrameSize(UOSInt width, UOSInt height);
		};
	}
}
#endif
