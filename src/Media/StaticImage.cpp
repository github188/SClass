#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ByteTool.h"
#include "Math/Math.h"
#include "Media/IImgResizer.h"
#include "Media/ImageTo8Bit.h"
#include "Media/ImageUtil.h"
#include "Media/StaticImage.h"
//#include <stdio.h>

Media::StaticImage::StaticImage(UOSInt dispWidth, UOSInt dispHeight, UInt32 fourcc, UInt32 bpp, Media::PixelFormat pf, UOSInt maxSize, const Media::ColorProfile *color, Media::ColorProfile::YUVType yuvType, Media::AlphaType atype, Media::YCOffset ycOfst) : Media::Image(dispWidth, dispHeight, 0, 0, fourcc, bpp, pf, maxSize, color, yuvType, atype, ycOfst)
{
	this->data = MemAllocA(UInt8, info->byteSize + 4);
}

Media::StaticImage::StaticImage(Media::FrameInfo *imgInfo) : Media::Image(imgInfo->dispWidth, imgInfo->dispHeight, imgInfo->storeWidth, imgInfo->storeHeight, imgInfo->fourcc, imgInfo->storeBPP, imgInfo->pf, imgInfo->byteSize, imgInfo->color, imgInfo->yuvType, imgInfo->atype, imgInfo->ycOfst)
{
	this->info->par2 = imgInfo->par2;
	this->info->hdpi = imgInfo->hdpi;
	this->info->vdpi = imgInfo->vdpi;
	this->info->ftype = imgInfo->ftype;
	this->data = MemAllocA(UInt8, info->byteSize + 4);
}

Media::StaticImage::~StaticImage()
{
	MemFreeA(data);
}

Media::Image *Media::StaticImage::Clone()
{
	Media::StaticImage *img;
	NEW_CLASS(img, Media::StaticImage(this->info));
	MemCopyANC(img->data, this->data, info->byteSize + 4);
	if (info->fourcc == 0)
	{
		if (this->pal)
		{
			MemCopyNO(img->pal, this->pal, 4 << info->storeBPP);
		}
	}
	if (this->hasHotSpot)
	{
		img->SetHotSpot(this->hotSpotX, this->hotSpotY);
	}
	return img;
}

Media::Image::ImageType Media::StaticImage::GetImageType()
{
	return Media::Image::IT_STATIC;
}

void Media::StaticImage::GetImageData(UInt8 *destBuff, OSInt left, OSInt top, OSInt width, OSInt height, OSInt destBpl)
{
	OSInt srcBpl = this->GetDataBpl();
	if (this->info->pf == Media::PF_PAL_1_A1 || this->info->pf == Media::PF_PAL_2_A1 || this->info->pf == Media::PF_PAL_4_A1 || this->info->pf == Media::PF_PAL_8_A1)
	{
		if (left < 0)
		{
//			destBuff += (-left * (this->info->storeBPP - 1) >> 3;
			width += left;
			left = 0;
		}
		if (top < 0)
		{
			destBuff += -top * destBpl;
			height += top;
			top = 0;
		}
		if (left + width > (OSInt)this->info->dispWidth)
		{
			width = this->info->dispWidth - left;
		}
		if (top + height > (OSInt)this->info->dispHeight)
		{
			height = this->info->dispHeight - top;
		}
		if (width > 0 && height > 0)
		{
			UInt8 *srcBuff = this->data;
			UInt8 *srcBuff2 = srcBuff + ((this->info->storeWidth * (this->info->storeBPP - 1) + 7) >> 3);
			OSInt lineSize1 = (width * (this->info->storeBPP - 1) + 7) >> 3;
			OSInt lineSize2 = (width + 7) >> 3;
			srcBuff = srcBuff + ((left * (this->info->storeBPP - 1)) >> 3) + top * srcBpl;
			srcBuff2 = srcBuff2 + (left >> 3) + top * srcBpl;
			while (height-- > 0)
			{
				MemCopyNANC(destBuff, srcBuff, lineSize1);
				MemCopyNANC(destBuff + lineSize1, srcBuff2, lineSize2);
				srcBuff += srcBpl;
				srcBuff2 += srcBpl;
				destBuff += destBpl;
			}
		}
	}
	else
	{
		if (left < 0)
		{
			destBuff += (-left * this->info->storeBPP) >> 3;
			width += left;
			left = 0;
		}
		if (top < 0)
		{
			destBuff += -top * destBpl;
			height += top;
			top = 0;
		}
		if (left + width > (OSInt)this->info->dispWidth)
		{
			width = this->info->dispWidth - left;
		}
		if (top + height > (OSInt)this->info->dispHeight)
		{
			height = this->info->dispHeight - top;
		}
		if (width > 0 && height > 0)
		{
			UInt8 *srcBuff = this->data;
			OSInt lineSize = (width * this->info->storeBPP + 7) >> 3;
			srcBuff = srcBuff + ((left * this->info->storeBPP) >> 3) + top * srcBpl;
			while (height-- > 0)
			{
				MemCopyNANC(destBuff, srcBuff, lineSize);
				srcBuff += srcBpl;
				destBuff += destBpl;
			}
		}
	}
}

