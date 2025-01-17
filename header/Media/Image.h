#ifndef _SM_MEDIA_IMAGE
#define _SM_MEDIA_IMAGE
#include "Media/FrameInfo.h"
#include "Media/EXIFData.h"

namespace Media
{
	class StaticImage;

	class Image
	{
	public:
		typedef enum
		{
			IT_STATIC,
			IT_GUIIMAGE
		} ImageType;

	public:
		Media::FrameInfo *info;
		Media::EXIFData *exif;
		Bool hasHotSpot;
		OSInt hotSpotX;
		OSInt hotSpotY;
		UInt8 *pal;
		
	public:
		Image(UOSInt dispWidth, UOSInt dispHeight, UOSInt storeWidth, UOSInt storeHeight, UInt32 fourcc, UInt32 bpp, Media::PixelFormat pf, UOSInt maxSize, const Media::ColorProfile *color, Media::ColorProfile::YUVType yuvType, Media::AlphaType atype, Media::YCOffset ycOfst);
		virtual ~Image();

		virtual Media::Image *Clone() = 0;
		virtual Media::Image::ImageType GetImageType() = 0;
		virtual void GetImageData(UInt8 *destBuff, OSInt left, OSInt top, UOSInt width, UOSInt height, UOSInt destBpl, Bool upsideDown) = 0;
		UOSInt GetDataBpl();
		Bool IsUpsideDown();
		void SetHotSpot(OSInt hotSpotX, OSInt hotSpotY);
		Bool HasHotSpot();
		OSInt GetHotSpotX();
		OSInt GetHotSpotY();

		Media::StaticImage *CreateStaticImage();
		Media::EXIFData *SetEXIFData(Media::EXIFData *exif);
		void ToString(Text::StringBuilderUTF *sb);

		static const UTF8Char *AlphaTypeGetName(AlphaType atype);
	};
}
#endif
