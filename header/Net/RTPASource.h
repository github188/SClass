#ifndef _SM_NET_RTPASOURCE
#define _SM_NET_RTPASOURCE
#include "Media/IAudioSource.h"
#include "Net/RTPCliChannel.h"
#include "Net/RTPAPLHandler.h"

namespace Net
{
	class RTPASource : public Media::IAudioSource
	{
	private:
		Net::RTPCliChannel *ch;
		Net::RTPAPLHandler *hdlr;
	public:
		RTPASource(Net::RTPCliChannel *ch, Net::RTPAPLHandler *hdlr);
		virtual ~RTPASource();

		virtual UTF8Char *GetSourceName(UTF8Char *buff);
		virtual Bool CanSeek();
		virtual Int32 GetStreamTime();
		virtual UInt32 SeekToTime(UInt32 time);
		virtual Bool TrimStream(UInt32 trimTimeStart, UInt32 trimTimeEnd, Int32 *syncTime);

		virtual void GetFormat(Media::AudioFormat *format);

		virtual Bool Start(Sync::Event *evt, UOSInt blkSize);
		virtual void Stop();
		virtual UOSInt ReadBlock(UInt8 *buff, UOSInt blkSize);
		virtual UOSInt GetMinBlockSize();
		virtual UInt32 GetCurrTime();
		virtual Bool IsEnd();
	};
}
#endif
