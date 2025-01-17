#ifndef _SM_MEDIA_AVIUTL_AUIPLUGIN
#define _SM_MEDIA_AVIUTL_AUIPLUGIN
#include "Data/ArrayList.h"
#include "IO/IFileSelector.h"
#include "Media/AudioFormat.h"
#include "Media/FrameInfo.h"
#include "Media/IMediaSource.h"

namespace Media
{
	namespace AVIUtl
	{
		class AUIPlugin
		{
		private:
			typedef struct
			{
				void *hMod;
				UInt32 useCnt;
				void *pluginTable;
			} PluginDetail;

		public:
			typedef struct
			{
				void *hand;
				UInt32 useCnt;
			} AUIInput;

		private:
			PluginDetail *plugin;

			AUIPlugin();
			AUIPlugin(AUIPlugin *plugin);
		public:
			~AUIPlugin();

			AUIPlugin *Clone();
			UOSInt LoadFile(const Char *fileName, Data::ArrayList<Media::IMediaSource*> *outArr);

			Bool CloseInput(void *hand);
			Bool GetInputVideoInfo(void *hand, Media::FrameInfo *info, UInt32 *frameRateNorm, UInt32 *frameRateDenorm, UInt32 *frameCnt);
			Bool GetInputAudioInfo(void *hand, Media::AudioFormat *af, UInt32 *sampleCnt);
			UOSInt GetVideoFrame(void *hand, UOSInt frameNum, UInt8 *buff);
			UOSInt GetAudioData(void *hand, UOSInt startSample, UOSInt sampleLength, UInt8 *buff);
			Bool IsVideoKeyFrame(void *hand, UInt32 frameNum);
			Bool ConfigInput(void *hInst, void *hWnd);
			void PrepareSelector(IO::IFileSelector *selector);

			static AUIPlugin *LoadPlugin(const WChar *fileName);
		};
	}
}
#endif
