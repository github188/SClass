#include "Stdafx.h"
#include "MyMemory.h"
#include "Media/MPEG4V.h"
#include "Media/StaticImage.h"
#include "Media/Decoder/VFWDecoder.h"
#include "Sync/Thread.h"
#include "Text/MyString.h"
#include <windows.h>
#include <vfw.h>

#include "Media/H264Parser.h"
#include "IO/DebugWriter.h"

Bool Media::Decoder::VFWDecoder::GetFCCHandlers(Int32 fourcc, Data::ArrayListInt32 *fccHdlrs, Data::ArrayListInt32 *outFccs, EncodingType *encType)
{
	switch (fourcc)
	{
	case mmioFOURCC('d', 'i', 'v', 'f'):
	case mmioFOURCC('d', 'i', 'v', 'x'):
	case mmioFOURCC('D', 'X', '5', '0'):
	case mmioFOURCC('D', 'I', 'V', 'X'):
	case mmioFOURCC('x', 'v', 'i', 'd'):
	case mmioFOURCC('X', 'V', 'I', 'D'):
	case mmioFOURCC('F', 'M', 'P', '4'):
		fccHdlrs->Add(*(Int32*)"xvid");
		fccHdlrs->Add(*(Int32*)"ffds");
		outFccs->Add(*(Int32*)"YV12");
		outFccs->Add(*(Int32*)"YUY2");
		*encType = ET_MP4V;
		break;
	case mmioFOURCC('M', 'P', '4', '2'):
		fccHdlrs->Add(*(Int32*)"ffds");
		fccHdlrs->Add(*(Int32*)"MP42");
		outFccs->Add(*(Int32*)"YV12");
		outFccs->Add(0);
		*encType = ET_UNKNOWN;
		break;
	case mmioFOURCC('H', '2', '6', '4'):
	case mmioFOURCC('h', '2', '6', '4'):
	case mmioFOURCC('X', '2', '6', '4'):
	case mmioFOURCC('x', '2', '6', '4'):
	case mmioFOURCC('A', 'V', 'C', '1'):
	case mmioFOURCC('a', 'v', 'c', '1'):
	case mmioFOURCC('V', 'S', 'S', 'H'):
		fccHdlrs->Add(*(Int32*)"x264");
		fccHdlrs->Add(*(Int32*)"X264");
		fccHdlrs->Add(*(Int32*)"ffds");
//		outFccs->Add(*(Int32*)"P210");
//		outFccs->Add(*(Int32*)"P216");
//		outFccs->Add(*(Int32*)"P010");
//		outFccs->Add(*(Int32*)"NV12");
		outFccs->Add(*(Int32*)"YV12");
		outFccs->Add(*(Int32*)"YUY2");
		outFccs->Add(*(Int32*)"UYVY");
		outFccs->Add(*(Int32*)"AYUV");
		*encType = ET_H264;
		break;
	case mmioFOURCC('M', 'P', 'G', '2'):
		fccHdlrs->Add(*(Int32*)"MPG2");
		fccHdlrs->Add(*(Int32*)"ffds");
		outFccs->Add(*(Int32*)"YV12");
		outFccs->Add(*(Int32*)"YUY2");
		outFccs->Add(*(Int32*)"UYVY");
		*encType = ET_MP2V;
		break;
	case mmioFOURCC('H', 'E', 'V', 'C'):
	case mmioFOURCC('X', '2', '6', '5'):
	case mmioFOURCC('x', '2', '6', '5'):
	case mmioFOURCC('H', '2', '6', '5'):
	case mmioFOURCC('h', '2', '6', '5'):
		fccHdlrs->Add(*(Int32*)"HEVC");
		fccHdlrs->Add(*(Int32*)"X265");
		fccHdlrs->Add(*(Int32*)"x265");
		fccHdlrs->Add(*(Int32*)"H265");
		fccHdlrs->Add(*(Int32*)"h265");
		outFccs->Add(*(Int32*)"YV12");
		outFccs->Add(*(Int32*)"YUY2");
		outFccs->Add(0);
		break;
	default:
		fccHdlrs->Add(fourcc);
		fccHdlrs->Add(*(Int32*)"ffds");
		outFccs->Add(*(Int32*)"YV12");
		outFccs->Add(*(Int32*)"YUY2");
		outFccs->Add(0);
		*encType = ET_UNKNOWN;
		break;
	}
	return true;
}

