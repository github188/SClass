#ifndef _SM_NET_RTPAACHANDLER
#define _SM_NET_RTPAACHANDLER
//RFC3640

#include "Net/RTPAPLHandler.h"
#include "IO/MemoryStream.h"
#include "Sync/Mutex.h"

namespace Net
{
	class RTPAACHandler : public RTPAPLHandler
	{
	private:
		typedef enum
		{
			AACM_UNKNOWN,
			AACM_GENERIC,
			AACM_CELP_CBR,
			AACM_CELP_VBR,
			AACM_AAC_LBR,
			AACM_AAC_HBR
		} AACMode;
	private:
		Int32 payloadType;
		Int32 freq;
		Int32 nChannel;
		AACMode aacm;
		Int32 streamType;
		Int32 profileId;
		Int32 config;
		UInt8 *buff;
		UOSInt buffSize;

		Sync::Event *evt;
		Sync::Event *dataEvt;

		Sync::Mutex *mut;
		IO::MemoryStream *mstm;
		Int32 lastSeq;
		Bool missSeq;

		UInt8 GetRateIndex();
	public:
		RTPAACHandler(Int32 payloadType, Int32 freq, Int32 nChannel);
		virtual ~RTPAACHandler();

		virtual void MediaDataReceived(UInt8 *buff, OSInt dataSize, Int32 seqNum, UInt32 ts);
		virtual void SetFormat(const UTF8Char *fmtStr);
		virtual Int32 GetPayloadType();

		virtual UTF8Char *GetSourceName(UTF8Char *buff);
		virtual Bool CanSeek();
		virtual Int32 GetStreamTime(); //ms
		virtual Int32 SeekToTime(Int32 time); //ms, ret actual time
		virtual Bool TrimStream(Int32 trimTimeStart, Int32 trimTimeEnd, Int32 *syncTime);

		virtual void GetFormat(Media::AudioFormat *format);

		virtual Bool Start(Sync::Event *evt, UOSInt blkSize);
		virtual void Stop();
		virtual UOSInt ReadBlock(UInt8 *buff, UOSInt blkSize); //ret actual block size
		virtual UOSInt GetMinBlockSize();
		virtual Int32 GetCurrTime();
		virtual Bool IsEnd();
	};
}
#endif