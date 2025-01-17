#ifndef _SM_MEDIA_FILEVIDEOSOURCE
#define _SM_MEDIA_FILEVIDEOSOURCE
#include "Data/ArrayListUInt32.h"
#include "Data/ArrayListUInt64.h"
#include "IO/IStreamData.h"
#include "Media/VideoSourceBase.h"
#include "Sync/Mutex.h"

namespace Media
{
	class FileVideoSource : public Media::VideoSourceBase
	{
	private:
		typedef struct
		{
			UInt32 frameTime;
			UInt32 frameNum;
			UInt8 *frameBuff;
			UOSInt frameSize;
			Media::IVideoSource::FrameStruct frameStruct;
			Media::FrameType fType;
			Media::IVideoSource::FrameFlag flags;
			Media::YCOffset ycOfst;
		} OutputFrameInfo;
	private:
		IO::IStreamData *data;
		Media::FrameInfo *frameInfo;
		UInt32 frameRateNorm;
		UInt32 frameRateDenorm;
		UOSInt maxFrameSize;
		UOSInt currFrameSize;
		Bool timeBased;

		Data::ArrayListUInt64 *frameOfsts;
		Data::ArrayListUInt32 *frameSizes;
		Data::ArrayListUInt32 *frameParts;
		Data::ArrayList<Bool> *frameIsKey;
		Data::ArrayListUInt32 *frameTimes;

		UInt32 currFrameNum;
		Bool playing;
		Bool playEnd;
		Bool playToStop;
		FrameCallback playCb;
		FrameChangeCallback fcCb;
		void *playCbData;
		Sync::Event *playEvt;
		Sync::Mutex *pbcMut;

		Bool outputRunning;
		Bool outputToStop;
		Sync::Event *outputEvt;
		Sync::Mutex *outputMut;
		OSInt outputStart;
		OSInt outputCount;
		OutputFrameInfo *outputFrames;

		Sync::Event *mainEvt;

		static UInt32 __stdcall PlayThread(void *userObj);
		static UInt32 __stdcall OutputThread(void *userObj);
	public:
		FileVideoSource(IO::IStreamData *data, Media::FrameInfo *frameInfo, UInt32 frameRateNorm, UInt32 frameRateDenorm, Bool timeBased);
		virtual ~FileVideoSource();

		void AddNewFrame(UInt64 frameOfst, UInt32 frameSize, Bool isKey, UInt32 frameTime);
		void AddFramePart(UInt64 frameOfst, UInt32 frameSize);
		void SetFrameRate(UInt32 frameRateNorm, UInt32 frameRateDenorm);

		virtual UTF8Char *GetSourceName(UTF8Char *buff);
		virtual const UTF8Char *GetFilterName();

		virtual Bool GetVideoInfo(Media::FrameInfo *info, UInt32 *frameRateNorm, UInt32 *frameRateDenorm, UOSInt *maxFrameSize);
		virtual Bool Init(FrameCallback cb, FrameChangeCallback fcCb, void *userData);
		virtual Bool Start(); //true = succeed
		virtual void Stop();
		virtual Bool IsRunning();

		virtual Int32 GetStreamTime(); //ms, -1 = infinity
		virtual Bool CanSeek();
		virtual UInt32 SeekToTime(UInt32 time); //ms, ret actual time
		virtual Bool IsRealTimeSrc();
		virtual Bool TrimStream(UInt32 trimTimeStart, UInt32 trimTimeEnd, Int32 *syncTime);

		virtual UOSInt GetDataSeekCount();

		virtual Bool HasFrameCount();
		virtual UOSInt GetFrameCount();
		virtual UInt32 GetFrameTime(UOSInt frameIndex);
		virtual void EnumFrameInfos(FrameInfoCallback cb, void *userData);
		virtual UOSInt GetFrameSize(UOSInt frameIndex);
		virtual UOSInt ReadFrame(UOSInt frameIndex, UInt8 *frameBuff);

		virtual UOSInt ReadNextFrame(UInt8 *frameBuff, UInt32 *frameTime, Media::FrameType *ftype); //ret 0 = no more frames

	};
}
#endif
