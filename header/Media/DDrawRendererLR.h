// require ddraw.lib
// require dxguid.lib

#ifndef _SM_MEDIA_DDRAWRENDERERLR
#define _SM_MEDIA_DDRAWRENDERERLR
#include "Sync/Event.h"
#include "Sync/Mutex.h"
#include "Media/FrameInfo.h"
#include "Media/CS/CSConverter.h"
#include "Media/IImgResizer.h"
#include "Media/DeinterlaceLR.h"
#include "Media/RefClock.h"
#include "Media/ColorManager.h"

namespace Media
{
	class DDrawRendererLR
	{
	private:
		typedef struct
		{
			void *surface;
			Int32 frameTime;
		} FrameBuffer;

		typedef struct
		{
			UInt8 *frameData;
			Int32 frameTime;
			Media::FrameType ftype;
		} FrameSource;
	private:
		void *ddObj;
		void *pSurface;
		void *bbSurface;
		void *clipper;

		FrameBuffer *surfaceBuff;
		OSInt surfaceBuffStart;
		OSInt surfaceBuffEnd;
		Sync::Event *surfaceBuffEvent;
		Sync::Mutex *surfaceBuffMut;

		void *hwnd;
		Media::FrameInfo info;
		Media::CS::CSConverter *csconv;
		Media::IImgResizer *resizer;
		Media::DeinterlaceLR *deinterlace;
		Media::RefClock *clk;
		Media::ColorManager *colorMgr;
		FrameSource *frameSources;
		Int32 frameSrcSize;
		UInt8 *frameRGBBuff;
		Int32 frameRGBSize;
		UInt8 *frameDIBuff;
		Bool fs;
		Int32 scnWidth;
		Int32 scnHeight;
		Bool firstFrame;
		Int32 lastFrameTime;
		Int32 fieldInt;
		Bool rendering;
		Int32 surfaceW;
		Int32 surfaceH;
		Bool realtimeMode;

		Bool scnUpdRunning;
		Bool scnUpdToStop;

		Bool frameProcRunning;
		Bool frameProcToStop;
		OSInt frameProcStart;
		OSInt frameProcEnd;
		Sync::Mutex *frameProcMut;
		Sync::Event *frameProcEvent;

	private:
		void GetDrawingRect(void *rc);
		void CreateSurface();
		void ReleaseSurface();
		void CreateSubSurface();
		void ReleaseSubSurface();
		void CreateFrameSrcBuff();
		void ReleaseFrameSrcBuff();
		OSInt GetNextSurface(Int32 frameTime);
		void PutNextSurface(OSInt index);

		static UInt32 __stdcall ScnUpdater(void *obj);
		static UInt32 __stdcall FrameProcesser(void *obj);
	public:
		DDrawRendererLR(void *hwnd, Media::ColorManager *colorMgr);
		~DDrawRendererLR();

		void OnSizeChanged();
		void StartRender(Media::RefClock *clk, Bool realtimeMode);
		void StopRender();
		void ChangeFrameFormat(Media::FrameInfo *info);
		void AddFrame(const UInt8 *frameData, Int32 frameTime, Media::FrameType ftype);
		void RenderFrame();
	};
};
#endif