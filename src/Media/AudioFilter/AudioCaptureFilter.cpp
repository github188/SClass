#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ByteTool.h"
#include "Media/AudioFilter/AudioCaptureFilter.h"
#include "Sync/Thread.h"
#define BUFFSIZE 1048576

UInt32 __stdcall Media::AudioFilter::AudioCaptureFilter::CaptureThread(void *userObj)
{
	Media::AudioFilter::AudioCaptureFilter *me = (Media::AudioFilter::AudioCaptureFilter*)userObj;
	UInt8 *tmpBuff;
	OSInt buffSize;
	me->running = true;
	while (!me->toStop)
	{
		me->writeMut->Lock();
		while (me->readBuffSize > 0)
		{
			me->readMut->Lock();
			tmpBuff = me->writeBuff;
			me->writeBuff = me->readBuff;
			me->readBuff = tmpBuff;
			buffSize = me->readBuffSize;
			me->readBuffSize = 0;
			me->readMut->Unlock();

			if (me->waveStm)
			{
				me->waveStm->Write(me->writeBuff, buffSize);
				me->dataSize += buffSize;
				me->fileSize += buffSize;
			}
			me->readMut->Unlock();
		}
		me->writeMut->Unlock();
		
		me->evt->Wait(1000);
	}
	me->running = false;
	return 0;
}

Media::AudioFilter::AudioCaptureFilter::AudioCaptureFilter(Media::IAudioSource *sourceAudio) : Media::IAudioFilter(sourceAudio)
{
	NEW_CLASS(this->readMut, Sync::Mutex());
	NEW_CLASS(this->writeMut, Sync::Mutex());
	NEW_CLASS(this->evt, Sync::Event(true, (const UTF8Char*)"Media.AudioFilter.AudioCaptureFilter.evt"));
	this->sourceAudio = sourceAudio;
	this->waveStm = 0;
	this->readBuff = MemAlloc(UInt8, BUFFSIZE);
	this->writeBuff = MemAlloc(UInt8, BUFFSIZE);
	this->readBuffSize = 0;
	this->writing = false;
	this->running = false;
	this->toStop = false;
	Sync::Thread::Create(CaptureThread, this);
}

Media::AudioFilter::AudioCaptureFilter::~AudioCaptureFilter()
{
	StopCapture();
	this->toStop = true;
	this->evt->Set();
	while (this->running)
	{
		Sync::Thread::Sleep(10);
	}
	DEL_CLASS(this->writeMut);
	DEL_CLASS(this->readMut);
	DEL_CLASS(this->evt);
	MemFree(this->readBuff);
	MemFree(this->writeBuff);
}

Int32 Media::AudioFilter::AudioCaptureFilter::SeekToTime(Int32 time)
{
	if (this->sourceAudio)
	{
		return this->sourceAudio->SeekToTime(time);
	}
	return 0;
}

UOSInt Media::AudioFilter::AudioCaptureFilter::ReadBlock(UInt8 *buff, UOSInt blkSize)
{
	if (this->sourceAudio == 0)
		return 0;

	UOSInt readSize = this->sourceAudio->ReadBlock(buff, blkSize);
	this->readMut->Lock();
	if (this->writing)
	{
		if (this->readBuffSize >= BUFFSIZE)
		{
		}
		else if (this->readBuffSize + blkSize > BUFFSIZE)
		{
			MemCopyNO(&this->readBuff[this->readBuffSize], buff, BUFFSIZE - this->readBuffSize);
			this->readBuffSize = BUFFSIZE;
			this->evt->Set();
		}
		else
		{
			MemCopyNO(&this->readBuff[this->readBuffSize], buff, blkSize);
			this->readBuffSize += blkSize;
			this->evt->Set();
		}
	}
	this->readMut->Unlock();
	return readSize;
}

Bool Media::AudioFilter::AudioCaptureFilter::StartCapture(const UTF8Char *fileName)
{
	if (this->sourceAudio == 0)
		return false;
	this->StopCapture();
	Media::AudioFormat format;
	this->sourceAudio->GetFormat(&format);
	UInt8 buff[74];
	*(Int32*)&buff[0] = *(Int32*)"RIFF";
	*(UInt32*)&buff[4] = 0;
	*(Int32*)&buff[8] = *(Int32*)"WAVE";
	*(Int32*)&buff[12] = *(Int32*)"JUNK";
	*(UInt32*)&buff[16] = 28;
	MemClear(&buff[20], 28);
	*(Int32*)&buff[48] = *(Int32*)"fmt ";
	*(Int32*)&buff[52] = 18 + format.extraSize;
	*(UInt16*)&buff[56] = format.formatId;
	*(UInt16*)&buff[58] = format.nChannels;
	*(Int32*)&buff[60] = format.frequency;
	*(Int32*)&buff[64] = format.bitRate >> 3;
	*(UInt16*)&buff[68] = format.align;
	*(UInt16*)&buff[70] = format.bitpersample;
	*(UInt16*)&buff[72] = format.extraSize;
	this->writeMut->Lock();
	NEW_CLASS(this->waveStm, IO::FileStream(fileName, IO::FileStream::FILE_MODE_CREATE, IO::FileStream::FILE_SHARE_DENY_NONE, IO::FileStream::BT_NORMAL));
	this->waveStm->Write(buff, 74);
	if (format.extraSize > 0)
	{
		this->dataOfst = 78 + format.extraSize;
		this->waveStm->Write(format.extra, format.extraSize);
	}
	else
	{
		this->dataOfst = 78;
	}
	*(Int32*)&buff[0] = *(Int32*)"data";
	*(Int32*)&buff[4] = 0;
	this->waveStm->Write(buff, 8);
	this->dataSize = 0;
	this->fileSize = this->dataOfst - 4;
	this->writeMut->Unlock();
	this->writing = true;
	return true;
}

void Media::AudioFilter::AudioCaptureFilter::StopCapture()
{
	this->writing = false;
	this->writeMut->Lock();
	if (this->waveStm)
	{
		if (this->fileSize >= 0x100000000LL)
		{
			UInt8 buff[48];
			*(Int32*)&buff[0] = *(Int32*)"RF64";
			*(Int32*)&buff[4] = -1;
			*(Int32*)&buff[12] = *(Int32*)"ds64";
			*(Int64*)&buff[20] = this->fileSize;
			*(Int64*)&buff[28] = this->dataSize;
			*(Int64*)&buff[36] = 0;
			*(Int32*)&buff[44] = 0;
			this->waveStm->Seek(IO::SeekableStream::ST_BEGIN, 0);
			this->waveStm->Write(buff, 48);

			this->waveStm->Seek(IO::SeekableStream::ST_BEGIN, dataOfst);
			this->waveStm->Write(&buff[4], 4);
		}
		else
		{
			this->waveStm->Seek(IO::SeekableStream::ST_BEGIN, 4);
			this->waveStm->Write((const UInt8*)&this->fileSize, 4);
			this->waveStm->Seek(IO::SeekableStream::ST_BEGIN, dataOfst);
			this->waveStm->Write((const UInt8*)&this->dataSize, 4);
		}
		DEL_CLASS(this->waveStm);
		this->waveStm = 0;
	}
	this->writeMut->Unlock();
}