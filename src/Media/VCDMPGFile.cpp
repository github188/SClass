#include "Stdafx.h"
#include "Data/ByteTool.h"
#include "IO/ISectorData.h"
#include "Media/M2VStreamSource.h"
#include "Media/MPAStreamSource.h"
#include "Media/MPEGVideoParser.h"
#include "Media/VCDMPGFile.h"
#include "Sync/Thread.h"
#define BUFFSIZE 1048576

UInt32 __stdcall Media::VCDMPGFile::PlayThread(void *userData)
{
	Media::VCDMPGFile *me = (Media::VCDMPGFile*)userData;
	Bool succ;
	UOSInt currOfst;
	UOSInt i;
	UOSInt j;
	UInt32 thisSize;
	Int64 scr;
//	Int32 muxRate;
	Int64 initScr = -1;
	Int64 last_scr_base = -1;
	Media::IMediaStream *mstm;
	Int32 lastFrameTime = 0;

	me->playStarted = true;
	me->playing = 2;
	me->vstm->ClearFrameBuff();
	me->vstm->SetStreamTime(me->startTime);
	Bool firstAudio = true;
	Bool firstVideo = false;
	while (!me->playToStop)
	{
		succ = me->data->ReadSector(me->readOfst, me->readBuff);
		if (!succ)
		{
			me->vstm->EndFrameStream();
			break;
		}
		/////////////////////////////////////////////////

	
		/*		if (buffSize == 0)
		{
			me->stm->EndFrameStream();
			break;
		}
		me->stm->WriteFrameStream(me->readBuff, buffSize);*/

		currOfst = 24;
		while (currOfst < 2348)
		{
			if (me->readBuff[currOfst] != 0 || me->readBuff[currOfst + 1] != 0 || me->readBuff[currOfst + 2] != 1)
			{
				break;
			}

			if (me->readBuff[currOfst + 3] == 0xB9) //End Of File
				break;
			if (me->readBuff[currOfst + 3] == 0xba) //Pack Start
			{
				scr = (((Int64)me->readBuff[currOfst + 4] & 0xe) << 29) | (me->readBuff[currOfst + 5] << 22) | ((me->readBuff[currOfst + 6] & 0xfe) << 14) | (me->readBuff[currOfst + 7] << 7) | (me->readBuff[currOfst + 8] >> 1);
//				muxRate = ((me->readBuff[currOfst + 9] & 0x7f) << 15) | (me->readBuff[currOfst + 10] << 7) | (me->readBuff[currOfst + 11] >> 1);
				currOfst += 12;

				if (initScr == -1)
				{
					initScr = scr;
				}
				if (scr < last_scr_base || scr > last_scr_base + 9000)
				{
					firstAudio = true;
					firstVideo = false;
				}
				last_scr_base = scr;
			}
			else
			{
				i = ReadMUInt16(&me->readBuff[currOfst + 4]);
				if (me->readBuff[currOfst + 3] == 0xbb) // System Header
				{
				}
				else if (me->readBuff[currOfst + 3] == 0xbd) //Private stream 1
				{
				}
				else if ((me->readBuff[currOfst + 3] & 0xe0) == 0xc0) //Audio stream
				{
					thisSize = ReadMUInt16(&me->readBuff[currOfst + 4]);
					if (2348 - currOfst < 6 + thisSize)
					{
						break;
					}

					j = 6;
					while (me->readBuff[currOfst + j] & 0x80)
					{
						j++;
					}
					if ((me->readBuff[currOfst + j] & 0xc0) == 0x40)
					{
						j += 2;
					}

					Int64 pts = 0;
					Int64 dts = 0;
					if ((me->readBuff[currOfst + j] & 0xf0) == 0x20)
					{
						pts = (((Int64)(me->readBuff[currOfst + j] & 0xe)) << 29) | (me->readBuff[currOfst + j + 1] << 22) | ((me->readBuff[currOfst + j + 2] & 0xfe) << 14) | (me->readBuff[currOfst + j + 3] << 7) | (me->readBuff[currOfst + j + 4] >> 1);
						dts = pts;
						j += 5;
					}
					else if ((me->readBuff[i + j] & 0xf0) == 0x30)
					{
						pts = (((Int64)(me->readBuff[currOfst + j] & 0xe)) << 29) | (me->readBuff[currOfst + j + 1] << 22) | ((me->readBuff[currOfst + j + 2] & 0xfe) << 14) | (me->readBuff[currOfst + j + 3] << 7) | (me->readBuff[currOfst + j + 4] >> 1);
						dts = (((Int64)(me->readBuff[currOfst + j + 5] & 0xe)) << 29) | (me->readBuff[currOfst + j + 6] << 22) | ((me->readBuff[currOfst + j + 7] & 0xfe) << 14) | (me->readBuff[currOfst + j + 8] << 7) | (me->readBuff[currOfst + j + 9] >> 1);
						j += 10;
					}
					else if (me->readBuff[j] == 0xf)
					{
						j++;
					}
					else
					{
					}
					UInt8 stmType = me->readBuff[currOfst + 3] & 0x1f;
					mstm = me->dataStms->Get(stmType);
					if (mstm)
					{
						if (firstAudio && dts != 0)
						{
							initScr = dts - 90 * mstm->GetFrameStreamTime();
							firstVideo = true;
							firstAudio = false;
						}
						mstm->WriteFrameStream(&me->readBuff[currOfst + j], thisSize - j + 6);
					}
				}
				else if ((me->readBuff[currOfst + 3] & 0xf0) == 0xe0) //Video stream
				{
					Int64 pts = 0;
					Int64 dts = 0;
					j = currOfst + 6;
					while (me->readBuff[j] & 0x80)
					{
						j++;
					}
					if ((me->readBuff[j] & 0xc0) == 0x40)
					{
						j += 2;
					}

					if ((me->readBuff[j] & 0xf0) == 0x20)
					{
						pts = (((Int64)me->readBuff[j] & 0xe) << 29) | (me->readBuff[j + 1] << 22) | ((me->readBuff[j + 2] & 0xfe) << 14) | (me->readBuff[j + 3] << 7) | (me->readBuff[j + 4] >> 1);;
						dts = pts;

						j += 5;
					}
					else if ((me->readBuff[j] & 0xf0) == 0x30)
					{
						pts = (((Int64)me->readBuff[j] & 0xe) << 29) | (me->readBuff[j + 1] << 22) | ((me->readBuff[j + 2] & 0xfe) << 14) | (me->readBuff[j + 3] << 7) | (me->readBuff[j + 4] >> 1);;
						dts = (((Int64)me->readBuff[j + 5] & 0xe) << 29) | (me->readBuff[j + 6] << 22) | ((me->readBuff[j + 7] & 0xfe) << 14) | (me->readBuff[j + 8] << 7) | (me->readBuff[j + 9] >> 1);;

						j += 10;
					}
					else if (me->readBuff[j] == 0xf)
					{
						j++;
					}
					else
					{
//						valid = false;
						break;
					}

					if (dts != 0)
					{
						Int32 frameTime = (Int32)((dts - initScr) / 90);
						if (frameTime < 0)
							frameTime = 0;
						if (firstVideo || frameTime > lastFrameTime)
						{
							me->vstm->SetStreamTime((UInt32)frameTime);
							firstVideo = false;
							lastFrameTime = frameTime;
						}
					}

					me->vstm->WriteFrameStream(&me->readBuff[j], 2348 - j);
				}
				else if (me->readBuff[currOfst + 3] == 0xbc) // program_stream_map
				{
				}
				else if (me->readBuff[currOfst + 3] == 0xbe) // padding stream
				{
				}
				else if (me->readBuff[currOfst + 3] == 0xbf) // private stream 2
				{
				}
				currOfst += 6 + i;
			}
		}
		me->readOfst++;
	}

	me->playing = 0;
	return 0;
}

