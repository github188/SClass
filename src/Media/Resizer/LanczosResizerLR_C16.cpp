#include "Stdafx.h"
#include "MyMemory.h"
#include "Math/Math.h"
#include "Sync/Event.h"
#include "Sync/Mutex.h"
#include "Sync/Thread.h"
#include "Media/RGBLUTGen.h"
#include "Media/Resizer/LanczosResizerLR_C16.h"
#include <math.h>
#include <float.h>

#define PI 3.141592653589793

extern "C"
{
	void LanczosResizerLR_C16_horizontal_filter(UInt8 *inPt, UInt8 *outPt,OSInt width, OSInt height, OSInt tap, OSInt *index, Int64 *weight, OSInt sstep, OSInt dstep, UInt8 *rgbTable);
	void LanczosResizerLR_C16_vertical_filter(UInt8 *inPt, UInt8 *outPt, OSInt width, OSInt height, OSInt tap, OSInt *index, Int64 *weight, OSInt sstep, OSInt dstep, UInt8 *rgbTable);
	void LanczosResizerLR_C16_collapse(UInt8 *inPt, UInt8 *outPt, OSInt width, OSInt height, OSInt sstep, OSInt dstep, UInt8 *rgbTable);
}

Double Media::Resizer::LanczosResizerLR_C16::lanczos3_weight(Double phase, OSInt nTap)
{
	Double ret;
	
	if(fabs(phase) < DBL_EPSILON)
	{
		return 1.0;
	}

	if ((fabs(phase) * 2) >= nTap){
		return 0.0;
	}

	ret = sin(PI * phase) * sin(PI * phase / nTap * 2) / (PI * PI * phase * phase / nTap * 2);

	return ret;
}

void Media::Resizer::LanczosResizerLR_C16::setup_interpolation_parameter(OSInt nTap, Double source_length, OSInt source_max_pos, UOSInt result_length, LRHPARAMETER *out, OSInt indexSep, Double offsetCorr)
{
	UOSInt i;
	OSInt j;
	OSInt n;
	Double *work;
	Double  sum;
	Double  pos;

	out->length = result_length;
	out->tap = nTap;
	out->weight = MemAllocA(Int64, out->length * out->tap);
	out->index = MemAllocA(OSInt, out->length * out->tap);

	work = MemAlloc(Double, out->tap);

	i = 0;
	while (i < result_length)
	{
		pos = (i+0.5)*source_length;
		pos = pos / result_length + offsetCorr;
		n = (Int32)floor(pos - (nTap / 2 - 0.5));//2.5);
		pos = (n+0.5-pos);
		sum = 0;
		for(j = 0; j < out->tap; j++)
		{
			if(n < 0){
				out->index[i * out->tap + j] = 0;
			}else if(n >= source_max_pos){
				out->index[i * out->tap + j] = (source_max_pos - 1) * indexSep;
			}else{
				out->index[i * out->tap + j] = n * indexSep;
			}
			work[j] = lanczos3_weight(pos, nTap);
			sum += work[j];
			pos += 1;
			n += 1;
		}

		j = 0;
		while (j < out->tap)
		{
			UInt16 v1 = (UInt16)(0xffff & Math::Double2Int32((work[j] / sum) * 32767.0));
			UInt16 v2 = (UInt16)(0xffff & Math::Double2Int32((work[j + 1] / sum) * 32767.0));
			UInt16 *tmpPtr = (UInt16*)&out->weight[i * out->tap + j];
			tmpPtr[0] = v1;
			tmpPtr[1] = v2;
			tmpPtr[2] = v1;
			tmpPtr[3] = v2;
			tmpPtr[4] = v1;
			tmpPtr[5] = v2;
			tmpPtr[6] = v1;
			tmpPtr[7] = v2;
			j += 2;
		}
		i++;
	}

	MemFree(work);
}