Bool Media::StaticImage::To32bpp()
{
	if (info->fourcc != 0)
		return false;

	OSInt dispWidth = this->info->dispWidth;
	OSInt dispHeight = this->info->dispHeight;
	OSInt storeWidth = this->info->storeWidth;
	UInt8 *pBits = (UInt8*)this->data;
	UInt8 *buff;
	UInt8 *pal = this->pal;
	OSInt buffSize;
	switch (info->pf)
	{
	case Media::PF_PAL_1:
	case Media::PF_PAL_W1:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 2) + 4);
		ImageUtil_ConvP1_ARGB32(pBits, buff, dispWidth, dispHeight, storeWidth >> 3, dispWidth << 2, pal);
		MemFreeA(this->data);
		MemFree(this->pal);
		this->pal = 0;
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_PAL_2:
	case Media::PF_PAL_W2:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 2) + 4);
		ImageUtil_ConvP2_ARGB32(pBits, buff, dispWidth, dispHeight, storeWidth >> 2, dispWidth << 2, pal);
		MemFreeA(this->data);
		MemFree(this->pal);
		this->pal = 0;
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_PAL_4:
	case Media::PF_PAL_W4:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 2) + 4);
		ImageUtil_ConvP4_ARGB32(pBits, buff, dispWidth, dispHeight, storeWidth >> 1, dispWidth << 2, pal);
		MemFreeA(this->data);
		MemFree(this->pal);
		this->pal = 0;
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_PAL_8:
	case Media::PF_PAL_W8:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 2) + 4);
		ImageUtil_ConvP8_ARGB32(pBits, buff, dispWidth, dispHeight, storeWidth, dispWidth << 2, pal);
		MemFreeA(this->data);
		MemFree(this->pal);
		this->pal = 0;
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_LE_R5G5B5:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 2) + 4);
		ImageUtil_ConvB5G5R5_ARGB32(pBits, buff, dispWidth, dispHeight, storeWidth << 1, dispWidth << 2);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_LE_R5G6B5:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 2) + 4);
		ImageUtil_ConvB5G6R5_ARGB32(pBits, buff, dispWidth, dispHeight, storeWidth << 1, dispWidth << 2);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_B8G8R8:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 2));
		ImageUtil_ConvB8G8R8_ARGB32(pBits, buff, dispWidth, dispHeight, storeWidth * 3, dispWidth << 2);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_R8G8B8:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 2));
		ImageUtil_ConvR8G8B8_ARGB32(pBits, buff, dispWidth, dispHeight, storeWidth * 3, dispWidth << 2);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_R8G8B8A8:
		ImageUtil_SwapRGB(this->data, this->info->storeWidth * this->info->storeHeight, 32);
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_B8G8R8A8:
		return true;
	case Media::PF_B8G8R8A1:
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_LE_B16G16R16:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 2));
		ImageUtil_ConvARGB48_32(pBits, buff, dispWidth, dispHeight, storeWidth * 6, dispWidth << 2);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_LE_B16G16R16A16:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 2));
		ImageUtil_ConvARGB64_32(pBits, buff, dispWidth, dispHeight, storeWidth << 3, dispWidth << 2);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_LE_W16:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 2));
		ImageUtil_ConvW16_ARGB32(pBits, buff, dispWidth, dispHeight, storeWidth << 1, dispWidth << 2);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_LE_W16A16:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 2));
		ImageUtil_ConvW16A16_ARGB32(pBits, buff, dispWidth, dispHeight, storeWidth << 2, dispWidth << 2);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_W8A8:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 2));
		ImageUtil_ConvW8A8_ARGB32(pBits, buff, dispWidth, dispHeight, storeWidth << 1, dispWidth << 2);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_LE_A2B10G10R10:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 2));
		ImageUtil_ConvA2B10G10R10_32(pBits, buff, dispWidth, dispHeight, storeWidth << 2, dispWidth << 2);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_LE_FB32G32R32A32:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 2));
		ImageUtil_ConvFB32G32R32A32_32(pBits, buff, dispWidth, dispHeight, storeWidth << 4, dispWidth << 2);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_LE_FB32G32R32:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 2));
		ImageUtil_ConvFB32G32R32_32(pBits, buff, dispWidth, dispHeight, storeWidth * 12, dispWidth << 2);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_LE_FW32A32:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 2));
		ImageUtil_ConvFW32A32_32(pBits, buff, dispWidth, dispHeight, storeWidth << 3, dispWidth << 2);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_LE_FW32:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 2));
		ImageUtil_ConvFW32_32(pBits, buff, dispWidth, dispHeight, storeWidth << 2, dispWidth << 2);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_PAL_1_A1:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 2) + 4);
		ImageUtil_ConvP1_A1_ARGB32(pBits, buff, dispWidth, dispHeight, storeWidth, dispWidth << 2, pal);
		MemFreeA(this->data);
		MemFree(this->pal);
		this->pal = 0;
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_PAL_2_A1:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 2) + 4);
		ImageUtil_ConvP2_A1_ARGB32(pBits, buff, dispWidth, dispHeight, storeWidth, dispWidth << 2, pal);
		MemFreeA(this->data);
		MemFree(this->pal);
		this->pal = 0;
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_PAL_4_A1:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 2) + 4);
		ImageUtil_ConvP4_A1_ARGB32(pBits, buff, dispWidth, dispHeight, storeWidth, dispWidth << 2, pal);
		MemFreeA(this->data);
		MemFree(this->pal);
		this->pal = 0;
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	case Media::PF_PAL_8_A1:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 2) + 4);
		ImageUtil_ConvP8_A1_ARGB32(pBits, buff, dispWidth, dispHeight, storeWidth, dispWidth << 2, pal);
		MemFreeA(this->data);
		MemFree(this->pal);
		this->pal = 0;
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 2);
		this->info->storeBPP = 32;
		this->info->pf = Media::PF_B8G8R8A8;
		return true;
	default:
		return false;
	}
}