Bool Media::VCDMPGFile::StartPlay()
{
	if (this->playing != 0)
		return true;
	this->playing = 1;
	this->playStarted = false;
	this->playToStop = false;
	Sync::Thread::Create(PlayThread, this);
	return true;
}

Bool Media::VCDMPGFile::StopPlay()
{
	this->playToStop = true;
	while (this->playing != 0)
	{
		Sync::Thread::Sleep(10);
	}
	this->readOfst = 0;
	this->startTime = 0;
	return true;
}

Media::VCDMPGFile::VCDMPGFile(IO::ISectorData *data, UInt64 startSector, UInt64 endSector) : Media::MediaFile(data->GetSourceNameObj())
{
	this->data = data->GetPartialData(startSector, endSector - startSector);
	this->readOfst = 0;
	this->readBuff = MemAlloc(UInt8, 2352);
	this->playing = 0;
	this->playToStop = false;
	this->startTime = 0;
	this->fleng = endSector - startSector;
	this->vstm = 0;

	NEW_CLASS(this->dataStms, Data::Int32Map<Media::IMediaStream*>());
	NEW_CLASS(this->audStms, Data::ArrayList<Media::IAudioSource*>());

	Bool succ;
	UOSInt currSector = 0;
	UOSInt currOfst;
	UOSInt i;
	UOSInt j;
//	Int64 scr;
//	Int32 muxRate;
	while (true)
	{
		if (currSector >= 75 && this->vstm && this->dataStms->GetCount() > 0)
		{
			break;
		}
		succ = this->data->ReadSector(currSector, this->readBuff);
		if (!succ)
		{
			break;;
		}
		
		currOfst = 24;
		while (currOfst < 2348)
		{
			if (this->readBuff[currOfst] != 0 || this->readBuff[currOfst + 1] != 0 || this->readBuff[currOfst + 2] != 1)
			{
				break;
			}

			if (this->readBuff[currOfst + 3] == 0xB9) //End Of File
				break;
			if (this->readBuff[currOfst + 3] == 0xba) //Pack Start
			{
//				scr = (((Int64)this->readBuff[currOfst + 4] & 0xe) << 29) | (this->readBuff[currOfst + 5] << 22) | ((this->readBuff[currOfst + 6] & 0xfe) << 14) | (this->readBuff[currOfst + 7] << 7) | (this->readBuff[currOfst + 8] >> 1);
//				muxRate = ((this->readBuff[currOfst + 9] & 0x7f) << 15) | (this->readBuff[currOfst + 10] << 7) | (this->readBuff[currOfst + 11] >> 1);
				currOfst += 12;
			}
			else
			{
				i = ReadMUInt16(&this->readBuff[currOfst + 4]);
				if (this->readBuff[currOfst + 3] == 0xbb) // System Header
				{
				}
				else if (this->readBuff[currOfst + 3] == 0xbd) //Private stream 1
				{
				}
				else if ((this->readBuff[currOfst + 3] & 0xe0) == 0xc0) //Audio stream
				{
//					Int64 pts = 0;
//					Int64 dts = 0;
					j = currOfst + 6;
					while (this->readBuff[j] & 0x80)
					{
						j++;
					}
					if ((this->readBuff[j] & 0xc0) == 0x40)
					{
						j += 2;
					}

					if ((this->readBuff[j] & 0xf0) == 0x20)
					{
//						pts = (((Int64)this->readBuff[j] & 0xe) << 29) | (this->readBuff[j + 1] << 22) | ((this->readBuff[j + 2] & 0xfe) << 14) | (this->readBuff[j + 3] << 7) | (this->readBuff[j + 4] >> 1);;
						j += 5;
					}
					else if ((this->readBuff[j] & 0xf0) == 0x30)
					{
//						pts = (((Int64)this->readBuff[j] & 0xe) << 29) | (this->readBuff[j + 1] << 22) | ((this->readBuff[j + 2] & 0xfe) << 14) | (this->readBuff[j + 3] << 7) | (this->readBuff[j + 4] >> 1);;
//						dts = (((Int64)this->readBuff[j + 5] & 0xe) << 29) | (this->readBuff[j + 6] << 22) | ((this->readBuff[j + 7] & 0xfe) << 14) | (this->readBuff[j + 8] << 7) | (this->readBuff[j + 9] >> 1);;
						j += 10;
					}
					else if (this->readBuff[j] == 0xf)
					{
						j++;
					}
					else
					{
//						valid = false;
						break;
					}

					Int32 stmId = this->readBuff[currOfst + 3] & 0x1f;
					Media::IMediaStream *mstm = this->dataStms->Get(stmId);
					if (mstm == 0)
					{
//						Int32 v = ReadMInt32(&this->readBuff[j]);
						if (this->readBuff[j] == 0xff && ((this->readBuff[j + 1] & 0xfe) == 0xfc || (this->readBuff[j + 1] & 0xfe) == 0xfa))
						{
							Media::MPAStreamSource *mstm = (Media::MPAStreamSource*)this->dataStms->Get(stmId);
							if (mstm == 0)
							{
								NEW_CLASS(mstm, Media::MPAStreamSource(this));
								this->dataStms->Put(stmId, mstm);
								this->audStms->Add(mstm);
							}
							if (!mstm->IsReady())
							{
								mstm->ParseHeader(&this->readBuff[j], i - j + 6);
							}
						}
					}
				}
				else if ((this->readBuff[currOfst + 3] & 0xf0) == 0xe0) //Video stream
				{
//					Int64 pts = 0;
//					Int64 dts = 0;
					j = currOfst + 6;
					while (this->readBuff[j] & 0x80)
					{
						j++;
					}
					if ((this->readBuff[j] & 0xc0) == 0x40)
					{
						j += 2;
					}

					if ((this->readBuff[j] & 0xf0) == 0x20)
					{
//						pts = (((Int64)this->readBuff[j] & 0xe) << 29) | (this->readBuff[j + 1] << 22) | ((this->readBuff[j + 2] & 0xfe) << 14) | (this->readBuff[j + 3] << 7) | (this->readBuff[j + 4] >> 1);;
						j += 5;
					}
					else if ((this->readBuff[j] & 0xf0) == 0x30)
					{
//						pts = (((Int64)this->readBuff[j] & 0xe) << 29) | (this->readBuff[j + 1] << 22) | ((this->readBuff[j + 2] & 0xfe) << 14) | (this->readBuff[j + 3] << 7) | (this->readBuff[j + 4] >> 1);;
//						dts = (((Int64)this->readBuff[j + 5] & 0xe) << 29) | (this->readBuff[j + 6] << 22) | ((this->readBuff[j + 7] & 0xfe) << 14) | (this->readBuff[j + 8] << 7) | (this->readBuff[j + 9] >> 1);;
						j += 10;
					}
					else if (this->readBuff[j] == 0xf)
					{
						j++;
					}
					else
					{
//						valid = false;
						break;
					}

					Bool isFrame = false;
					if (this->readBuff[j] == 0 && this->readBuff[j + 1] == 0 && this->readBuff[j + 2] == 1 && this->readBuff[j + 3] == 0xb3)
					{
						isFrame = true;
					}
					Media::FrameInfo frameInfo;
					UInt32 frameRateNorm;
					UInt32 frameRateDenorm;
					if (isFrame && vstm == 0 && Media::MPEGVideoParser::GetFrameInfo(&this->readBuff[j], 2348 - j, &frameInfo, &frameRateNorm, &frameRateDenorm, 0, true))
					{
						NEW_CLASS(vstm, Media::M2VStreamSource(this));
						this->vstm->DetectStreamInfo(&this->readBuff[j], 2348 - j);
					}

					break;
				}
				else if (this->readBuff[currOfst + 3] == 0xbc) // program_stream_map
				{
				}
				else if (this->readBuff[currOfst + 3] == 0xbe) // padding stream
				{
				}
				else if (this->readBuff[currOfst + 3] == 0xbf) // private stream 2
				{
				}
				currOfst += 6 + i;
			}

		}
		currSector++;
	}
}