void Media::Decoder::VFWDecoder::ProcVideoFrame(UInt32 frameTime, UInt32 frameNum, UInt8 **imgData, UOSInt dataSize, Media::IVideoSource::FrameStruct frameStruct, Media::FrameType frameType, Media::IVideoSource::FrameFlag flags, Media::YCOffset ycOfst)
{
	this->lastYCOfst = ycOfst;
	Media::IVideoSource::FrameFlag bFlags = (Media::IVideoSource::FrameFlag)((flags & (Media::IVideoSource::FF_REALTIME)) | Media::IVideoSource::FF_BFRAMEPROC);

	((BITMAPINFOHEADER*)this->bmihSrc)->biSizeImage = (DWORD)dataSize;
	if (ICDecompress((HIC)this->hic, (frameStruct != Media::IVideoSource::FS_I)?ICDECOMPRESS_NOTKEYFRAME:0, (BITMAPINFOHEADER*)this->bmihSrc, imgData[0], (BITMAPINFOHEADER*)this->bmihDest, this->frameBuff) == ICERR_OK)
	{
		if (((BITMAPINFOHEADER*)this->bmihDest)->biCompression != 0)
		{
			OSInt i;
			if (flags & Media::IVideoSource::FF_DISCONTTIME)
			{
				this->bDiscard = this->bCnt;
			}

			if (this->frameBuff[this->uOfst] < 16 && this->frameBuff[this->vOfst] < 16 && this->bCnt < 20)
			{
				this->bBuff[this->bCnt].frameTime = frameTime;
				this->bBuff[this->bCnt].frameNum = frameNum;
				this->bBuff[this->bCnt].frameType = frameType;
				this->bCnt++;
				this->frameCb(this->bBuff[0].frameTime, this->bBuff[0].frameNum, &this->frameBuff, this->maxFrameSize, frameStruct, this->frameCbData, Media::FT_DISCARD, bFlags, ycOfst);
			}
			else
			{
				if (bCnt == 0)
				{
					if (frameStruct == Media::IVideoSource::FS_B)
					{
						this->bBuff[this->bCnt].frameTime = frameTime;
						this->bBuff[this->bCnt].frameNum = frameNum;
						this->bBuff[this->bCnt].frameType = frameType;
						this->bCnt++;
						this->frameCb(this->bBuff[0].frameTime, this->bBuff[0].frameNum, &this->frameBuff, this->maxFrameSize, frameStruct, this->frameCbData, Media::FT_DISCARD, bFlags, ycOfst);
					}
					else
					{
						this->frameCb(frameTime, frameNum, &this->frameBuff, this->maxFrameSize, frameStruct, this->frameCbData, frameType, flags, ycOfst);
					}
				}
				else
				{
					if (this->bDiscard > 0)
					{
						this->frameCb(this->bBuff[0].frameTime, this->bBuff[0].frameNum, &this->frameBuff, this->maxFrameSize, frameStruct, this->frameCbData, Media::FT_DISCARD, bFlags, ycOfst);
						this->bDiscard--;
					}
					else
					{
						this->frameCb(this->bBuff[0].frameTime, this->bBuff[0].frameNum, &this->frameBuff, this->maxFrameSize, frameStruct, this->frameCbData, this->bBuff[0].frameType, bFlags, ycOfst);
					}
					i = 1;
					while (i < this->bCnt)
					{
						this->bBuff[i - 1].frameTime = this->bBuff[i].frameTime;
						this->bBuff[i - 1].frameNum = this->bBuff[i].frameNum;
						this->bBuff[i - 1].frameType = this->bBuff[i].frameType;
						i++;
					}
					i = this->bCnt - 1;
					this->bBuff[i].frameTime = frameTime;
					this->bBuff[i].frameNum = frameNum;
					this->bBuff[i].frameType = frameType;
				}
			}
		}
		else
		{
			this->frameCb(frameTime, frameNum, &this->frameBuff, this->maxFrameSize, frameStruct, this->frameCbData, frameType, flags, ycOfst);
		}

		if (this->imgCb)
		{
			Media::FrameInfo info;
			Media::StaticImage *img;
			Int32 frameRateNorm;
			Int32 frameRateDenorm;
			UOSInt maxFrameSize;
			this->GetVideoInfo(&info, &frameRateNorm, &frameRateDenorm, &maxFrameSize);
			NEW_CLASS(img, Media::StaticImage(&info));
			MemCopyNO(img->data, this->frameBuff, this->maxFrameSize);
			this->imgCb(this->imgCbData, frameTime, frameNum, img);
			this->imgCb = 0;
			this->imgCbData = 0;
		}

		if (this->frameChg)// && this->hasBFrame)
		{
//			UInt32 ret;
			this->frameChg = false;
/*			ICDecompressEnd((HIC)this->hic);
			ICClose((HIC)this->hic);
			this->hic = 0;
			this->hic = ICOpen(ICTYPE_VIDEO, this->sourceFCC, ICMODE_DECOMPRESS);
			ret = ICDecompressQuery((HIC)this->hic, (BITMAPINFOHEADER*)this->bmihSrc, 0);
			ret = ICDecompressBegin((HIC)this->hic, (BITMAPINFOHEADER*)this->bmihSrc, (BITMAPINFOHEADER*)this->bmihDest);
			ret = ICDecompress((HIC)this->hic, (frameStruct != Media::IVideoSource::FS_I)?ICDECOMPRESS_NOTKEYFRAME:0, (BITMAPINFOHEADER*)this->bmihSrc, imgData, (BITMAPINFOHEADER*)this->bmihDest, this->frameBuff);*/
		}
	}
	else
	{
//		writer.WriteLine(L"Cannot decode frame");
	}
}