Bool Media::StaticImage::To64bpp()
{
	if (info->fourcc != 0)
		return false;

	OSInt dispWidth = this->info->dispWidth;
	OSInt dispHeight = this->info->dispHeight;
	OSInt storeWidth = this->info->storeWidth;
	UInt8 *pBits = (UInt8*)this->data;
	UInt8 *buff;
	OSInt buffSize;
	switch (info->pf)
	{
	case Media::PF_PAL_1:
	case Media::PF_PAL_W1:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 3) + 4);
		ImageUtil_ConvP1_ARGB64(pBits, buff, dispWidth, dispHeight, storeWidth >> 3, dispWidth << 3, this->pal);
		MemFreeA(this->data);
		MemFree(this->pal);
		this->pal = 0;
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispWidth << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_PAL_2:
	case Media::PF_PAL_W2:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 3) + 4);
		ImageUtil_ConvP2_ARGB64(pBits, buff, dispWidth, dispHeight, storeWidth >> 2, dispWidth << 3, this->pal);
		MemFreeA(this->data);
		MemFree(this->pal);
		this->pal = 0;
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispWidth << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_PAL_4:
	case Media::PF_PAL_W4:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 3) + 4);
		ImageUtil_ConvP4_ARGB64(pBits, buff, dispWidth, dispHeight, storeWidth >> 1, dispWidth << 3, this->pal);
		MemFreeA(this->data);
		MemFree(this->pal);
		this->pal = 0;
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispWidth << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_PAL_8:
	case Media::PF_PAL_W8:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 3) + 4);
		ImageUtil_ConvP8_ARGB64(pBits, buff, dispWidth, dispHeight, storeWidth, dispWidth << 3, this->pal);
		MemFreeA(this->data);
		MemFree(this->pal);
		this->pal = 0;
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispWidth << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_LE_R5G5B5:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 3) + 4);
		ImageUtil_ConvB5G5R5_ARGB64(pBits, buff, dispWidth, dispHeight, storeWidth << 1, dispWidth << 3);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispWidth << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_LE_R5G6B5:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 3) + 4);
		ImageUtil_ConvB5G6R5_ARGB64(pBits, buff, dispWidth, dispHeight, storeWidth << 1, dispWidth << 3);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispWidth << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_B8G8R8:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 3));
		ImageUtil_ConvB8G8R8_ARGB64(pBits, buff, dispWidth, dispHeight, storeWidth * 3, dispWidth << 3);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispWidth << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_R8G8B8:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 3));
		ImageUtil_ConvR8G8B8_ARGB64(pBits, buff, dispWidth, dispHeight, storeWidth * 3, dispWidth << 3);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispWidth << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_B8G8R8A1:
	case Media::PF_B8G8R8A8:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 3));
		ImageUtil_ConvARGB32_64(pBits, buff, dispWidth, dispHeight, storeWidth << 2, dispWidth << 3);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispWidth << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_LE_B16G16R16:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 3));
		ImageUtil_ConvARGB48_64(pBits, buff, dispWidth, dispHeight, storeWidth * 6, dispWidth << 3);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispWidth << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_LE_B16G16R16A16:
		return true;
	case Media::PF_LE_W16:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 3));
		ImageUtil_ConvW16_ARGB64(pBits, buff, dispWidth, dispHeight, storeWidth << 1, dispWidth << 3);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_LE_W16A16:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 3));
		ImageUtil_ConvW16A16_ARGB64(pBits, buff, dispWidth, dispHeight, storeWidth << 2, dispWidth << 3);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_W8A8:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 3));
		ImageUtil_ConvW8A8_ARGB64(pBits, buff, dispWidth, dispHeight, storeWidth << 1, dispWidth << 3);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_LE_A2B10G10R10:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 3));
		ImageUtil_ConvA2B10G10R10_64(pBits, buff, dispWidth, dispHeight, storeWidth << 2, dispWidth << 3);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_LE_FB32G32R32A32:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 3));
		ImageUtil_ConvFB32G32R32A32_64(pBits, buff, dispWidth, dispHeight, storeWidth << 4, dispWidth << 3);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_LE_FB32G32R32:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 3));
		ImageUtil_ConvFB32G32R32_64(pBits, buff, dispWidth, dispHeight, storeWidth * 12, dispWidth << 3);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_LE_FW32A32:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 3));
		ImageUtil_ConvFW32A32_64(pBits, buff, dispWidth, dispHeight, storeWidth << 3, dispWidth << 3);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_LE_FW32:
		buff = MemAllocA(UInt8, buffSize = (dispWidth * dispHeight << 3));
		ImageUtil_ConvFW32_64(pBits, buff, dispWidth, dispHeight, storeWidth << 2, dispWidth << 3);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispHeight << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_PAL_1_A1:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 3) + 4);
		ImageUtil_ConvP1_A1_ARGB64(pBits, buff, dispWidth, dispHeight, storeWidth, dispWidth << 3, this->pal);
		MemFreeA(this->data);
		MemFree(this->pal);
		this->pal = 0;
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispWidth << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_PAL_2_A1:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 3) + 4);
		ImageUtil_ConvP2_A1_ARGB64(pBits, buff, dispWidth, dispHeight, storeWidth, dispWidth << 3, this->pal);
		MemFreeA(this->data);
		MemFree(this->pal);
		this->pal = 0;
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispWidth << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_PAL_4_A1:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 3) + 4);
		ImageUtil_ConvP4_A1_ARGB64(pBits, buff, dispWidth, dispHeight, storeWidth, dispWidth << 3, this->pal);
		MemFreeA(this->data);
		MemFree(this->pal);
		this->pal = 0;
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispWidth << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	case Media::PF_PAL_8_A1:
		buff = MemAllocA(UInt8, (dispWidth * dispHeight << 3) + 4);
		ImageUtil_ConvP8_A1_ARGB64(pBits, buff, dispWidth, dispHeight, storeWidth, dispWidth << 3, this->pal);
		MemFreeA(this->data);
		MemFree(this->pal);
		this->pal = 0;
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = (dispWidth * dispWidth << 3);
		this->info->storeBPP = 64;
		this->info->pf = Media::PF_LE_B16G16R16A16;
		return true;
	default:
		return false;
	}
}

