#ifndef _SM_MEDIA_CS_CSUYVY_LRGBC
#define _SM_MEDIA_CS_CSUYVY_LRGBC
#include "Media/CS/CSYUV_LRGBC.h"
#include "Sync/Event.h"

namespace Media
{
	namespace CS
	{
		class CSUYVY_LRGBC : public Media::CS::CSYUV_LRGBC
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
			CSUYVY_LRGBC(const Media::ColorProfile *srcProfile, const Media::ColorProfile *destProfile, Media::ColorProfile::YUVType yuvType, Media::ColorManagerSess *colorSess);
			virtual ~CSUYVY_LRGBC();
			virtual void ConvertV2(UInt8 **srcPtr, UInt8 *destPtr, UOSInt dispWidth, UOSInt dispHeight, UOSInt srcStoreWidth, UOSInt srcStoreHeight, OSInt destRGBBpl, Media::FrameType ftype, Media::YCOffset ycOfst);
			virtual UOSInt GetSrcFrameSize(UOSInt width, UOSInt height);
		};
	}
}
#endif