void Media::Resizer::LanczosResizerLR_C16::setup_decimation_parameter(OSInt nTap, Double source_length, OSInt source_max_pos, UOSInt result_length, LRHPARAMETER *out, OSInt indexSep, Double offsetCorr)
{
	UOSInt i;
	OSInt j;
	OSInt n;
	OSInt ttap;
	Double *work;
	Double  sum;
	Double  pos, phase;

	out->length = result_length;
	out->tap = (OSInt)Math::Fix((nTap * (source_length) + (result_length - 1)) / result_length);
	ttap = out->tap;
	out->tap += out->tap & 1;

	out->weight = MemAllocA(Int64, out->length * out->tap);
	out->index = MemAllocA(OSInt, out->length * out->tap);
	
	work = MemAlloc(Double, out->tap);

	i = 0;
	while (i < result_length)
	{
		pos = (i - (nTap / 2) + 0.5) * source_length / result_length + 0.5;
		n = (Int32)floor(pos + offsetCorr);
		sum = 0;
		j = 0;
		while (j < ttap)
		{
			phase = (n+0.5)*result_length;
			phase /= source_length;
			phase -= (i+0.5);
			if(n < 0){
				out->index[i * out->tap + j] = 0;
			}else if(n >= source_max_pos){
				out->index[i * out->tap + j] = (source_max_pos-1) * indexSep;
			}else{
				out->index[i * out->tap + j] = n * indexSep;
			}
			work[j] = lanczos3_weight(phase, nTap);
			sum += work[j];
			n += 1;
			j++;
		}
		if (ttap & 1)
		{
			work[ttap] = 0;
			out->index[i * out->tap + ttap] = out->index[i * out->tap + ttap - 1];
		}

		j = 0;
		while (j < ttap)
		{
			UInt16 v1 = (UInt16)(0xffff & Math::Double2Int32((work[j] / sum) * 32767.0));
			UInt16 v2 = (UInt16)(0xffff & Math::Double2Int32((work[j + 1] / sum) * 32767.0));
			UInt16 *tmpPtr = (UInt16*)&out->weight[i * out->tap + j];
			tmpPtr[0] = v1;
			tmpPtr[1] = v2;
			tmpPtr[2] = v1;
			tmpPtr[3] = v2;
			tmpPtr[4] = v1;
			tmpPtr[5] = v2;
			tmpPtr[6] = v1;
			tmpPtr[7] = v2;
			j += 2;
		}
		i++;
	}

	MemFree(work);
}

void Media::Resizer::LanczosResizerLR_C16::mt_horizontal_filter(UInt8 *inPt, UInt8 *outPt, UOSInt width, UOSInt height, OSInt tap, OSInt *index, Int64 *weight, OSInt sstep, OSInt dstep)
{
	UOSInt currHeight;
	UOSInt lastHeight = height;
	UOSInt i = this->nThread;
	while (i-- > 0)
	{
		currHeight = MulDivUOS(i, height, this->nThread);
		this->params[i].inPt = inPt + currHeight * sstep;
		this->params[i].outPt = outPt + currHeight * dstep;
		this->params[i].width = width;
		this->params[i].height = lastHeight - currHeight;
		this->params[i].tap = tap;
		this->params[i].index = index;
		this->params[i].weight = weight;
		this->params[i].sstep = sstep;
		this->params[i].dstep = dstep;

		this->params[i].funcType = 3;
		this->ptask->AddTask(DoTask, &this->params[i]);
		lastHeight = currHeight;
	}
	this->ptask->WaitForIdle();
}

void Media::Resizer::LanczosResizerLR_C16::mt_vertical_filter(UInt8 *inPt, UInt8 *outPt, UOSInt width, UOSInt height, OSInt tap, OSInt *index, Int64 *weight, OSInt sstep, OSInt dstep)
{
	UOSInt currHeight;
	UOSInt lastHeight = height;
	UOSInt i = this->nThread;
	while (i-- > 0)
	{
		currHeight = MulDivUOS(i, height, this->nThread);
		this->params[i].inPt = inPt;
		this->params[i].outPt = outPt + currHeight * dstep;
		this->params[i].width = width;
		this->params[i].height = lastHeight - currHeight;
		this->params[i].tap = tap;
		this->params[i].index = index + currHeight * tap;
		this->params[i].weight = weight + currHeight * tap;
		this->params[i].sstep = sstep;
		this->params[i].dstep = dstep;

		this->params[i].funcType = 5;
		this->ptask->AddTask(DoTask, &this->params[i]);
		lastHeight = currHeight;
	}
	this->ptask->WaitForIdle();
}