Bool Media::StaticImage::ToPal8()
{
	if (info->fourcc != 0)
		return false;

	UInt8 *buff;
	switch (this->info->pf)
	{
	case Media::PF_LE_FB32G32R32A32:
	case Media::PF_LE_FB32G32R32:
	case Media::PF_LE_B16G16R16A16:
	case Media::PF_LE_B16G16R16:
	case Media::PF_B8G8R8:
		this->To32bpp();
	case Media::PF_B8G8R8A8:
		buff = MemAllocA(UInt8, this->info->dispWidth * this->info->dispHeight);
		if (this->pal == 0)
		{
			this->pal = MemAlloc(UInt8, 1024);
		}
		Media::ImageTo8Bit::From32bpp(this->data, buff, this->pal, this->info->dispWidth, this->info->dispHeight, this->GetDataBpl(), this->info->dispWidth);
		MemFreeA(this->data);
		this->data = buff;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = this->info->dispWidth * this->info->dispHeight;
		this->info->storeBPP = 8;
		this->info->pf = Media::PF_PAL_8;
		return true;
	default:
		return false;
	}
	return false;
}

Bool Media::StaticImage::FillColor(Int32 color)
{
	if (this->info->fourcc == 0 && this->info->storeBPP == 32 && this->info->pf == Media::PF_B8G8R8A8)
	{
		ImageUtil_ColorFill32(this->data, this->info->storeWidth * this->info->storeHeight, color);
		return true;
	}
	return false;
}

