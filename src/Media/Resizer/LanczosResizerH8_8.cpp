#include "Stdafx.h"
#include "MyMemory.h"
#include "Manage/HiResClock.h"
#include "Math/Math.h"
#include "Media/IImgResizer.h"
#include "Media/ImageCopyC.h"
#include "Media/Resizer/LanczosResizerH8_8.h"
#include "Sync/Event.h"
#include "Sync/Mutex.h"
#include "Sync/Thread.h"
#include <math.h>
#include <float.h>

#define PI 3.141592653589793

extern "C"
{
	void LanczosResizerH8_8_horizontal_filter(const UInt8 *inPt, UInt8 *outPt, OSInt width, OSInt height, OSInt tap, OSInt *index, Int64 *weight, OSInt sstep, OSInt dstep);
	void LanczosResizerH8_8_horizontal_filter8(const UInt8 *inPt, UInt8 *outPt, OSInt width, OSInt height, OSInt tap, OSInt *index, Int64 *weight, OSInt sstep, OSInt dstep);
	void LanczosResizerH8_8_vertical_filter(const UInt8 *inPt, UInt8 *outPt, OSInt width, OSInt height, OSInt tap, OSInt *index, Int64 *weight, OSInt sstep, OSInt dstep);
	void LanczosResizerH8_8_expand(const UInt8 *inPt, UInt8 *outPt, OSInt width, OSInt height, OSInt sstep, OSInt dstep);
	void LanczosResizerH8_8_collapse(const UInt8 *inPt, UInt8 *outPt, OSInt width, OSInt height, OSInt sstep, OSInt dstep);
}

Double Media::Resizer::LanczosResizerH8_8::lanczos3_weight(Double phase, OSInt nTap)
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

void Media::Resizer::LanczosResizerH8_8::setup_interpolation_parameter_v(OSInt nTap, Double source_length, OSInt source_max_pos, UOSInt result_length, LRHPARAMETER *out, OSInt indexSep, Double offsetCorr)
{
	UOSInt i;
	OSInt j;
	OSInt n;
	Double *work;
	Double  sum;
	Double  pos;

	out->length = result_length;
	out->tap = nTap;
	out->weight = MemAllocA(Int64, out->length * out->tap + 1);
	out->index = MemAllocA(OSInt, out->length * out->tap);

	work = MemAlloc(Double, out->tap);

	for(i=0;i<result_length;i++){
		pos = (i+0.5)*source_length;
		pos = pos / result_length + offsetCorr;
		n = (Int32)floor(pos - (nTap / 2 - 0.5));//2.5);
		pos = (n+0.5-pos);
		sum = 0;
		for(j=0;j<out->tap;j++){
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

		for(j=0;j<out->tap;j++){
			Int64 i64tmp = 0xffff & (Int64)((work[j] / sum) * 32767.0);
			out->weight[i * out->tap+j] = (i64tmp << 48) | (i64tmp << 32) | (i64tmp << 16) | i64tmp;
		}
	}

	MemFree(work);
}

void Media::Resizer::LanczosResizerH8_8::setup_decimation_parameter_v(OSInt nTap, Double source_length, OSInt source_max_pos, UOSInt result_length, LRHPARAMETER *out, OSInt indexSep, Double offsetCorr)
{
	UOSInt i;
	OSInt j;
	OSInt n;
	Double *work;
	Double  sum;
	Double  pos, phase;

	out->length = result_length;
	out->tap = (OSInt)Math::Fix((nTap * (source_length) + (result_length - 1)) / result_length);

	out->weight = MemAllocA(Int64, out->length * out->tap);
	out->index = MemAllocA(OSInt, out->length * out->tap);
	
	work = MemAlloc(Double, out->tap);

	for(i = 0; i < result_length; i++)
	{
		pos = (i - (nTap / 2) + 0.5) * source_length / result_length + 0.5;
		n = (OSInt)floor(pos + offsetCorr);
		sum = 0;
		for (j = 0; j < out->tap; j++)
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
		}

		for (j = 0; j < out->tap; j++)
		{
			Int64 i64tmp = 0xffff & (Int64)((work[j] / sum) * 32767.0);
			out->weight[i * out->tap+j] = (i64tmp << 48) | (i64tmp << 32) | (i64tmp << 16) | i64tmp;
		}
	}

	MemFree(work);
}