Media::VCDMPGFile::~VCDMPGFile()
{
	if (this->playing)
	{
		this->StopPlay();
	}
	Data::ArrayList<Media::IMediaStream*> *dataList = this->dataStms->GetValues();
	Media::IMediaStream *stm;
	UOSInt i;
	i = dataList->GetCount();
	while (i-- > 0)
	{
		stm = dataList->GetItem(i);
		DEL_CLASS(stm);
	}
	DEL_CLASS(this->dataStms);
	DEL_CLASS(this->audStms);
	SDEL_CLASS(this->vstm);
	DEL_CLASS(this->data);
	MemFree(this->readBuff);
}

UOSInt Media::VCDMPGFile::AddSource(Media::IMediaSource *src, Int32 syncTime)
{
	return (UOSInt)-1;
}

Media::IMediaSource *Media::VCDMPGFile::GetStream(UOSInt index, Int32 *syncTime)
{
	if (index > this->audStms->GetCount())
		return 0;
	if (syncTime)
		*syncTime = 0;
	if (index == 0)
		return this->vstm;
	else
		return this->audStms->GetItem(index - 1);
}

void Media::VCDMPGFile::KeepStream(UOSInt index, Bool toKeep)
{
}

UTF8Char *Media::VCDMPGFile::GetMediaName(UTF8Char *buff)
{
	return this->GetSourceName(buff);
}

Int32 Media::VCDMPGFile::GetStreamTime()
{
	return (Int32)(this->fleng * 1000 / 75);
}

Bool Media::VCDMPGFile::StartAudio()
{
	return StartPlay();
}

Bool Media::VCDMPGFile::StopAudio()
{
	return StopPlay();
}

Bool Media::VCDMPGFile::StartVideo()
{
	return StartPlay();
}

Bool Media::VCDMPGFile::StopVideo()
{
	return StopPlay();
}

Bool Media::VCDMPGFile::IsRunning()
{
	return this->playing != 0;
}

UInt32 Media::VCDMPGFile::SeekToTime(UInt32 mediaTime)
{
	if (this->playing)
		return 0;
	this->readOfst = mediaTime * 75 / 1000;
	this->startTime = mediaTime;
	return mediaTime;
}

Bool Media::VCDMPGFile::IsRealTimeSrc()
{
	return false;
}

Bool Media::VCDMPGFile::CanSeek()
{
	return true;
}

UOSInt Media::VCDMPGFile::GetDataSeekCount()
{
	return this->data->GetSeekCount();
}