Bool Media::StaticImage::MultiplyAlpha(Double alpha)
{
	if (this->info->fourcc == 0 && this->info->storeBPP == 32 && this->info->pf == Media::PF_B8G8R8A8)
	{
		if (this->info->atype == Media::AT_PREMUL_ALPHA)
		{
			return false;
		}
		else if (this->info->atype == Media::AT_NO_ALPHA)
		{
			Int32 i32a = Math::Double2Int32(alpha * 255);
			UInt8 a;
			if (i32a < 0)
				a = 0;
			else if (i32a > 255)
				a = 255;
			else
				a = (UInt8)i32a;
			UInt8 *ptr = this->data;
			OSInt cnt = this->info->storeWidth * this->info->storeHeight;
			while (cnt-- > 0)
			{
				ptr[3] = a;
				ptr += 4;
			}
			this->info->atype = Media::AT_ALPHA;
			return true;
		}
		else
		{
			UInt8 atable[256];
			Int32 i32a;
			OSInt i = 256;
			while (i-- > 0)
			{
				i32a = Math::Double2Int32(alpha * i);
				if (i32a < 0)
					atable[i] = 0;
				else if (i32a > 255)
					atable[i] = 255;
				else
					atable[i] = (UInt8)i32a;
			}
			UInt8 *ptr = this->data;
			i = this->info->storeWidth * this->info->storeHeight;
			while (i-- > 0)
			{
				ptr[3] = atable[ptr[3]];
				ptr += 4;
			}
			return true;
		}
	}
	return false;
}

Bool Media::StaticImage::MultiplyColor(Int32 color)
{
	if (this->info->fourcc == 0 && this->info->storeBPP == 32 && this->info->pf == Media::PF_B8G8R8A8)
	{
		ImageUtil_ImageColorMul32(this->data, this->info->storeWidth, this->info->storeHeight, this->info->storeWidth << 2, color);
		return true;
	}
	return false;
}