void Media::Resizer::LanczosResizerH8_8::setup_interpolation_parameter_h(OSInt nTap, Double source_length, OSInt source_max_pos, UOSInt result_length, LRHPARAMETER *out, OSInt indexSep, Double offsetCorr)
{
	UOSInt i;
	OSInt j;
	OSInt n;
	Double *work;
	Double  sum;
	Double  pos;

	out->length = result_length;
	out->tap = nTap;
	if ((result_length & 3) == 0 && out->tap == 8)
	{
		out->weight = MemAllocA(Int64, (out->length >> 2) * 34);
		out->index = MemAllocA(OSInt, out->length);
	}
	else
	{
		out->weight = MemAllocA(Int64, out->length * out->tap + 1);
		out->index = MemAllocA(OSInt, out->length * out->tap);
	}

	work = MemAlloc(Double, out->tap);

	for(i=0;i<result_length;i++){
		pos = (i+0.5)*source_length;
		pos = pos / result_length + offsetCorr;
		n = (Int32)floor(pos - (nTap / 2 - 0.5));//2.5);
		pos = (n+0.5-pos);
		sum = 0;
		if ((result_length & 3) == 0 && out->tap == 8)
		{
			OSInt ind = 34 * (i >> 2);
			OSInt index[8];

			j = 0;
			while (j < out->tap)
			{
				work[j] = lanczos3_weight(pos, nTap);
				index[j] = n;
				sum += work[j];
				pos += 1;
				n++;
				j++;
			}
			while (index[0] < 0)
			{
				work[0] = work[0] + work[1];
				index[0] = index[1];
				j = 2;
				while (j < out->tap)
				{
					work[j - 1] = work[j];
					index[j - 1] = index[j];
					j++;
				}
				index[7] = 0;
				work[7] = 0.0;
			}
			while (index[7] >= source_max_pos)
			{
				work[7] = work[7] + work[6];
				index[7] = index[6];
				j = 6;
				while (j-- > 0)
				{
					work[j + 1] = work[j];
					index[j + 1] = index[j];
				}

				index[0] = index[1] - 1;
				work[0] = 0;
			}
			*(Int32*)&out->weight[ind] = (Int32)(index[0] * indexSep);

			for(j=0;j<out->tap;j++){
				Int64 i64tmp = 0xffff & (Int64)((work[j] / sum) * 32767.0);
				out->weight[ind+j+2] = (i64tmp << 48) | (i64tmp << 32) | (i64tmp << 16) | i64tmp;
			}

			pos = (i + 1.5) * source_length;
			pos = pos / result_length + offsetCorr;
			n = (Int32)Math::Fix(pos - (nTap / 2 - 0.5));//2.5);
			pos = (n + 0.5 - pos);
			sum = 0;

			j = 0;
			while (j < out->tap)
			{
				work[j] = lanczos3_weight(pos, nTap);
				index[j] = n;
				sum += work[j];
				pos += 1;
				n++;
				j++;
			}
			while (index[0] < 0)
			{
				work[0] = work[0] + work[1];
				index[0] = index[1];
				j = 2;
				while (j < out->tap)
				{
					work[j - 1] = work[j];
					index[j - 1] = index[j];
					j++;
				}
				index[7] = 0;
				work[7] = 0.0;
			}
			while (index[7] >= source_max_pos)
			{
				work[7] = work[7] + work[6];
				index[7] = index[6];
				j = 6;
				while (j-- > 0)
				{
					work[j + 1] = work[j];
					index[j + 1] = index[j];
				}

				index[0] = index[1] - 1;
				work[0] = 0;
			}
			((Int32*)&out->weight[ind])[1] = (Int32)(index[0] * indexSep);

			for(j=0;j<out->tap;j++){
				Int64 i64tmp = 0xffff & (Int64)((work[j] / sum) * 32767.0);
				out->weight[ind+j+10] = (i64tmp << 48) | (i64tmp << 32) | (i64tmp << 16) | i64tmp;
			}

			pos = (i + 2.5) * source_length;
			pos = pos / result_length + offsetCorr;
			n = (Int32)Math::Fix(pos - (nTap / 2 - 0.5));//2.5);
			pos = (n + 0.5 - pos);
			sum = 0;

			j = 0;
			while (j < out->tap)
			{
				work[j] = lanczos3_weight(pos, nTap);
				index[j] = n;
				sum += work[j];
				pos += 1;
				n++;
				j++;
			}
			while (index[0] < 0)
			{
				work[0] = work[0] + work[1];
				index[0] = index[1];
				j = 2;
				while (j < out->tap)
				{
					work[j - 1] = work[j];
					index[j - 1] = index[j];
					j++;
				}
				index[7] = 0;
				work[7] = 0.0;
			}
			while (index[7] >= source_max_pos)
			{
				work[7] = work[7] + work[6];
				index[7] = index[6];
				j = 6;
				while (j-- > 0)
				{
					work[j + 1] = work[j];
					index[j + 1] = index[j];
				}

				index[0] = index[1] - 1;
				work[0] = 0;
			}
			((Int32*)&out->weight[ind])[2] = (Int32)(index[0] * indexSep);

			for(j=0;j<out->tap;j++){
				Int64 i64tmp = 0xffff & (Int64)((work[j] / sum) * 32767.0);
				out->weight[ind+j+18] = (i64tmp << 48) | (i64tmp << 32) | (i64tmp << 16) | i64tmp;
			}

			pos = (i + 3.5) * source_length;
			pos = pos / result_length + offsetCorr;
			n = (Int32)Math::Fix(pos - (nTap / 2 - 0.5));//2.5);
			pos = (n + 0.5 - pos);
			sum = 0;

			j = 0;
			while (j < out->tap)
			{
				work[j] = lanczos3_weight(pos, nTap);
				index[j] = n;
				sum += work[j];
				pos += 1;
				n++;
				j++;
			}
			while (index[0] < 0)
			{
				work[0] = work[0] + work[1];
				index[0] = index[1];
				j = 2;
				while (j < out->tap)
				{
					work[j - 1] = work[j];
					index[j - 1] = index[j];
					j++;
				}
				index[7] = 0;
				work[7] = 0.0;
			}
			while (index[7] >= source_max_pos)
			{
				work[7] = work[7] + work[6];
				index[7] = index[6];
				j = 6;
				while (j-- > 0)
				{
					work[j + 1] = work[j];
					index[j + 1] = index[j];
				}

				index[0] = index[1] - 1;
				work[0] = 0;
			}
			((Int32*)&out->weight[ind])[3] = (Int32)(index[0] * indexSep);

			for(j=0;j<out->tap;j++){
				Int64 i64tmp = 0xffff & (Int64)((work[j] / sum) * 32767.0);
				out->weight[ind+j+26] = (i64tmp << 48) | (i64tmp << 32) | (i64tmp << 16) | i64tmp;
			}
			i += 3;
		}
		else
		{
			for(j=0;j<out->tap;j++){
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

			for(j=0;j<out->tap;j++){
				Int64 i64tmp = 0xffff & (Int64)((work[j] / sum) * 32767.0);
				out->weight[i * out->tap+j] = (i64tmp << 48) | (i64tmp << 32) | (i64tmp << 16) | i64tmp;
			}
		}
	}

	MemFree(work);
}

void Media::Resizer::LanczosResizerH8_8::setup_decimation_parameter_h(OSInt nTap, Double source_length, OSInt source_max_pos, UOSInt result_length, LRHPARAMETER *out, OSInt indexSep, Double offsetCorr)
{
	UOSInt i;
	OSInt j;
	OSInt n;
	Double *work;
	Double  sum;
	Double  pos, phase;

	out->length = result_length;
	out->tap = (OSInt)Math::Fix((nTap * (source_length) + (result_length - 1)) / result_length);

	if ((result_length & 3) == 0 && out->tap == 8)
	{
		out->weight = MemAllocA(Int64, (out->length >> 2) * 34);
		out->index = MemAllocA(OSInt, out->length);
	}
	else
	{
		out->weight = MemAllocA(Int64, out->length * out->tap);
		out->index = MemAllocA(OSInt, out->length * out->tap);
	}
	
	work = MemAlloc(Double, out->tap);

	for(i = 0; i < result_length; i++)
	{
		pos = (i - (nTap / 2) + 0.5) * source_length / result_length + 0.5;
		n = (OSInt)floor(pos + offsetCorr);
		sum = 0;
		if ((result_length & 3) == 0 && out->tap == 8)
		{
			OSInt ind = 34 * (i >> 2);
			OSInt index[8];
			j = 0;
			while (j < 8)
			{
				phase = ((n + 0.5) * result_length / source_length) - (i + 0.5);
				index[j] = n;
				work[j] = lanczos3_weight(phase, nTap);
				sum += work[j];
				n += 1;
				j++;
			}
			while (index[0] < 0)
			{
				work[0] = work[0] + work[1];
				index[0] = index[1];
				j = 2;
				while (j < out->tap)
				{
					work[j - 1] = work[j];
					index[j - 1] = index[j];
					j++;
				}
				index[7] = 0;
				work[7] = 0.0;
			}
			while (index[7] >= source_max_pos - 1)
			{
				work[7] = work[7] + work[6];
				index[7] = index[6];
				j = 6;
				while (j-- > 0)
				{
					work[j + 1] = work[j];
					index[j + 1] = index[j];
				}

				index[0] = index[1] - 1;
				work[0] = 0;
			}
			((Int32*)&out->weight[ind])[0] = (Int32)(index[0] * indexSep);

			for (j = 0; j < out->tap; j++)
			{
				Int64 i64tmp = 0xffff & (Int64)((work[j] / sum) * 32767.0);
				out->weight[ind + j + 2] = (i64tmp << 48) | (i64tmp << 32) | (i64tmp << 16) | i64tmp;
			}

			pos = (i - (nTap / 2) + 1.5) * source_length / result_length + 0.5;
			n = (OSInt)floor(pos + offsetCorr);
			sum = 0;
			j = 0;
			while (j < 8)
			{
				phase = ((n + 0.5) * result_length / source_length) - (i + 1.5);
				index[j] = n;
				work[j] = lanczos3_weight(phase, nTap);
				sum += work[j];
				n += 1;
				j++;
			}
			while (index[0] < 0)
			{
				work[0] = work[0] + work[1];
				index[0] = index[1];
				j = 2;
				while (j < out->tap)
				{
					work[j - 1] = work[j];
					index[j - 1] = index[j];
					j++;
				}
				index[7] = 0;
				work[7] = 0.0;
			}
			while (index[7] >= source_max_pos - 1)
			{
				work[7] = work[7] + work[6];
				index[7] = index[6];
				j = 6;
				while (j-- > 0)
				{
					work[j + 1] = work[j];
					index[j + 1] = index[j];
				}

				index[0] = index[1] - 1;
				work[0] = 0;
			}
			((Int32*)&out->weight[ind])[1] = (Int32)(index[0] * indexSep);

			for (j = 0; j < out->tap; j++)
			{
				Int64 i64tmp = 0xffff & (Int64)((work[j] / sum) * 32767.0);
				out->weight[ind + j + 10] = (i64tmp << 48) | (i64tmp << 32) | (i64tmp << 16) | i64tmp;
			}

			pos = (i - (nTap / 2) + 2.5) * source_length / result_length + 0.5;
			n = (OSInt)floor(pos + offsetCorr);
			sum = 0;
			j = 0;
			while (j < 8)
			{
				phase = ((n + 0.5) * result_length / source_length) - (i + 2.5);
				index[j] = n;
				work[j] = lanczos3_weight(phase, nTap);
				sum += work[j];
				n += 1;
				j++;
			}
			while (index[0] < 0)
			{
				work[0] = work[0] + work[1];
				index[0] = index[1];
				j = 2;
				while (j < out->tap)
				{
					work[j - 1] = work[j];
					index[j - 1] = index[j];
					j++;
				}
				index[7] = 0;
				work[7] = 0.0;
			}
			while (index[7] >= source_max_pos - 1)
			{
				work[7] = work[7] + work[6];
				index[7] = index[6];
				j = 6;
				while (j-- > 0)
				{
					work[j + 1] = work[j];
					index[j + 1] = index[j];
				}

				index[0] = index[1] - 1;
				work[0] = 0;
			}
			((Int32*)&out->weight[ind])[2] = (Int32)(index[0] * indexSep);

			for (j = 0; j < out->tap; j++)
			{
				Int64 i64tmp = 0xffff & (Int64)((work[j] / sum) * 32767.0);
				out->weight[ind + j + 18] = (i64tmp << 48) | (i64tmp << 32) | (i64tmp << 16) | i64tmp;
			}

			pos = (i - (nTap / 2) + 3.5) * source_length / result_length + 0.5;
			n = (OSInt)floor(pos + offsetCorr);
			sum = 0;
			j = 0;
			while (j < 8)
			{
				phase = ((n + 0.5) * result_length / source_length) - (i + 3.5);
				index[j] = n;
				work[j] = lanczos3_weight(phase, nTap);
				sum += work[j];
				n += 1;
				j++;
			}
			while (index[0] < 0)
			{
				work[0] = work[0] + work[1];
				index[0] = index[1];
				j = 2;
				while (j < out->tap)
				{
					work[j - 1] = work[j];
					index[j - 1] = index[j];
					j++;
				}
				index[7] = 0;
				work[7] = 0.0;
			}
			while (index[7] >= source_max_pos - 1)
			{
				work[7] = work[7] + work[6];
				index[7] = index[6];
				j = 6;
				while (j-- > 0)
				{
					work[j + 1] = work[j];
					index[j + 1] = index[j];
				}

				index[0] = index[1] - 1;
				work[0] = 0;
			}
			((Int32*)&out->weight[ind])[3] = (Int32)(index[0] * indexSep);

			for (j = 0; j < out->tap; j++)
			{
				Int64 i64tmp = 0xffff & (Int64)((work[j] / sum) * 32767.0);
				out->weight[ind + j + 26] = (i64tmp << 48) | (i64tmp << 32) | (i64tmp << 16) | i64tmp;
			}
			i += 3;
		}
		else
		{
			for (j = 0; j < out->tap; j++)
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
			}

			for (j = 0; j < out->tap; j++)
			{
				Int64 i64tmp = 0xffff & (Int64)((work[j] / sum) * 32767.0);
				out->weight[i * out->tap+j] = (i64tmp << 48) | (i64tmp << 32) | (i64tmp << 16) | i64tmp;
			}
		}
	}

	MemFree(work);
}