void Media::Resizer::LanczosResizerLR_C16::mt_collapse(UInt8 *inPt, UInt8 *outPt, UOSInt width, UOSInt height, OSInt sstep, OSInt dstep)
{
	UOSInt currHeight;
	UOSInt lastHeight = height;
	UOSInt i = this->nThread;
	while (i-- > 0)
	{
		currHeight = MulDivUOS(i, height, this->nThread);
		this->params[i].inPt = inPt + currHeight * sstep;
		this->params[i].outPt = outPt + currHeight * dstep;
		this->params[i].width = width;
		this->params[i].height = lastHeight - currHeight;
		this->params[i].sstep = sstep;
		this->params[i].dstep = dstep;

		this->params[i].funcType = 9;
		this->ptask->AddTask(DoTask, &this->params[i]);
		lastHeight = currHeight;
	}
	this->ptask->WaitForIdle();
}

void Media::Resizer::LanczosResizerLR_C16::UpdateRGBTable()
{
	if (this->rgbTable == 0)
	{
		this->rgbTable = MemAlloc(UInt8, 65536 * 6);
	}
	Media::RGBLUTGen lutGen(this->colorSess);
	lutGen.GenLRGB_RGB16(this->rgbTable, this->destColor, 14, this->srcRefLuminance);
}

void __stdcall Media::Resizer::LanczosResizerLR_C16::DoTask(void *obj)
{
	TaskParam *ts = (TaskParam*)obj;
	if (ts->funcType == 3)
	{
		LanczosResizerLR_C16_horizontal_filter(ts->inPt, ts->outPt, ts->width, ts->height, ts->tap, ts->index, ts->weight, ts->sstep, ts->dstep, ts->me->rgbTable);
	}
	else if (ts->funcType == 5)
	{
		LanczosResizerLR_C16_vertical_filter(ts->inPt, ts->outPt, ts->width, ts->height, ts->tap, ts->index, ts->weight, ts->sstep, ts->dstep, ts->me->rgbTable);
	}
	else if (ts->funcType == 9)
	{
		LanczosResizerLR_C16_collapse(ts->inPt, ts->outPt, ts->width, ts->height, ts->sstep, ts->dstep, ts->me->rgbTable);
	}
}

void Media::Resizer::LanczosResizerLR_C16::DestoryHori()
{
	if (hIndex)
	{
		MemFreeA(hIndex);
		hIndex = 0;
	}
	if (hWeight)
	{
		MemFreeA(hWeight);
		hWeight = 0;
	}
	hsSize = 0;
}

void Media::Resizer::LanczosResizerLR_C16::DestoryVert()
{
	if (vIndex)
	{
		MemFreeA(vIndex);
		vIndex = 0;
	}
	if (vWeight)
	{
		MemFreeA(vWeight);
		vWeight = 0;
	}
	vsSize = 0;
	vsStep = 0;
}

Media::Resizer::LanczosResizerLR_C16::LanczosResizerLR_C16(OSInt hnTap, OSInt vnTap, const Media::ColorProfile *destColor, Media::ColorManagerSess *colorSess, Media::AlphaType srcAlphaType, Double srcRefLuminance) : Media::IImgResizer(srcAlphaType)
{
	OSInt i;
	nThread = Sync::Thread::GetThreadCnt();
	if (this->nThread > 4)
	{
		this->nThread = this->nThread >> 1;
	}

	this->hnTap = hnTap << 1;
	this->vnTap = vnTap << 1;
	this->rgbChanged = true;
	NEW_CLASS(this->destColor, Media::ColorProfile(destColor));
	if (colorSess)
	{
		this->colorSess = colorSess;
		this->colorSess->AddHandler(this);
	}
	else
	{
		this->colorSess = 0;
	}
	this->srcRefLuminance = srcRefLuminance;
	this->rgbTable = 0;

	this->params = MemAlloc(TaskParam, this->nThread);
	MemClear(this->params, sizeof(TaskParam) * this->nThread);
	i = nThread;
	while(i-- > 0)
	{
		this->params[i].me = this;
	}
	NEW_CLASS(this->ptask, Sync::ParallelTask(this->nThread, false));

	hsSize = 0;
	hsOfst = 0;
	hdSize = 0;
	hIndex = 0;
	hWeight = 0;
	hTap = 0;

	vsSize = 0;
	vsOfst = 0;
	vdSize = 0;
	vsStep = 0;
	vIndex = 0;
	vWeight = 0;
	vTap = 0;

	buffW = 0;
	buffH = 0;
	buffPtr = 0;
	NEW_CLASS(mut, Sync::Mutex());
}

