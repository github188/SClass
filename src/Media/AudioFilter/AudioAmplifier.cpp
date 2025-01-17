#include "Stdafx.h"
#include "MyMemory.h"
#include "Math/Math.h"
#include "Media/AudioUtil.h"
#include "Media/AudioFilter/AudioAmplifier.h"

Media::AudioFilter::AudioAmplifier::AudioAmplifier(Media::IAudioSource *sourceAudio) : Media::IAudioFilter(sourceAudio)
{
	Media::AudioFormat fmt;
	this->sourceAudio = 0;
	this->level = 1.0;
	sourceAudio->GetFormat(&fmt);
	if (fmt.formatId != 0x1)
		return;
	if (fmt.bitpersample != 16 && fmt.bitpersample != 8)
		return;
	this->bitCount = fmt.bitpersample;
	this->sourceAudio = sourceAudio;
}

Media::AudioFilter::AudioAmplifier::~AudioAmplifier()
{
}

void Media::AudioFilter::AudioAmplifier::GetFormat(AudioFormat *format)
{
	if (this->sourceAudio)
	{
		Media::AudioFormat fmt;
		this->sourceAudio->GetFormat(&fmt);
		format->formatId = 1;
		format->bitpersample = (UInt16)this->bitCount;
		format->frequency = fmt.frequency;
		format->nChannels = fmt.nChannels;
		format->bitRate = fmt.frequency * fmt.nChannels * this->bitCount;
		format->align = fmt.nChannels * this->bitCount >> 3;
		format->other = 0;
		format->intType = Media::AudioFormat::IT_NORMAL;
		format->extraSize = 0;
		format->extra = 0;
	}
	else
	{
		format->Clear();
	}
}

UInt32 Media::AudioFilter::AudioAmplifier::SeekToTime(UInt32 time)
{
	if (this->sourceAudio)
	{
		return this->sourceAudio->SeekToTime(time);
	}
	return 0;
}

UOSInt Media::AudioFilter::AudioAmplifier::ReadBlock(UInt8 *buff, UOSInt blkSize)
{
	if (this->sourceAudio == 0)
		return 0;

	UOSInt readSize = this->sourceAudio->ReadBlock(buff, blkSize);
	Double thisVol = this->level;
	if (thisVol != 1.0)
	{
		Int32 iVol = Math::Double2Int32(thisVol * 65536.0);
		if (this->bitCount == 16)
		{
			AudioUtil_Amplify16(buff, readSize, iVol);
		}
		else if (this->bitCount == 8)
		{
			AudioUtil_Amplify8(buff, readSize, iVol);
		}
	}
	return readSize;
}

void Media::AudioFilter::AudioAmplifier::SetLevel(Double level)
{
	if (level >= 0)
	{
		this->level = level;
	}
}