Media::Decoder::VFWDecoder::VFWDecoder(Media::IVideoSource *sourceVideo) : Media::Decoder::VDecoderBase(sourceVideo)
{
	this->sourceVideo = 0;
	this->frameRateNorm = 0;
	this->frameRateDenorm = 0;
	this->maxFrameSize = 0;
	this->bmihSrc = 0;
	this->bmihDest = 0;
	this->hic = 0;
	this->frameCb = 0;
	this->frameCbData = 0;
	this->frameBuff = 0;
	this->imgCb = 0;
	this->imgCbData = 0;
	this->frameChg = false;
	this->endProcessing = false;

	this->bCnt = 0;
	this->bDiscard = 0;
	this->uOfst = 0;
	this->vOfst = 0;
	
/*	this->hasBFrame = false;
	this->bFrameDrop = false;
	this->lastIsB = false;
	this->lastFrameNum = 0;
	this->lastFrameTime = 0;
	this->lastFrameType = Media::FT_NON_INTERLACE;
	this->lastFrameFlags = Media::IVideoSource::FF_DISCONTTIME;*/

	Int32 frameRateNorm;
	Int32 frameRateDenorm;
	UOSInt maxFrameSize;
	Media::FrameInfo frameInfo;
	if (!sourceVideo->GetVideoInfo(&frameInfo, &frameRateNorm, &frameRateDenorm, &maxFrameSize))
		return;
	Data::ArrayListInt32 fccHdlrs;
	Data::ArrayListInt32 outFccs;

	GetFCCHandlers(frameInfo.fourcc, &fccHdlrs, &outFccs, &this->encType);

	BITMAPINFOHEADER bmihOut;
	BITMAPINFOHEADER bmih;
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = (LONG)frameInfo.storeWidth;
	bmih.biHeight = (LONG)frameInfo.storeHeight;
	bmih.biPlanes = 1;
	bmih.biBitCount = frameInfo.storeBPP;
	bmih.biCompression = frameInfo.fourcc;
	bmih.biSizeImage = (DWORD)maxFrameSize;
	bmih.biXPelsPerMeter = 72;
	bmih.biYPelsPerMeter = 72;
	bmih.biClrImportant = 0;
	bmih.biClrUsed = 0;

	OSInt fccIndex = 0;
	OSInt j = fccHdlrs.GetCount();
	Int32 fcc;
	Int32 outFcc;
	OSInt k;
	OSInt l;
	while (fccIndex < j)
	{
		fcc = fccHdlrs.GetItem(fccIndex);
		fccIndex++;

		HIC hic = ICOpen(ICTYPE_VIDEO, fcc, ICMODE_DECOMPRESS);
		if (hic == 0)
		{
			continue;
		}

		if (ICERR_OK != ICDecompressQuery(hic, &bmih, 0))
		{
			ICClose(hic);
			continue;
		}
		MemCopyNO(&bmihOut, &bmih, bmih.biSize);

		k = 0;
		l = outFccs.GetCount();
		while (k < l)
		{
			outFcc = outFccs.GetItem(k);
			if (outFcc == *(Int32*)"NV12")
			{
				if (fcc == *(Int32*)"ffds")
				{
					outFcc = *(Int32*)"YV12";
					bmihOut.biCompression = *(UInt32*)"YV12";
				}
				else
				{
					bmihOut.biCompression = *(UInt32*)"NV12";
				}
				bmihOut.biBitCount = 12;
				bmihOut.biSizeImage = (bmihOut.biWidth * bmihOut.biHeight * 3) >> 1;
				this->vOfst = bmihOut.biWidth * bmihOut.biHeight;
				this->uOfst = (bmihOut.biWidth * bmihOut.biHeight >> 2) + this->vOfst;
			}
			else if (outFcc == *(Int32*)"YV12")
			{
				bmihOut.biCompression = *(UInt32*)"YV12";
				bmihOut.biBitCount = 12;
				bmihOut.biSizeImage = (bmihOut.biWidth * bmihOut.biHeight * bmihOut.biBitCount) >> 3;
				this->vOfst = bmihOut.biWidth * bmihOut.biHeight;
				this->uOfst = (bmihOut.biWidth * bmihOut.biHeight >> 2) + this->vOfst;
			}
			else if (outFcc == *(Int32*)"YUY2")
			{
				bmihOut.biCompression = *(UInt32*)"YUY2";
				bmihOut.biBitCount = 16;
				bmihOut.biSizeImage = (bmihOut.biWidth * bmihOut.biHeight * bmihOut.biBitCount) >> 3;
				this->uOfst = 1;
				this->vOfst = 3;
			}
			else if (outFcc == *(Int32*)"UYVY")
			{
				bmihOut.biCompression = *(UInt32*)"UYVY";
				bmihOut.biBitCount = 16;
				bmihOut.biSizeImage = (bmihOut.biWidth * bmihOut.biHeight * bmihOut.biBitCount) >> 3;
				this->uOfst = 0;
				this->vOfst = 2;
			}
			else if (outFcc == *(Int32*)"AYUV")
			{
				bmihOut.biCompression = *(UInt32*)"AYUV";
				bmihOut.biBitCount = 32;
				bmihOut.biSizeImage = (bmihOut.biWidth * bmihOut.biHeight * bmihOut.biBitCount) >> 3;
				this->uOfst = 2;
				this->vOfst = 3;
			}
			else if (outFcc == *(Int32*)"P010")
			{
				bmihOut.biCompression = *(UInt32*)"P010";
				bmihOut.biBitCount = 24;
				bmihOut.biSizeImage = (bmihOut.biWidth * bmihOut.biHeight * bmihOut.biBitCount) >> 3;
			}
			else if (outFcc == *(Int32*)"P210")
			{
				bmihOut.biCompression = *(UInt32*)"P210";
				bmihOut.biBitCount = 32;
				bmihOut.biSizeImage = (bmihOut.biWidth * bmihOut.biHeight * bmihOut.biBitCount) >> 3;
			}
			else if (outFcc == *(Int32*)"P016")
			{
				bmihOut.biCompression = *(UInt32*)"P016";
				bmihOut.biBitCount = 24;
				bmihOut.biSizeImage = (bmihOut.biWidth * bmihOut.biHeight * bmihOut.biBitCount) >> 3;
			}
			else if (outFcc == *(Int32*)"P216")
			{
				bmihOut.biCompression = *(UInt32*)"P216";
				bmihOut.biBitCount = 32;
				bmihOut.biSizeImage = (bmihOut.biWidth * bmihOut.biHeight * bmihOut.biBitCount) >> 3;
			}
			else if (outFcc == *(Int32*)"Y416")
			{
				bmihOut.biCompression = *(UInt32*)"Y416";
				bmihOut.biBitCount = 64;
				bmihOut.biSizeImage = (bmihOut.biWidth * bmihOut.biHeight * bmihOut.biBitCount) >> 3;
			}

			if (ICDecompressBegin(hic, &bmih, &bmihOut) == ICERR_OK)
			{
				this->bmihSrc = MemAlloc(UInt8, bmih.biSize);
				this->bmihDest = MemAlloc(UInt8, bmih.biSize);
				MemCopyNO(this->bmihSrc, &bmih, bmih.biSize);
				MemCopyNO(this->bmihDest, &bmihOut, bmih.biSize);
				this->frameRateNorm = frameRateNorm;
				this->frameRateDenorm = frameRateDenorm;
				if (outFcc == *(Int32*)"YV12" || outFcc == *(Int32*)"NV12")
				{
					Int32 storeWidth;
					Int32 storeHeight;
					if (bmihOut.biWidth & 3)
					{
						storeWidth = bmihOut.biWidth + 4 - (bmihOut.biWidth & 3);
					}
					else
					{
						storeWidth = bmihOut.biWidth;
					}
					if (bmihOut.biHeight & 1)
					{
						storeHeight = bmihOut.biHeight + 1;
					}
					else
					{
						storeHeight = bmihOut.biHeight;
					}

					this->maxFrameSize = (storeWidth * storeHeight * bmihOut.biBitCount) >> 3;
				}
				else
				{
					this->maxFrameSize = (bmihOut.biWidth * bmihOut.biHeight * bmihOut.biBitCount) >> 3;
				}
				this->hic = hic;
				this->sourceFCC = fcc;
				this->sourceVideo = sourceVideo;
				this->frameBuff = MemAllocA64(UInt8, this->maxFrameSize);
				MemClear(this->frameBuff, this->maxFrameSize);
				return;
			}
			k++;
		}
		
		ICClose(hic);
	}
}

