#ifndef _SM_MEDIA_ASIOOUTRENDERER
#define _SM_MEDIA_ASIOOUTRENDERER
#include "Sync/Event.h"
#include "Media/IAudioRenderer.h"

namespace Media
{
	class ASIOOutRenderer : public IAudioRenderer
	{
	private:
		void *asiodrv;
		const WChar *drvName;
		void *bufferInfos;
		Int32 *sampleTypes;
		Int32 bufferSize;
		Bool bufferCreated;
		Bool playing;
		Bool toStop;
		Media::RefClock *clk;
		Int32 debug;
		Bool threadInit;

		Media::IAudioSource *audSrc;
		Int32 bufferIndex;
		Int64 bufferOfst;
		Bool bufferFilled;
		Sync::Event *bufferEvt;

		EndNotifier endHdlr;
		void *endHdlrObj;

		Int32 buffTime;

		static Int32 GetDeviceIndex(const UTF8Char *buff);
		static UInt32 __stdcall PlayThread(void *obj);
		void InitDevice(Int32 devId);

	public:
		static OSInt GetDeviceCount();
		static UTF8Char *GetDeviceName(UTF8Char *buff, OSInt devNo);

		Int64 SwitchBuffer(Int32 index);

		ASIOOutRenderer(const UTF8Char *devName);
		ASIOOutRenderer(Int32 devId);
		virtual ~ASIOOutRenderer();

		virtual Bool IsError();
		virtual Bool BindAudio(Media::IAudioSource *audsrc);
		virtual void AudioInit(Media::RefClock *clk);
		virtual void Start();
		virtual void Stop();
		virtual Bool IsPlaying();
		virtual void SetEndNotify(EndNotifier endHdlr, void *endHdlrObj);
		virtual void SetBufferTime(Int32 ms);

		virtual Int32 GetDeviceVolume();
		virtual void SetDeviceVolume(Int32 volume);

		void Test();
	};
};
#endif