Bool Media::StaticImage::Resize(Media::IImgResizer *resizer, UOSInt newWidth, UOSInt newHeight)
{
	if (this->info->fourcc != 0)
		return false;
	if (this->To32bpp())
	{
		UInt8 *outBuff = MemAllocA(UInt8, (newWidth * newHeight << 2) + 4);
		resizer->Resize((UInt8*)this->data, this->info->storeWidth << 2, Math::OSInt2Double(this->info->dispWidth), Math::OSInt2Double(this->info->dispHeight), 0, 0, outBuff, newWidth << 2, newWidth, newHeight);
		MemFreeA(this->data);
		this->data = outBuff;
		this->info->dispWidth = newWidth;
		this->info->dispHeight = newHeight;
		this->info->storeWidth = this->info->dispWidth;
		this->info->storeHeight = this->info->dispHeight;
		this->info->byteSize = newWidth * newHeight << 2;
		return true;
	}
	return false;
}

Bool Media::StaticImage::RotateImage(RotateType rtype)
{
	if (this->info->fourcc == 0)
	{
		OSInt srcWidth = this->info->dispWidth;
		OSInt srcHeight = this->info->dispHeight;
		UInt8 *outBuff;
		if (this->info->storeBPP <= 32 && this->info->pf != Media::PF_LE_W16)
		{
			if (this->To32bpp())
			{
				if (rtype == RT_CW90)
				{
					outBuff = MemAllocA(UInt8, (this->info->dispWidth * this->info->dispHeight << 2) + 4);
					ImageUtil_Rotate32_CW90(this->data, outBuff, this->info->dispWidth, this->info->dispHeight, this->info->storeWidth << 2, this->info->dispHeight << 2);
					MemFreeA(this->data);
					this->data = outBuff;
					this->info->dispWidth = srcHeight;
					this->info->dispHeight = srcWidth;
					this->info->storeWidth = this->info->dispWidth;
					this->info->storeHeight = this->info->dispHeight;
					return true;
				}
				else if (rtype == RT_CW180)
				{
					outBuff = MemAllocA(UInt8, (this->info->dispWidth * this->info->dispHeight << 2) + 4);
					ImageUtil_Rotate32_CW180(this->data, outBuff, this->info->dispWidth, this->info->dispHeight, this->info->storeWidth << 2, this->info->dispWidth << 2);
					MemFreeA(this->data);
					this->data = outBuff;
					this->info->storeWidth = this->info->dispWidth;
					this->info->storeHeight = this->info->dispHeight;
					return true;
				}
				else if (rtype == RT_CW270)
				{
					outBuff = MemAllocA(UInt8, (this->info->dispWidth * this->info->dispHeight << 2) + 4);
					ImageUtil_Rotate32_CW270(this->data, outBuff, this->info->dispWidth, this->info->dispHeight, this->info->storeWidth << 2, this->info->dispHeight << 2);
					MemFreeA(this->data);
					this->data = outBuff;
					this->info->dispWidth = srcHeight;
					this->info->dispHeight = srcWidth;
					this->info->storeWidth = this->info->dispWidth;
					this->info->storeHeight = this->info->dispHeight;
					return true;
				}
			}
		}
		else
		{
			if (this->To64bpp())
			{
				if (rtype == RT_CW90)
				{
					outBuff = MemAllocA(UInt8, (this->info->dispWidth * this->info->dispHeight << 3) + 4);
					ImageUtil_Rotate64_CW90(this->data, outBuff, this->info->dispWidth, this->info->dispHeight, this->info->storeWidth << 3, this->info->dispHeight << 3);
					MemFreeA(this->data);
					this->data = outBuff;
					this->info->dispWidth = srcHeight;
					this->info->dispHeight = srcWidth;
					this->info->storeWidth = this->info->dispWidth;
					this->info->storeHeight = this->info->dispHeight;
					return true;
				}
				else if (rtype == RT_CW180)
				{
					outBuff = MemAllocA(UInt8, (this->info->dispWidth * this->info->dispHeight << 3) + 4);
					ImageUtil_Rotate64_CW180(this->data, outBuff, this->info->dispWidth, this->info->dispHeight, this->info->storeWidth << 3, this->info->dispWidth << 3);
					MemFreeA(this->data);
					this->data = outBuff;
					this->info->storeWidth = this->info->dispWidth;
					this->info->storeHeight = this->info->dispHeight;
					return true;
				}
				else if (rtype == RT_CW270)
				{
					outBuff = MemAllocA(UInt8, (this->info->dispWidth * this->info->dispHeight << 3) + 4);
					ImageUtil_Rotate64_CW270(this->data, outBuff, this->info->dispWidth, this->info->dispHeight, this->info->storeWidth << 3, this->info->dispHeight << 3);
					MemFreeA(this->data);
					this->data = outBuff;
					this->info->dispWidth = srcHeight;
					this->info->dispHeight = srcWidth;
					this->info->storeWidth = this->info->dispWidth;
					this->info->storeHeight = this->info->dispHeight;
					return true;
				}
			}
		}
	}
	return false;
}

