#ifndef _SM_MEDIA_ANDROIDVIDEOCAPTURE
#define _SM_MEDIA_ANDROIDVIDEOCAPTURE
#include "Data/ArrayList.h"
#include "Media/IVideoCapture.h"
#include "Text/StringBuilderUTF.h"

namespace Media
{
	class AndroidVideoCapture : public Media::IVideoCapture
	{
	private:
		void *cameraMgr;
		const Char *cameraId;
		OSInt camWidth;
		OSInt camHeight;
		Int32 camFourcc;
	
		FrameCallback cb;
		FrameChangeCallback fcCb;
		void *userData;

		Bool started;
		void *session;
		void *captureRequest;
		void *outputs;
		void *output;
		void *device;
		void *reader;

		static UInt32 __stdcall PlayThread(void *userObj);
	public:
		AndroidVideoCapture(void *cameraMgr, const Char *cameraId);
		virtual ~AndroidVideoCapture();
		
		virtual UTF8Char *GetSourceName(UTF8Char *buff);
		virtual const UTF8Char *GetFilterName();
		virtual Bool GetVideoInfo(Media::FrameInfo *info, Int32 *frameRateNorm, Int32 *frameRateDenorm, UOSInt *maxFrameSize);
		virtual Bool Init(FrameCallback cb, FrameChangeCallback fcCb, void *userData);
		virtual Bool Start();
		virtual void Stop();
		virtual Bool IsRunning();

		virtual void SetPreferSize(UOSInt width, UOSInt height, UInt32 fourcc, UInt32 bpp, Int32 frameRateNumer, Int32 frameRateDenom);
		virtual UOSInt GetSupportedFormats(VideoFormat *fmtArr, UOSInt maxCnt);
		virtual void GetInfo(Text::StringBuilderUTF *sb);
		virtual OSInt GetDataSeekCount();
	};

	class AndroidVideoCaptureMgr
	{
	private:
		void *cameraMgr;
		void *cameraIdList;
	public:
		AndroidVideoCaptureMgr();
		~AndroidVideoCaptureMgr();

		OSInt GetDeviceList(Data::ArrayList<Int32> *devList);
		UTF8Char *GetDeviceName(UTF8Char *buff, OSInt devId);

		Media::IVideoCapture *CreateDevice(OSInt devId);
	};
}
#endif
