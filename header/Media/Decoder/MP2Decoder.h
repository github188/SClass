#ifndef _SM_MEDIA_DECODER_MP2DECODER
#define _SM_MEDIA_DECODER_MP2DECODER
#include "Media/Decoder/ADecoderBase.h"

namespace Media
{
	namespace Decoder
	{
		class MP2Decoder : public Media::Decoder::ADecoderBase
		{
		private:
			Int32 nChannel;
			void *context;
			UOSInt blkSize;
			Int64 totalReadSize;

			Sync::Event *readEvt;
		public:
			MP2Decoder(IAudioSource *sourceAudio);
			virtual ~MP2Decoder();

			virtual void GetFormat(AudioFormat *format);

			virtual Int32 SeekToTime(Int32 time); //ms, ret actual time
			virtual Bool Start(Sync::Event *evt, UOSInt blkSize);
			virtual void Stop();
			virtual UOSInt ReadBlock(UInt8 *buff, UOSInt blkSize); //ret actual block size
			virtual UOSInt GetMinBlockSize();
		};
	}
}
#endif