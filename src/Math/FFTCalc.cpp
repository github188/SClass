#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ByteTool.h"
#include "Math/FFTCalc.h"
#include "Math/Math.h"

extern "C"
{
	void FFTCalc_ApplyWindowI16(Double *complexOut, UInt8 *sampleIn, Double *sampleWindow, OSInt sampleCnt, OSInt sampleAdd, Double sampleMul);
	void FFTCalc_ApplyWindowI24(Double *complexOut, UInt8 *sampleIn, Double *sampleWindow, OSInt sampleCnt, OSInt sampleAdd, Double sampleMul);
	void FFTCalc_FFT2Freq(Double *freq, Double *complexIn, OSInt sampleCnt);
	OSInt FFTCalc_Forward(Double *complexData, OSInt sampleCount);
}

void Math::FFTCalc::BuildSampleWin()
{
	Double pi2;
	Double a0;
	Double a1;
	Double a2;
	Double a3;
	Double a4;
	Double invK;
	OSInt j;
	OSInt k;
	switch (this->wtype)
	{
	default:
	case WT_RECTANGULAR:
		k = this->sampleCount;
		j = 0;
		while (j < k)
		{
			this->sampleWindow[j] = 1.0;
			j++;
		}
		break;

	case WT_TRIANGULAR:
		k = sampleCount >> 1;
		j = 0;
		while (j < k)
		{
			this->sampleWindow[j]= (j + 0.5) / k;
			j++;
		}
		while (j < sampleCount)
		{
			this->sampleWindow[j] = (sampleCount - j + 0.5) / k;
			j++;
		}
		break;

	case WT_HAMMING:
		k = sampleCount - 1;
		pi2 = 2 * Math::PI;
		a0 = 0.53836;
		a1 = 0.46164;
		j = 0;
		while (j < sampleCount)
		{
			this->sampleWindow[j] = (a0 - a1 * Math::Cos(j * pi2 / k));
			j++;
		}
		break;

	case WT_BLACKMANN:
		k = sampleCount - 1;
		pi2 = 2 * Math::PI;
		a0 = 7938.0 / 18608.0;
		a1 = 9240.0 / 18608.0;
		a2 = 1430.0 / 18608.0;
		j = 0;
		while (j < sampleCount)
		{
			this->sampleWindow[j] = (a0 - a1 * Math::Cos(j * pi2 / k) + a2 * Math::Cos(2 * pi2 * j / k));
			j++;
		}
		break;

	case WT_NUTTALL:
		k = sampleCount - 1;
		pi2 = 2 * Math::PI;
		a0 = 0.355768;
		a1 = 0.487396;
		a2 = 0.144232;
		a3 = 0.012604;
		j = 0;
		while (j < sampleCount)
		{
			this->sampleWindow[j] = (a0 - a1 * Math::Cos(j * pi2 / k) + a2 * Math::Cos(2 * pi2 * j / k) - a3 * Math::Cos(3 * pi2 * j / k));
			j++;
		}
		break;

	case WT_BLACKMANN_NUTTALL:
		k = sampleCount - 1;
		pi2 = 2 * Math::PI;
		a0 = 0.3635819;
		a1 = 0.4891775;
		a2 = 0.1365995;
		a3 = 0.0106411;
		j = 0;
		while (j < sampleCount)
		{
			this->sampleWindow[j] = (a0 - a1 * Math::Cos(j * pi2 / k) + a2 * Math::Cos(2 * pi2 * j / k) - a3 * Math::Cos(3 * pi2 * j / k));
			j++;
		}
		break;

	case WT_BLACKMANN_HARRIS:
		k = sampleCount - 1;
		invK = 1.0 / k;
		pi2 = 2 * Math::PI;
		a0 = 0.35875;
		a1 = 0.48829;
		a2 = 0.14128;
		a3 = 0.01168;
		j = 0;
		while (j < sampleCount)
		{
			this->sampleWindow[j] = (a0 - a1 * Math::Cos(j * pi2 * invK) + a2 * Math::Cos(2 * pi2 * j * invK) - a3 * Math::Cos(3 * pi2 * j * invK));
			j++;
		}
		break;

	case WT_FLAT_TOP:
		k = sampleCount - 1;
		pi2 = 2 * Math::PI;
		a0 = 1;
		a1 = 1.93;
		a2 = 1.29;
		a3 = 0.388;
		a4 = 0.028;
		j = 0;
		while (j < sampleCount)
		{
			this->sampleWindow[j] = (a0 - a1 * Math::Cos(j * pi2 / k) + a2 * Math::Cos(2 * pi2 * j / k) - a3 * Math::Cos(3 * pi2 * j / k) + a4 * Math::Cos(4 * pi2 * j / k));
			j++;
		}
		break;
	}
}

Math::FFTCalc::FFTCalc(OSInt sampleCount, WindowType wtype)
{
	this->sampleCount = sampleCount;
	this->wtype = wtype;
	this->sampleWindow = MemAllocA(Double, sampleCount);
	this->sampleTemp = MemAllocA(Double, sampleCount << 1);
	this->BuildSampleWin();
}

Math::FFTCalc::~FFTCalc()
{
	MemFreeA(this->sampleWindow);
	MemFreeA(this->sampleTemp);
}

Bool Math::FFTCalc::ForwardBits(UInt8 *samples, Double *freq, SampleType sampleType, OSInt nChannels, Double magnify)
{
	if (sampleType == Math::FFTCalc::ST_I24)
	{
		FFTCalc_ApplyWindowI24(this->sampleTemp, samples, this->sampleWindow, this->sampleCount, nChannels * 3, magnify / 8388608.0);
	}
	else if (sampleType == Math::FFTCalc::ST_I16)
	{
		FFTCalc_ApplyWindowI16(this->sampleTemp, samples, this->sampleWindow, this->sampleCount, nChannels * 2, magnify / 32768.0);
	}

	FFTCalc_Forward(this->sampleTemp, this->sampleCount);

	FFTCalc_FFT2Freq(freq, this->sampleTemp, this->sampleCount);
	return true;
}