void Media::Resizer::LanczosResizerH8_8::mt_horizontal_filter(UInt8 *inPt, UInt8 *outPt, UOSInt width, UOSInt height, OSInt tap, OSInt *index, Int64 *weight, OSInt sstep, OSInt dstep)
{
	OSInt currHeight;
	OSInt lastHeight = height;
	OSInt i = this->nThread;
	Manage::HiResClock clk;
	while (i-- > 0)
	{
		currHeight = MulDivOS(i, height, this->nThread);
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
	this->hTime = clk.GetTimeDiff();
}

void Media::Resizer::LanczosResizerH8_8::mt_horizontal_filter8(UInt8 *inPt, UInt8 *outPt, UOSInt width, UOSInt height, OSInt tap, OSInt *index, Int64 *weight, OSInt sstep, OSInt dstep)
{
	OSInt currHeight;
	OSInt lastHeight = height;
	OSInt i = this->nThread;
	Manage::HiResClock clk;
	while (i-- > 0)
	{
		currHeight = MulDivOS(i, height, this->nThread);
		this->params[i].inPt = inPt + currHeight * sstep;
		this->params[i].outPt = outPt + currHeight * dstep;
		this->params[i].width = width;
		this->params[i].height = lastHeight - currHeight;
		this->params[i].tap = tap;
		this->params[i].index = index;
		this->params[i].weight = weight;
		this->params[i].sstep = sstep;
		this->params[i].dstep = dstep;

		this->params[i].funcType = 13;
		this->ptask->AddTask(DoTask, &this->params[i]);
		lastHeight = currHeight;
	}
	this->ptask->WaitForIdle();
	this->hTime = clk.GetTimeDiff();
}

void Media::Resizer::LanczosResizerH8_8::mt_vertical_filter(UInt8 *inPt, UInt8 *outPt, UOSInt width, UOSInt height, OSInt tap, OSInt *index, Int64 *weight, OSInt sstep, OSInt dstep)
{
	OSInt currHeight;
	OSInt lastHeight = height;
	OSInt i = this->nThread;
	Manage::HiResClock clk;
	while (i-- > 0)
	{
		currHeight = MulDivOS(i, height, this->nThread);
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
	this->vTime = clk.GetTimeDiff();
}

void Media::Resizer::LanczosResizerH8_8::mt_expand(UInt8 *inPt, UInt8 *outPt, UOSInt width, UOSInt height, OSInt sstep, OSInt dstep)
{
	OSInt currHeight;
	OSInt lastHeight = height;
	OSInt i = this->nThread;
	while (i-- > 0)
	{
		currHeight = MulDivOS(i, height, this->nThread);
		this->params[i].inPt = inPt + currHeight * sstep;
		this->params[i].outPt = outPt + currHeight * dstep;
		this->params[i].width = width;
		this->params[i].height = lastHeight - currHeight;
		this->params[i].sstep = sstep;
		this->params[i].dstep = dstep;

		this->params[i].funcType = 7;
		this->ptask->AddTask(DoTask, &this->params[i]);
		lastHeight = currHeight;
	}
	this->ptask->WaitForIdle();

}

void Media::Resizer::LanczosResizerH8_8::mt_collapse(UInt8 *inPt, UInt8 *outPt, UOSInt width, UOSInt height, OSInt sstep, OSInt dstep)
{
	OSInt currHeight;
	OSInt lastHeight = height;
	OSInt i = this->nThread;
	while (i-- > 0)
	{
		currHeight = MulDivOS(i, height, this->nThread);
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

void Media::Resizer::LanczosResizerH8_8::mt_copy(UInt8 *inPt, UInt8 *outPt, UOSInt width, UOSInt height, OSInt sstep, OSInt dstep)
{
	OSInt currHeight;
	OSInt lastHeight = height;
	OSInt i = this->nThread;
	while (i-- > 0)
	{
		currHeight = MulDivOS(i, height, this->nThread);
		this->params[i].inPt = inPt + currHeight * sstep;
		this->params[i].outPt = outPt + currHeight * dstep;
		this->params[i].width = width;
		this->params[i].height = lastHeight - currHeight;
		this->params[i].sstep = sstep;
		this->params[i].dstep = dstep;

		this->params[i].funcType = 11;
		this->ptask->AddTask(DoTask, &this->params[i]);
		lastHeight = currHeight;
	}
	this->ptask->WaitForIdle();
}

void __stdcall Media::Resizer::LanczosResizerH8_8::DoTask(void *obj)
{
	TaskParam *ts = (TaskParam*)obj;
	if (ts->funcType == 3)
	{
		LanczosResizerH8_8_horizontal_filter(ts->inPt, ts->outPt, ts->width, ts->height, ts->tap, ts->index, ts->weight, ts->sstep, ts->dstep);
	}
	else if (ts->funcType == 5)
	{
		LanczosResizerH8_8_vertical_filter(ts->inPt, ts->outPt, ts->width, ts->height, ts->tap, ts->index, ts->weight, ts->sstep, ts->dstep);
	}
	else if (ts->funcType == 7)
	{
		LanczosResizerH8_8_expand(ts->inPt, ts->outPt, ts->width, ts->height, ts->sstep, ts->dstep);
	}
	else if (ts->funcType == 9)
	{
		LanczosResizerH8_8_collapse(ts->inPt, ts->outPt, ts->width, ts->height, ts->sstep, ts->dstep);
	}
	else if (ts->funcType == 11)
	{
		ImageCopy_ImgCopy(ts->inPt, ts->outPt, ts->width << 2, ts->height, ts->sstep, ts->dstep);
	}
	else if (ts->funcType == 13)
	{
		LanczosResizerH8_8_horizontal_filter8(ts->inPt, ts->outPt, ts->width, ts->height, ts->tap, ts->index, ts->weight, ts->sstep, ts->dstep);
	}
}

void Media::Resizer::LanczosResizerH8_8::DestoryHori()
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

void Media::Resizer::LanczosResizerH8_8::DestoryVert()
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

Media::Resizer::LanczosResizerH8_8::LanczosResizerH8_8(OSInt hnTap, OSInt vnTap, Media::AlphaType srcAlphaType) : Media::IImgResizer(srcAlphaType)
{
	OSInt i;
	this->nThread = Sync::Thread::GetThreadCnt();

	if (this->nThread > 6)
	{
		this->nThread = 6;
	}

	this->hnTap = hnTap << 1;
	this->vnTap = vnTap << 1;
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
	this->hTime = 0;
	this->vTime = 0;
	NEW_CLASS(mut, Sync::Mutex());
}

Media::Resizer::LanczosResizerH8_8::~LanczosResizerH8_8()
{
	DEL_CLASS(this->ptask);
	MemFree(this->params);

	DestoryHori();
	DestoryVert();
	if (buffPtr)
	{
		MemFreeA64(buffPtr);
		buffPtr = 0;
	}
	DEL_CLASS(mut);
}

void Media::Resizer::LanczosResizerH8_8::Resize(UInt8 *src, OSInt sbpl, Double swidth, Double sheight, Double xOfst, Double yOfst, UInt8 *dest, OSInt dbpl, UOSInt dwidth, UOSInt dheight)
{
	LRHPARAMETER prm;
	if (dwidth < 1 || dheight < 1)
		return;

	Double w = xOfst + swidth;
	Double h = yOfst + sheight;
	UOSInt siWidth = (OSInt)w;
	UOSInt siHeight = (OSInt)h;
	w -= siWidth;
	h -= siHeight;
	if (w > 0)
		siWidth++;
	if (h > 0)
		siHeight++;

	if (siWidth != dwidth && siHeight != dheight)
	{
		mut->Lock();
		if (this->hsSize != swidth || this->hdSize != dwidth || this->hsOfst != xOfst)
		{
			DestoryHori();

			if (swidth > dwidth)
			{
				setup_decimation_parameter_h(this->hnTap, swidth, siWidth, dwidth, &prm, 4, xOfst);
			}
			else
			{
				setup_interpolation_parameter_h(this->hnTap, swidth, siWidth, dwidth,&prm, 4, xOfst);
			}
			hsSize = swidth;
			hdSize = dwidth;
			hsOfst = xOfst;
			hIndex = prm.index;
			hWeight = prm.weight;
			hTap = prm.tap;
		}

		if (this->vsSize != sheight || this->vdSize != dheight || this->vsStep != sbpl || this->vsOfst != yOfst)
		{
			DestoryVert();

			if (sheight > dheight)
			{
				setup_decimation_parameter_v(this->vnTap, sheight, siHeight, dheight, &prm, sbpl, yOfst);
			}
			else
			{
				setup_interpolation_parameter_v(this->vnTap, sheight, siHeight, dheight, &prm, sbpl, yOfst);
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
		if (dheight < 16)
		{
			LanczosResizerH8_8_vertical_filter(src, buffPtr, siWidth, dheight, vTap, vIndex, vWeight, sbpl, siWidth << 3);
			LanczosResizerH8_8_horizontal_filter(buffPtr, dest, dwidth, dheight, hTap,hIndex, hWeight, siWidth << 3, dbpl);
		}
		else
		{
			mt_vertical_filter(src, buffPtr, siWidth, dheight, vTap, vIndex, vWeight, sbpl, siWidth << 3);
//			LanczosResizerH8_8_vertical_filter(src, buffPtr, siWidth, dheight, vTap, vIndex, vWeight, sbpl, siWidth << 3);
//			LanczosResizerH8_8_horizontal_filter(buffPtr, dest, dwidth, dheight, hTap,hIndex, hWeight, siWidth << 3, dbpl);
			mt_horizontal_filter(buffPtr, dest, dwidth, dheight, hTap,hIndex, hWeight, siWidth << 3, dbpl);
		}
		mut->Unlock();
	}
	else if (siWidth != dwidth)
	{
		mut->Lock();
		if (hsSize != swidth || hdSize != dwidth || hsOfst != xOfst)
		{
			DestoryHori();

			if (swidth > dwidth)
			{
				setup_decimation_parameter_h(this->hnTap, swidth, siWidth, dwidth, &prm, 4, xOfst);
			}
			else
			{
				setup_interpolation_parameter_h(this->hnTap, swidth, siWidth, dwidth, &prm, 4, xOfst);
			}
			hsSize = swidth;
			hdSize = dwidth;
			hsOfst = xOfst;
			hIndex = prm.index;
			hWeight = prm.weight;
			hTap = prm.tap;
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
		if (sheight < 16)
		{
			LanczosResizerH8_8_horizontal_filter8(src, dest, dwidth, siHeight, hTap, hIndex, hWeight, sbpl, dbpl);
		}
		else
		{
			mt_horizontal_filter8(src, dest, dwidth, siHeight, hTap, hIndex, hWeight, sbpl, dbpl);
		}
		mut->Unlock();
	}
	else if (siHeight != dheight)
	{
		mut->Lock();
		if (vsSize != sheight || vdSize != dheight || vsStep != sbpl || vsOfst != yOfst)
		{
			DestoryVert();

			if (sheight > dheight)
			{
				setup_decimation_parameter_v(this->vnTap, sheight, siHeight, dheight, &prm, sbpl, yOfst);
			}
			else
			{
				setup_interpolation_parameter_v(this->vnTap, sheight, siHeight, dheight, &prm, sbpl, yOfst);
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
		if (dheight < 16)
		{
			LanczosResizerH8_8_vertical_filter(src, buffPtr, siWidth, dheight, vTap, vIndex, vWeight, sbpl, siWidth << 3);
			LanczosResizerH8_8_collapse(buffPtr, dest, siWidth, dheight, siWidth << 3, dbpl);
		}
		else
		{
			mt_vertical_filter(src, buffPtr, siWidth, dheight, vTap, vIndex, vWeight, sbpl, siWidth << 3);
			mt_collapse(buffPtr, dest, siWidth, dheight, siWidth << 3, dbpl);
		}
		mut->Unlock();
	}
	else
	{
		ImageCopy_ImgCopy(src, dest, siWidth << 2, dheight, sbpl, dbpl);
	}
}

Bool Media::Resizer::LanczosResizerH8_8::Resize(Media::StaticImage *srcImg, Media::StaticImage *destImg)
{
	if (srcImg->info->fourcc != 0 && srcImg->info->fourcc != *(UInt32*)"DIB")
		return false;
	if (destImg->info->fourcc != 0 && destImg->info->fourcc != *(UInt32*)"DIB")
		return false;
	if (srcImg->info->pf != Media::PF_B8G8R8A8 || destImg->info->pf != Media::PF_B8G8R8A8)
		return false;
	if (srcImg->info->fourcc == destImg->info->fourcc)
	{
		Resize(srcImg->data, srcImg->GetDataBpl(), Math::OSInt2Double(srcImg->info->dispWidth), Math::OSInt2Double(srcImg->info->dispHeight), 0, 0, destImg->data, destImg->GetDataBpl(), destImg->info->dispWidth, destImg->info->dispHeight);
		return true;
	}
	else
	{
		OSInt dbpl = destImg->GetDataBpl();
		Resize(srcImg->data, srcImg->GetDataBpl(), Math::OSInt2Double(srcImg->info->dispWidth), Math::OSInt2Double(srcImg->info->dispHeight), 0, 0, destImg->data + (destImg->info->storeHeight - 1) * dbpl, -dbpl, destImg->info->dispWidth, destImg->info->dispHeight);
		return true;
	}
}

Bool Media::Resizer::LanczosResizerH8_8::IsSupported(Media::FrameInfo *srcInfo)
{
	if (srcInfo->fourcc != 0)
		return false;
	if (srcInfo->pf != Media::PF_B8G8R8A8)
		return false;
	return true;
}

Media::StaticImage *Media::Resizer::LanczosResizerH8_8::ProcessToNewPartial(Media::StaticImage *srcImage, Double srcX1, Double srcY1, Double srcX2, Double srcY2)
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
		targetHeight = Math::Double2Int32(srcX2 - srcX1);//srcImage->info->height;
	}
	CalOutputSize(srcImage->info, targetWidth, targetHeight, &destInfo, rar);
	NEW_CLASS(img, Media::StaticImage(&destInfo));
	if (srcImage->exif)
	{
		img->exif = srcImage->exif->Clone();
	}
	Int32 tlx = (Int32)srcX1;
	Int32 tly = (Int32)srcY1;
	Resize(srcImage->data + (tlx << 2) + tly * srcImage->GetDataBpl(), srcImage->GetDataBpl(), srcX2 - srcX1, srcY2 - srcY1, srcX1 - tlx, srcY1 - tly, img->data, img->GetDataBpl(), img->info->dispWidth, img->info->dispHeight);
	if (img->exif)
	{
		img->exif->SetWidth((UInt32)img->info->dispWidth);
		img->exif->SetHeight((UInt32)img->info->dispHeight);
	}
	return img;
}

Double Media::Resizer::LanczosResizerH8_8::GetHAvgTime()
{
	return this->hTime;
}

Double Media::Resizer::LanczosResizerH8_8::GetVAvgTime()
{
	return this->vTime;
}