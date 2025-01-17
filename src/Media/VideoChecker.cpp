#include "Stdafx.h"
#include "MyMemory.h"
#include "Media/VideoChecker.h"

void __stdcall Media::VideoChecker::OnVideoFrame(UInt32 frameTime, UInt32 frameNum, UInt8 **imgData, UOSInt dataSize, Media::IVideoSource::FrameStruct frameStruct, void *userData, Media::FrameType frameType, Media::IVideoSource::FrameFlag flags, Media::YCOffset ycOfst)
{
	DecodeStatus *status = (DecodeStatus *)userData;

	if (frameType != Media::FT_DISCARD)
	{
		status->sampleCnt++;
		status->lastSampleTime = frameTime;
	}
}

void __stdcall Media::VideoChecker::OnVideoChange(Media::IVideoSource::FrameChange frChg, void *userData)
{
	DecodeStatus *status = (DecodeStatus *)userData;
	if (frChg == Media::IVideoSource::FC_ENDPLAY)
	{
		status->isEnd = true;
		status->evt->Set();
	}
}

void __stdcall Media::VideoChecker::OnAudioEnd(void *userData)
{
	DecodeStatus *status = (DecodeStatus *)userData;
	status->isEnd = true;
	status->evt->Set();
}

Media::VideoChecker::VideoChecker(Bool allowTimeSkip)
{
	this->allowTimeSkip = allowTimeSkip;
	NEW_CLASS(this->evt, Sync::Event(true, (const UTF8Char*)"Media.VideoChecker.evt"));
	NEW_CLASS(this->vdecoders, Media::Decoder::VideoDecoderFinder());
	NEW_CLASS(this->adecoders, Media::Decoder::AudioDecoderFinder());
}

Media::VideoChecker::~VideoChecker()
{
	DEL_CLASS(this->adecoders);
	DEL_CLASS(this->vdecoders);
	DEL_CLASS(this->evt);
}

void Media::VideoChecker::SetAllowTimeSkip(Bool allowTimeSkip)
{
	this->allowTimeSkip = allowTimeSkip;
}

Bool Media::VideoChecker::IsValid(Media::MediaFile *mediaFile)
{
	DecodeStatus *status;
	Media::IMediaSource *msrc;
	Data::ArrayList<DecodeStatus*> statusList;
	Bool isEnd;
	UOSInt i = 0;
	Int32 syncTime;
	Bool valid = true;
	UInt32 videoTime = 0;
	while (true)
	{
		msrc = mediaFile->GetStream(i, &syncTime);
		if (msrc == 0)
		{
			break;
		}
		status = MemAlloc(DecodeStatus, 1);
		status->sampleCnt = 0;
		status->lastSampleTime = 0;
		status->isEnd = false;
		status->adecoder = 0;
		status->vdecoder = 0;
		status->renderer = 0;
		status->evt = evt;
		if (msrc->GetMediaType() == Media::MEDIA_TYPE_VIDEO)
		{
			status->vdecoder = vdecoders->DecodeVideo((Media::IVideoSource*)msrc);
			if (status->vdecoder)
			{
				status->vdecoder->Init(OnVideoFrame, OnVideoChange, status);
			}
			else
			{
				status->isEnd = true;
			}
		}
		else if (msrc->GetMediaType() == Media::MEDIA_TYPE_AUDIO)
		{
			status->adecoder = adecoders->DecodeAudio((Media::IAudioSource*)msrc);
			if (status->adecoder)
			{
				NEW_CLASS(status->renderer, Media::NullRenderer());
				status->renderer->BindAudio(status->adecoder);
				status->renderer->SetEndNotify(OnAudioEnd, status);
				status->renderer->AudioInit(0);
			}
			else
			{
				status->isEnd = true;
			}
		}
		else
		{
			valid = false;
		}

		statusList.Add(status);
		i++;
	}
	if (valid)
	{
		i = statusList.GetCount();
		while (i-- > 0)
		{
			status = statusList.GetItem(i);
			if (status->vdecoder)
			{
				status->vdecoder->Start();
			}
			if (status->adecoder)
			{
				status->renderer->Start();
			}
		}
		isEnd = false;
		while (!isEnd)
		{
			isEnd = true;
			i = statusList.GetCount();
			while (i-- > 0)
			{
				status = statusList.GetItem(i);
				if (!status->isEnd)
				{
					isEnd = false;
				}
			}
			if (isEnd)
				break;
			evt->Wait(1000);
		}

		i = statusList.GetCount();
		while (i-- > 0)
		{
			status = statusList.GetItem(i);
			msrc = mediaFile->GetStream(i, &syncTime);
			if (msrc->GetMediaType() == Media::MEDIA_TYPE_VIDEO)
			{
				videoTime = status->lastSampleTime;
			}
		}

		i = statusList.GetCount();
		while (i-- > 0)
		{
			status = statusList.GetItem(i);
			if (status->renderer)
			{
				status->sampleCnt = status->renderer->GetSampleCnt();
			}
			SDEL_CLASS(status->renderer);
			SDEL_CLASS(status->vdecoder);
			SDEL_CLASS(status->adecoder);

			msrc = mediaFile->GetStream(i, &syncTime);
			if (msrc->GetMediaType() == Media::MEDIA_TYPE_VIDEO)
			{
				Media::IVideoSource *video = (Media::IVideoSource *)msrc;
				OSInt frameDiff = (OSInt)(video->GetFrameCount() - status->sampleCnt);
				if (frameDiff < -10 || frameDiff > 10)
				{
					valid = false;
				}
			}
			else if (msrc->GetMediaType() == Media::MEDIA_TYPE_AUDIO)
			{
				Media::IAudioSource *audio = (Media::IAudioSource *)msrc;
				Media::AudioFormat fmt;
				audio->GetFormat(&fmt);
				Int64 tdiff = audio->GetStreamTime() - (Int64)(status->sampleCnt * 1000 / fmt.frequency);
				Int64 tdiff2 = (Int64)videoTime - (Int64)(status->sampleCnt * 1000 / fmt.frequency);
				if (this->allowTimeSkip)
				{
					if ((tdiff < -1000 || tdiff > 1000) && (tdiff2 < -1000 || tdiff2 > 1000))
					{
						valid = false;
					}
				}
				else
				{
					if ((tdiff < -1000 || tdiff > 1000) || (tdiff2 < -1000 || tdiff2 > 1000))
					{
						valid = false;
					}
				}
			}
			else
			{
				valid = false;
			}
		}
		
	}
	i = statusList.GetCount();
	while (i-- > 0)
	{
		status = statusList.GetItem(i);
		MemFree(status);
	}
	return valid;
}
