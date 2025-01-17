#ifndef _SM_MEDIA_AUDIOFILTER_AUDIOAMPLIFIER
#define _SM_MEDIA_AUDIOFILTER_AUDIOAMPLIFIER
#include "Media/IAudioFilter.h"
#include "Sync/Mutex.h"

namespace Media
{
	namespace AudioFilter
	{
		class AudioAmplifier : public IAudioFilter
		{
		private:
			UInt32 bitCount;
			Double level;
		public:
			AudioAmplifier(IAudioSource *sourceAudio);
			virtual ~AudioAmplifier();

			virtual void GetFormat(AudioFormat *format);
			virtual UInt32 SeekToTime(UInt32 time); //ms, ret actual time
			virtual UOSInt ReadBlock(UInt8 *buff, UOSInt blkSize); //ret actual block size

			void SetLevel(Double level);
		};
	}
}
#endif