Media::Resizer::LanczosResizerLR_C16::~LanczosResizerLR_C16()
{
	if (this->colorSess)
	{
		this->colorSess->RemoveHandler(this);
	}
	DEL_CLASS(this->ptask);
	MemFree(this->params);

	DestoryHori();
	DestoryVert();
	if (buffPtr)
	{
		MemFreeA64(buffPtr);
		buffPtr = 0;
	}
	if (this->rgbTable)
	{
		MemFree(this->rgbTable);
	}
	DEL_CLASS(this->destColor);
	DEL_CLASS(mut);
}

void Media::Resizer::LanczosResizerLR_C16::Resize(UInt8 *src, OSInt sbpl, Double swidth, Double sheight, Double xOfst, Double yOfst, UInt8 *dest, OSInt dbpl, UOSInt dwidth, UOSInt dheight)
{
	LRHPARAMETER prm;
	Double w;
	Double h;
	UOSInt siWidth;
	UOSInt siHeight;

	if (dwidth < 16 || dheight < 16)
		return;

	w = xOfst + swidth;
	h = yOfst + sheight;
	siWidth = (OSInt)w;
	siHeight = (OSInt)h;
	w -= siWidth;
	h -= siHeight;
	if (w > 0)
		siWidth++;
	if (h > 0)
		siHeight++;

	if (siWidth <= 0 || siHeight <= 0)
		return;
	if (this->rgbChanged)
	{
		this->rgbChanged = false;
		UpdateRGBTable();
	}

	if (swidth != dwidth && sheight != dheight)
	{
		mut->Lock();
		if (this->hsSize != swidth || this->hdSize != dwidth || this->hsOfst != xOfst)
		{
			DestoryHori();

			if (swidth > dwidth)
			{
				setup_decimation_parameter(this->hnTap, swidth, siWidth, dwidth, &prm, 8, xOfst);
			}
			else
			{
				setup_interpolation_parameter(this->hnTap, swidth, siWidth, dwidth,&prm, 8, xOfst);
			}
			hsSize = swidth;
			hdSize = dwidth;
			hsOfst = xOfst;
			hIndex = prm.index;
			hWeight = prm.weight;
			hTap = prm.tap;
		}

		if (this->vsSize != sheight || this->vdSize != dheight || this->vsStep != (OSInt)(dwidth << 3) || this->vsOfst != yOfst)
		{
			DestoryVert();

			if (sheight > dheight)
			{
				setup_decimation_parameter(this->vnTap, sheight, siHeight, dheight, &prm, dwidth << 3, yOfst);
			}
			else
			{
				setup_interpolation_parameter(this->vnTap, sheight, siHeight, dheight, &prm, dwidth << 3, yOfst);
			}
			vsSize = sheight;
			vdSize = dheight;
			vsOfst = yOfst;
			vsStep = dwidth << 3;
			vIndex = prm.index;
			vWeight = prm.weight;
			vTap = prm.tap;
		}
		
		if (siHeight != buffH || (dwidth != buffW))
		{
			if (buffPtr)
			{
				MemFreeA64(buffPtr);
				buffPtr = 0;
			}
			buffW = dwidth;
			buffH = siHeight;
			buffPtr = MemAllocA64(UInt8, buffW * buffH << 3);
		}
		mt_horizontal_filter(src, buffPtr, dwidth, siHeight, hTap,hIndex, hWeight, sbpl, dwidth << 3);
		mt_vertical_filter(buffPtr, dest, dwidth, dheight, vTap, vIndex, vWeight, dwidth << 3, dbpl);
		mut->Unlock();
	}
	else if (swidth != dwidth)
	{
		mut->Lock();
		if (hsSize != swidth || hdSize != dwidth || hsOfst != xOfst)
		{
			DestoryHori();

			if (swidth > dwidth)
			{
				setup_decimation_parameter(this->hnTap, swidth, siWidth, dwidth, &prm, 8, xOfst);
			}
			else
			{
				setup_interpolation_parameter(this->hnTap, swidth, siWidth, dwidth, &prm, 8, xOfst);
			}
			hsSize = swidth;
			hdSize = dwidth;
			hsOfst = xOfst;
			hIndex = prm.index;
			hWeight = prm.weight;
			hTap = prm.tap;
		}
		if (dheight != buffH || (dwidth != buffW))
		{
			if (buffPtr)
			{
				MemFreeA64(buffPtr);
				buffPtr = 0;
			}
			buffW = dwidth;
			buffH = dheight;
			buffPtr = MemAllocA64(UInt8, buffW * buffH << 3);
		}
		mt_horizontal_filter(src, buffPtr, dwidth, siHeight, hTap, hIndex, hWeight, sbpl, dwidth << 3);
		mt_collapse(buffPtr, dest, dwidth, dheight, dwidth << 3, dbpl);
		mut->Unlock();
	}
	else if (sheight != dheight)
	{
		mut->Lock();
		if (vsSize != sheight || vdSize != dheight || vsStep != sbpl || vsOfst != yOfst)
		{
			DestoryVert();

			if (sheight > dheight)
			{
				setup_decimation_parameter(this->vnTap, sheight, siHeight, dheight, &prm, sbpl, yOfst);
			}
			else
			{
				setup_interpolation_parameter(this->vnTap, sheight, siHeight, dheight, &prm, sbpl, yOfst);
			}
			vsSize = sheight;
			vdSize = dheight;
			vsOfst = yOfst;
			vsStep = sbpl;
			vIndex = prm.index;
			vWeight = prm.weight;
			vTap = prm.tap;
		}
		if (dheight != buffH || (siWidth != buffW))
		{
			if (buffPtr)
			{
				MemFreeA64(buffPtr);
				buffPtr = 0;
			}
			buffW = siWidth;
			buffH = dheight;
			buffPtr = MemAllocA64(UInt8, buffW * buffH << 3);
		}
		mt_vertical_filter(src, dest, siWidth, dheight, vTap, vIndex, vWeight, sbpl, dbpl);
		mut->Unlock();
	}
	else
	{
		mut->Lock();
		mt_collapse(src, dest, siWidth, dheight, siWidth << 3, dbpl);
		mut->Unlock();

	}
}

