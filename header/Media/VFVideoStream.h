#ifndef _SM_MEDIA_VFVIDEOSTREAM
#define _SM_MEDIA_VFVIDEOSTREAM
#include "Media/VFPManager.h"
#include "Media/FrameInfo.h"
#include "Media/VideoSourceBase.h"

namespace Media
{
	class VFVideoStream : public VideoSourceBase
	{
	private:
		Media::VFMediaFile *mfile;
		
		Media::FrameInfo *info;
		Int32 frameRate;
		Int32 frameRateScale;
		Int32 frameCnt;
		Bool seeked;

		Int32 currFrameNum;
		Bool playing;
		FrameCallback frameCb;
		FrameChangeCallback fcCb;
		void *frameCbObj;
		Bool threadRunning;
		Bool threadToStop;
		Sync::Event *threadEvt;

		static UInt32 __stdcall PlayThread(void *userObj);
	public:
		VFVideoStream(Media::VFMediaFile *mfile);
		~VFVideoStream();

		virtual UTF8Char *GetSourceName(UTF8Char *buff);
		virtual const UTF8Char *GetFilterName();

		virtual Bool GetVideoInfo(Media::FrameInfo *info, Int32 *frameRateNorm, Int32 *frameRateDenorm, UOSInt *maxFrameSize);
		virtual Bool Init(FrameCallback cb, FrameChangeCallback fcCb, void *userObj);
		virtual Bool Start();
		virtual void Stop();
		virtual Bool IsRunning();

		virtual Int32 GetStreamTime(); //ms, -1 = infinity
		virtual Bool CanSeek();
		virtual Int32 SeekToTime(Int32 time); //ms, ret actual time
		virtual Bool IsRealTimeSrc();
		virtual Bool TrimStream(Int32 trimTimeStart, Int32 trimTimeEnd, Int32 *syncTime);
		virtual Bool SetPreferFrameType(Media::FrameType ftype);

		virtual OSInt GetDataSeekCount();

		virtual OSInt GetFrameCount();
		virtual UInt32 GetFrameTime(UOSInt frameIndex);
		virtual void EnumFrameInfos(FrameInfoCallback cb, void *userData);

		virtual OSInt ReadNextFrame(UInt8 *frameBuff, Int32 *frameTime, Media::FrameType *ftype); //ret 0 = no more frames
	};
}
#endif