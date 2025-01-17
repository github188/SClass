#ifndef _SM_MEDIA_IREALTIMEVIDEOSOURCE
#define _SM_MEDIA_IREALTIMEVIDEOSOURCE
#include "Media/IVideoSource.h"

namespace Media
{
	class IRealtimeVideoSource : public IVideoSource
	{
	protected:
		UOSInt cropLeft;
		UOSInt cropTop;
		UOSInt cropRight;
		UOSInt cropBottom;
	public:
		IRealtimeVideoSource();
		virtual ~IRealtimeVideoSource();

		virtual UTF8Char *GetSourceName(UTF8Char *buff) = 0;

		virtual void SetBorderCrop(UOSInt cropLeft, UOSInt cropTop, UOSInt cropRight, UOSInt cropBottom);
		virtual void GetBorderCrop(UOSInt *cropLeft, UOSInt *cropTop, UOSInt *cropRight, UOSInt *cropBottom);

		virtual Bool GetVideoInfo(Media::FrameInfo *info, UInt32 *frameRateNorm, UInt32 *frameRateDenorm, UOSInt *maxFrameSize) = 0;
		virtual Bool Init(FrameCallback cb, FrameChangeCallback fcCb, void *userData) = 0;
		virtual Bool Start() = 0; //true = succeed
		virtual void Stop() = 0;
		virtual Bool IsVideoCapture();

		virtual Int32 GetStreamTime();
		virtual Bool CanSeek();
		virtual UInt32 SeekToTime(UInt32 time);
		virtual Bool IsRealTimeSrc();
		virtual Bool TrimStream(UInt32 trimTimeStart, UInt32 trimTimeEnd, Int32 *syncTime);

		virtual Bool HasFrameCount();
		virtual UOSInt GetFrameCount();
		virtual UInt32 GetFrameTime(UOSInt frameIndex);
		virtual void EnumFrameInfos(FrameInfoCallback cb, void *userData);

		virtual UOSInt ReadNextFrame(UInt8 *frameBuff, UInt32 *frameTime, Media::FrameType *ftype); //ret 0 = no more frames
	};
}
#endif