void Media::Resizer::LanczosResizerLR_C16::YUVParamChanged(const Media::IColorHandler::YUVPARAM *yuvParam)
{
}

void Media::Resizer::LanczosResizerLR_C16::RGBParamChanged(const Media::IColorHandler::RGBPARAM2 *rgbParam)
{
	this->rgbChanged = true;
}

void Media::Resizer::LanczosResizerLR_C16::SetSrcRefLuminance(Double srcRefLuminance)
{
	this->srcRefLuminance = srcRefLuminance;
	this->rgbChanged = true;
}

Bool Media::Resizer::LanczosResizerLR_C16::IsSupported(Media::FrameInfo *srcInfo)
{
	if (srcInfo->fourcc != *(UInt32*)"LRGB")
		return false;
	return true;
}

Media::StaticImage *Media::Resizer::LanczosResizerLR_C16::ProcessToNewPartial(Media::StaticImage *srcImage, Double srcX1, Double srcY1, Double srcX2, Double srcY2)
{
	Media::FrameInfo destInfo;
	Media::StaticImage *img;
	if (!IsSupported(srcImage->info))
		return 0;
	OSInt targetWidth = this->targetWidth;
	OSInt targetHeight = this->targetHeight;
	if (targetWidth == 0)
	{
		targetWidth = Math::Double2Int32(srcX2 - srcX1);//srcImage->info->width;
	}
	if (targetHeight == 0)
	{
		targetHeight = Math::Double2Int32(srcY2 - srcY1);//srcImage->info->height;
	}
	CalOutputSize(srcImage->info, targetWidth, targetHeight, &destInfo, rar);
	destInfo.fourcc = 0;
	destInfo.storeBPP = 16;
	destInfo.pf = Media::PF_LE_R5G6B5;
	destInfo.color->GetPrimaries()->Set(srcImage->info->color->GetPrimaries());
	if (this->destColor->GetRTranParam()->GetTranType() != Media::CS::TRANT_VUNKNOWN && this->destColor->GetRTranParam()->GetTranType() != Media::CS::TRANT_PUNKNOWN)
	{
		destInfo.color->GetRTranParam()->Set(this->destColor->GetRTranParam());
		destInfo.color->GetGTranParam()->Set(this->destColor->GetGTranParam());
		destInfo.color->GetBTranParam()->Set(this->destColor->GetBTranParam());
	}
	NEW_CLASS(img, Media::StaticImage(&destInfo));
	Int32 tlx = (Int32)srcX1;
	Int32 tly = (Int32)srcY1;
	Resize(srcImage->data + srcImage->GetDataBpl() * tly + (tlx << 3), srcImage->GetDataBpl(), srcX2 - srcX1, srcY2 - srcY1, srcX1 - tlx, srcY1 - tly, img->data, img->GetDataBpl(), destInfo.dispWidth, destInfo.dispHeight);
	return img;
}