Double Media::StaticImage::CalcPSNR(Media::StaticImage *simg)
{
//	printf("CalcPSNR\r\n");
	if (simg->info->dispWidth != this->info->dispWidth)
	{
//		printf("dispWidth not equal\r\n");
		return 0;
	}
	if (simg->info->dispHeight != this->info->dispHeight)
	{
//		printf("dispHeight not equal\r\n");
		return 0;
	}
	if (simg->info->fourcc != 0 || this->info->fourcc != 0)
	{
//		printf("fourcc not valid\r\n");
		return 0;
	}
	if (simg->info->pf != this->info->pf)
	{
//		printf("PixelFormat not equal: %d != %d\r\n", this->info->pf, simg->info->pf);
		return 0;
	}
	if (this->info->pf == Media::PF_LE_B16G16R16A16)
	{
		OSInt i;
		OSInt j;
		Int64 sum = 0;
		Int32 v;
		UInt8 *sptr = this->data;
		UInt8 *dptr = simg->data;
		OSInt sAdd = (this->info->storeWidth - this->info->dispWidth) * 8; 
		OSInt dAdd = (simg->info->storeWidth - simg->info->dispWidth) * 8;
		i = this->info->dispHeight;
		while (i-- > 0)
		{
			j = this->info->dispWidth;
			while (j-- > 0)
			{
				v = (Int32)ReadUInt16(&sptr[0]) - (Int32)ReadUInt16(&dptr[0]);
				sum += v * v;
				v = (Int32)ReadUInt16(&sptr[2]) - (Int32)ReadUInt16(&dptr[2]);
				sum += v * v;
				v = (Int32)ReadUInt16(&sptr[4]) - (Int32)ReadUInt16(&dptr[4]);
				sum += v * v;
				sptr += 8;
				dptr += 8;
			}
			sptr += sAdd;
			dptr += dAdd;
		}
		if (sum == 0)
		{
//			printf("sum = 0\r\n");
			return 0;
		}
		return 20 * Math::Log10(65535) - 10 * Math::Log10(sum / (Double)(this->info->dispWidth * this->info->dispHeight * 3));
	}
	else if (this->info->pf == Media::PF_B8G8R8A8)
	{
		OSInt i;
		OSInt j;
		Int64 sum = 0;
		Int32 v;
		UInt8 *sptr = this->data;
		UInt8 *dptr = simg->data;
		OSInt sAdd = (this->info->storeWidth - this->info->dispWidth) * 4; 
		OSInt dAdd = (simg->info->storeWidth - simg->info->dispWidth) * 4;
		i = this->info->dispHeight;
		while (i-- > 0)
		{
			j = this->info->dispWidth;
			while (j-- > 0)
			{
				v = sptr[0] - (Int32)dptr[0];
				sum += v * v;
				v = sptr[1] - (Int32)dptr[1];
				sum += v * v;
				v = sptr[2] - (Int32)dptr[2];
				sum += v * v;
				sptr += 4;
				dptr += 4;
			}
			sptr += sAdd;
			dptr += dAdd;
		}
		if (sum == 0)
			return 0;
		return 20 * Math::Log10(255) - 10 * Math::Log10(sum / (Double)(this->info->dispWidth * this->info->dispHeight * 3));
	}
	else
	{
//		printf("PixelFormat not support\r\n");
		return 0;
	}
}
