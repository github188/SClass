#ifndef _SM_MEDIA_DECODER_VDECODERCHAIN
#define _SM_MEDIA_DECODER_VDECODERCHAIN
#include "Data/ArrayList.h"
#include "Media/Decoder/VDecoderBase.h"

namespace Media
{
	namespace Decoder
	{
		class VDecoderChain : public VDecoderBase
		{
		private:
			Data::ArrayList<Media::IVideoSource *> *srcFilters;

			virtual void ProcVideoFrame(UInt32 frameTime, UInt32 frameNum, UInt8 **imgData, UOSInt dataSize, Media::IVideoSource::FrameStruct frameStruct, Media::FrameType frameType, Media::IVideoSource::FrameFlag flags, Media::YCOffset ycOfst);
		public:
			VDecoderChain(IVideoSource *sourceVideo);
			virtual ~VDecoderChain();

			virtual const UTF8Char *GetFilterName();

			void AddDecoder(Media::IVideoSource *decoder);
			virtual Bool CaptureImage(ImageCallback imgCb, void *userData);

			virtual OSInt GetFrameCount(); //-1 = unknown;
			virtual UInt32 GetFrameTime(UOSInt frameIndex);
			virtual void EnumFrameInfos(FrameInfoCallback cb, void *userData);

			virtual Bool GetVideoInfo(Media::FrameInfo *info, Int32 *frameRateNorm, Int32 *frameRateDenorm, UOSInt *maxFrameSize);
		};
	}
}
#endif