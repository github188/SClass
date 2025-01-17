#ifndef _SM_MEDIA_VIDEOSOURCEBASE
#define _SM_MEDIA_VIDEOSOURCEBASE
#include "Media/IVideoSource.h"

namespace Media
{
	class VideoSourceBase : public Media::IVideoSource
	{
	protected:
		UOSInt cropLeft;
		UOSInt cropTop;
		UOSInt cropRight;
		UOSInt cropBottom;

	public:
		VideoSourceBase();
		virtual ~VideoSourceBase();

		virtual void SetBorderCrop(UOSInt cropLeft, UOSInt cropTop, UOSInt cropRight, UOSInt cropBottom);
		virtual void GetBorderCrop(UOSInt *cropLeft, UOSInt *cropTop, UOSInt *cropRight, UOSInt *cropBottom);
	};
}
#endif
