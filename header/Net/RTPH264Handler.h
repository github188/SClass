#ifndef _SM_NET_RTPH264HANDLER
#define _SM_NET_RTPH264HANDLER
#include "Net/RTPVPLHandler.h"
#include "IO/MemoryStream.h"
#include "Sync/Mutex.h"

namespace Net
{
	class RTPH264Handler : public RTPVPLHandler
	{
	private:
		Int32 payloadType;
		FrameCallback cb;
		FrameChangeCallback fcCb;
		void *cbData;

		UInt32 frameNum;
		Media::FrameInfo *frameInfo;
		Int32 packetMode;

		Sync::Mutex *mut;
		IO::MemoryStream *mstm;
		Int32 lastSeq;
		Bool missSeq;
		UInt8 *sps;
		OSInt spsSize;
		UInt8 *pps;
		OSInt ppsSize;
		Bool isKey;
		Bool firstFrame;


	public:
		RTPH264Handler(Int32 payloadType);
		virtual ~RTPH264Handler();

		virtual void MediaDataReceived(UInt8 *buff, OSInt dataSize, Int32 seqNum, UInt32 ts);
		virtual void SetFormat(const UTF8Char *fmtStr);
		virtual Int32 GetPayloadType();

		virtual UTF8Char *GetSourceName(UTF8Char *buff);
		virtual const UTF8Char *GetFilterName();

		virtual Bool GetVideoInfo(Media::FrameInfo *info, Int32 *frameRateNorm, Int32 *frameRateDenorm, UOSInt *maxFrameSize);
		virtual Bool Init(FrameCallback cb, FrameChangeCallback fcCb, void *userData);
		virtual Bool Start(); //true = succeed
		virtual void Stop();
		virtual Bool IsRunning();

		virtual Int32 GetStreamTime(); //ms, -1 = infinity
		virtual Bool CanSeek();
		virtual Int32 SeekToTime(Int32 time); //ms, ret actual time
		virtual Bool IsRealTimeSrc();
		virtual Bool TrimStream(Int32 trimTimeStart, Int32 trimTimeEnd, Int32 *syncTime);

		virtual OSInt GetDataSeekCount();

		virtual OSInt GetFrameCount(); //-1 = unknown;
		virtual UInt32 GetFrameTime(UOSInt frameIndex);
		virtual void EnumFrameInfos(FrameInfoCallback cb, void *userData);

		virtual OSInt ReadNextFrame(UInt8 *frameBuff, Int32 *frameTime, Media::FrameType *ftype); //ret 0 = no more frames

	};
}
#endif