Media::Decoder::VFWDecoder::~VFWDecoder()
{
	this->Stop();
	if (this->hic)
	{
		ICDecompressEnd((HIC)this->hic);
		ICClose((HIC)this->hic);
	}
	if (this->bmihSrc)
	{
		MemFree(this->bmihSrc);
		this->bmihSrc = 0;
	}
	if (this->bmihDest)
	{
		MemFree(this->bmihDest);
		this->bmihDest = 0;
	}
	if (this->frameBuff)
	{
		MemFreeA64(this->frameBuff);
		this->frameBuff = 0;
	}
}

Bool Media::Decoder::VFWDecoder::CaptureImage(ImageCallback imgCb, void *userData)
{
	this->imgCb = imgCb;
	this->imgCbData = userData;
	return true;
}

const UTF8Char *Media::Decoder::VFWDecoder::GetFilterName()
{
	return (const UTF8Char*)"VFWDecoder";
}

Bool Media::Decoder::VFWDecoder::GetVideoInfo(Media::FrameInfo *info, Int32 *frameRateNorm, Int32 *frameRateDenorm, UOSInt *maxFrameSize)
{
	if (this->sourceVideo == 0)
		return false;
	if (!this->sourceVideo->GetVideoInfo(info, frameRateNorm, frameRateDenorm, maxFrameSize))
		return false;
	BITMAPINFOHEADER *bmih = (BITMAPINFOHEADER *)this->bmihDest;
	*frameRateNorm = this->frameRateNorm;
	*frameRateDenorm = this->frameRateDenorm;
	*maxFrameSize = this->maxFrameSize;
	info->storeWidth = bmih->biWidth;
	info->storeHeight = bmih->biHeight;
	info->dispWidth = info->storeWidth;
	info->dispHeight = info->storeHeight;
	info->storeBPP = bmih->biBitCount;
	info->fourcc = bmih->biCompression;
	info->pf = Media::FrameInfo::GetDefPixelFormat(bmih->biCompression, bmih->biBitCount);
	info->byteSize = this->maxFrameSize;
	return true;
}

