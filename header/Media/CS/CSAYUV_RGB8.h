#ifndef _SM_MEDIA_CS_CSAYUV_RGB8
#define _SM_MEDIA_CS_CSAYUV_RGB8
#include "Media/CS/CSYUV_RGB8.h"
#include "Sync/Event.h"

namespace Media
{
	namespace CS
	{
		class CSAYUV_RGB8 : public Media::CS::CSYUV_RGB8
		{
		private:
			typedef struct
			{
				Sync::Event *evt;
				OSInt status; // 0 = not running, 1 = idling, 2 = toExit, 3 = converting, 4 = finished
				UInt8 *yPtr;
				UInt8 *dest;
				UOSInt width;
				UOSInt height;
				OSInt dbpl;
			} THREADSTAT;

			UOSInt currId;
			UOSInt nThread;
			Sync::Event *evtMain;
			THREADSTAT *stats;

			static UInt32 __stdcall WorkerThread(void *obj);
		public:
			CSAYUV_RGB8(const Media::ColorProfile *srcColor, const Media::ColorProfile *destColor, Media::ColorProfile::YUVType yuvType, Media::ColorManagerSess *colorSess);
			virtual ~CSAYUV_RGB8();
			virtual void ConvertV2(UInt8 **srcPtr, UInt8 *destPtr, UOSInt dispWidth, UOSInt dispHeight, UOSInt srcStoreWidth, UOSInt srcStoreHeight, OSInt destRGBBpl, Media::FrameType ftype, Media::YCOffset ycOfst);
			virtual UOSInt GetSrcFrameSize(UOSInt width, UOSInt height);
		};
	}
}
#endif