void Media::Decoder::VFWDecoder::Stop()
{
	if (this->sourceVideo == 0)
		return;

	this->started = false;
	this->sourceVideo->Stop();
	while (this->endProcessing)
	{
		Sync::Thread::Sleep(10);
	}
	this->frameCb = 0;
	this->frameCbData = 0;
}


OSInt Media::Decoder::VFWDecoder::GetFrameCount()
{
	return this->sourceVideo->GetFrameCount();
}

UInt32 Media::Decoder::VFWDecoder::GetFrameTime(UOSInt frameIndex)
{
	return this->sourceVideo->GetFrameTime(frameIndex);
}

void Media::Decoder::VFWDecoder::EnumFrameInfos(FrameInfoCallback cb, void *userData)
{
	return this->sourceVideo->EnumFrameInfos(cb, userData);
}

void Media::Decoder::VFWDecoder::OnFrameChanged(Media::IVideoSource::FrameChange fc)
{
	if (fc == Media::IVideoSource::FC_PAR)
	{
		this->frameChg = true;
	}
	else if (fc == Media::IVideoSource::FC_ENDPLAY)
	{
/*		if (this->hasBFrame)
		{
			((BITMAPINFOHEADER*)this->bmihSrc)->biSizeImage = 0;
			if (ICDecompress((HIC)this->hic, ICDECOMPRESS_NOTKEYFRAME, (BITMAPINFOHEADER*)this->bmihSrc, 0, (BITMAPINFOHEADER*)this->bmihDest, this->frameBuff) == ICERR_OK)
			{
				this->frameCb(this->lastFrameTime, this->lastFrameNum, this->frameBuff, this->maxFrameSize, Media::IVideoSource::FS_N, this->frameCbData, this->lastFrameType, this->lastFrameFlags, this->lastYCOfst);
			}
		}*/
		if (this->bCnt > 0)
		{
			OSInt i;
			this->endProcessing = true;
			((BITMAPINFOHEADER*)this->bmihSrc)->biSizeImage = 0;
			i = 0;
			while (i < this->bCnt)
			{
				if (ICDecompress((HIC)this->hic, ICDECOMPRESS_NOTKEYFRAME, (BITMAPINFOHEADER*)this->bmihSrc, 0, (BITMAPINFOHEADER*)this->bmihDest, this->frameBuff) == ICERR_OK)
				{
					this->frameCb(this->bBuff[i].frameTime, this->bBuff[i].frameNum, &this->frameBuff, this->maxFrameSize, Media::IVideoSource::FS_N, this->frameCbData, this->bBuff[i].frameType, Media::IVideoSource::FF_BFRAMEPROC, this->lastYCOfst);
				}
				i++;
			}
			this->endProcessing = false;
		}
	}
}