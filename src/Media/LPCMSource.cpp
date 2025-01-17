#include "Stdafx.h"
#include "MyMemory.h"
#include "Sync/Event.h"
#include "Text/MyString.h"
#include "IO/IStreamData.h"
#include "Media/IMediaSource.h"
#include "Media/IAudioSource.h"
#include "Media/LPCMSource.h"

Media::LPCMSource::LPCMSource(const UTF8Char *name)
{
	this->format.Clear();
	this->data = 0;

	if (name)
	{
		this->name = Text::StrCopyNew(name);
	}
	else
	{
		this->name = 0;
	}

	this->readEvt = 0;
	this->readOfst = 0;
}

void Media::LPCMSource::SetData(IO::IStreamData *fd, UInt64 ofst, UInt64 length, Media::AudioFormat *format)
{
	if (this->data)
	{
		DEL_CLASS(this->data);
	}
	this->format.FromAudioFormat(format);

	this->data = fd->GetPartialData(ofst, length);
}

Media::LPCMSource::LPCMSource(IO::IStreamData *fd, UInt64 ofst, UInt64 length, Media::AudioFormat *format, const UTF8Char *name)
{
	this->format.FromAudioFormat(format);

	this->data = fd->GetPartialData(ofst, length);

	if (name)
	{
		this->name = Text::StrCopyNew(name);
	}
	else
	{
		this->name = 0;
	}

	this->readEvt = 0;
	this->readOfst = 0;
}

Media::LPCMSource::~LPCMSource()
{
	DEL_CLASS(this->data);
	if (this->name)
	{
		Text::StrDelNew(this->name);
	}
}

UTF8Char *Media::LPCMSource::GetSourceName(UTF8Char *buff)
{
	if (this->name == 0)
		return 0;
	return Text::StrConcat(buff, this->name);
}

Bool Media::LPCMSource::CanSeek()
{
	return true;
}

Int32 Media::LPCMSource::GetStreamTime()
{
	return (Int32)(this->data->GetDataSize() * 1000 / this->format.frequency / (UInt32)(this->format.bitpersample >> 3) / this->format.nChannels);
}

UInt32 Media::LPCMSource::SeekToTime(UInt32 time)
{
	UInt32 blk = (this->format.nChannels * (UInt32)this->format.bitpersample >> 3);
	this->readOfst = time * (UInt64)this->format.frequency / 1000 * blk;
	return (UInt32)(this->readOfst * 1000 / this->format.frequency / blk);
}

Bool Media::LPCMSource::TrimStream(UInt32 trimTimeStart, UInt32 trimTimeEnd, Int32 *syncTime)
{
	UInt32 blk = (this->format.nChannels * (UInt32)this->format.bitpersample >> 3);
	if (trimTimeEnd == (UInt32)-1)
	{
		if (trimTimeStart >= 0)
		{
			UInt64 ofst = trimTimeStart * (UInt64)this->format.frequency / 1000 * blk;
			IO::IStreamData *newData = this->data->GetPartialData(ofst, this->data->GetDataSize() - ofst);
			DEL_CLASS(this->data);
			this->data = newData;
			if (syncTime)
			{
				*syncTime = 0;
			}
		}
		else
		{
			if (syncTime)
			{
				*syncTime = (Int32)trimTimeStart;
			}
		}
	}
	else
	{
		UInt64 ofst1 = trimTimeStart * (UInt64)this->format.frequency / 1000 * blk;
		UInt64 ofst2 = trimTimeEnd * (UInt64)this->format.frequency / 1000 * blk;
		UInt64 dataSize = this->data->GetDataSize();
		if (ofst2 > dataSize)
			ofst2 = dataSize;
		if (trimTimeStart >= 0)
		{
			IO::IStreamData *newData = this->data->GetPartialData(ofst1, ofst2 - ofst1);
			DEL_CLASS(this->data);
			this->data = newData;
			if (syncTime)
			{
				*syncTime = 0;
			}
		}
		else
		{
			IO::IStreamData *newData = this->data->GetPartialData(0, ofst2);
			DEL_CLASS(this->data);
			this->data = newData;
			if (syncTime)
			{
				*syncTime = (Int32)trimTimeStart;
			}
		}
	}
	return true;
}

void Media::LPCMSource::GetFormat(AudioFormat *format)
{
	format->FromAudioFormat(&this->format);
}

Bool Media::LPCMSource::Start(Sync::Event *evt, UOSInt blkSize)
{
	this->readEvt = evt;
	if (this->readEvt)
		this->readEvt->Set();
	return true;
}

void Media::LPCMSource::Stop()
{
	this->readEvt = 0;
	this->readOfst = 0;
}

UOSInt Media::LPCMSource::ReadBlock(UInt8 *buff, UOSInt blkSize)
{
	UOSInt readSize = 0;
#ifndef HAS_ASM32
	UOSInt i;
#endif
	if (this->format.intType == Media::AudioFormat::IT_BIGENDIAN)
	{
		UInt32 blk = (this->format.nChannels * (UInt32)this->format.bitpersample >> 3);
		readSize = blkSize / blk;
		readSize = this->data->GetRealData(this->readOfst, readSize * blk, buff);
		if (this->format.bitpersample == 16)
		{
#ifdef HAS_ASM32
			_asm
			{
				mov esi,buff
				mov ecx,readSize
				shr ecx,1
				jz rblk1_16exit
rblk1_16lop:
				lodsw
				mov byte ptr [esi - 2],ah
				mov byte ptr [esi - 1],al
				dec ecx
				jnz rblk1_16lop
rblk1_16exit:
			}
#else
			if ((i = readSize >> 1) != 0)
			{
				UInt8 tmp;
				while (i-- > 0)
				{
					tmp = buff[0];
					buff[0] = buff[1];
					buff[1] = tmp;
					buff += 2;
				}
			}
#endif
		}
		else if (this->format.bitpersample == 24)
		{
#ifdef HAS_ASM32
			_asm
			{
				mov esi,buff
				mov eax,readSize
				mov edx,0
				mov ecx,3
				div ecx
				mov ecx,eax
				cmp ecx,0
				jz rblk1_24exit
rblk1_24lop:
				mov al, byte ptr [esi]
				mov dl, byte ptr [esi+2]
				mov byte ptr [esi],dl
				mov byte ptr [esi+2],al
				add esi,3
				dec ecx
				jnz rblk1_24lop
rblk1_24exit:
			}
#else
			if ((i = readSize / 3) != 0)
			{
				UInt8 tmp;
				while (i-- > 0)
				{
					tmp = buff[0];
					buff[0] = buff[2];
					buff[2] = tmp;
					buff += 3;
				}
			}
#endif
		}
	}
	else if (this->format.intType == Media::AudioFormat::IT_BIGENDIAN16)
	{
		if (this->format.bitpersample == 16)
		{
			UInt32 blk = (this->format.nChannels * (UInt32)this->format.bitpersample >> 3);
			readSize = blkSize / blk;
			readSize = this->data->GetRealData(this->readOfst, readSize * blk, buff);
#ifdef HAS_ASM32
			_asm
			{
				mov esi,buff
				mov ecx,readSize
				shr ecx,1
				jz rblk2_16exit
rblk2_16lop:
				lodsw
				mov byte ptr [esi - 2],ah
				mov byte ptr [esi - 1],al
				dec ecx
				jnz rblk2_16lop
rblk2_16exit:
			}
#else
			if ((i = readSize >> 1) != 0)
			{
				UInt8 tmp;
				while (i-- > 0)
				{
					tmp = buff[0];
					buff[0] = buff[1];
					buff[1] = tmp;
					buff += 2;
				}
			}
#endif
		}
		else if (this->format.bitpersample == 24)
		{
			if (this->format.nChannels == 2)
			{
				UInt32 blk = 12;//(this->format.nChannels * this->format.bitpersample >> 3);
				readSize = blkSize / blk;
				readSize = this->data->GetRealData(this->readOfst, readSize * blk, buff);
#ifdef HAS_ASM32
				_asm
				{
					mov esi,buff
					mov eax,readSize
					mov edx,0
					mov ecx,12
					div ecx
					mov ecx,eax
					cmp ecx,0
					jz rblk2_24_2exit
rblk2_24_2lop:
					mov ax,word ptr [esi]
					mov dx,word ptr [esi+2]
					mov byte ptr [esi+1],ah
					mov byte ptr [esi+2],al
					mov ax,word ptr [esi+4]
					mov byte ptr [esi+4],dh
					mov byte ptr [esi+5],dl
					mov dx,word ptr [esi+8]
					mov byte ptr [esi],dl
					mov byte ptr [esi+3],dh
					mov dx,word ptr [esi+6]
					mov byte ptr [esi+7],ah
					mov byte ptr [esi+8],al
					mov ax,word ptr [esi+10]
					mov byte ptr [esi+10],dh
					mov byte ptr [esi+11],dl
					mov byte ptr [esi+6],al
					mov byte ptr [esi+9],ah
					add esi,12
					dec ecx
					jnz rblk2_24_2lop
rblk2_24_2exit:
				}
#else
				if ((i = readSize / 12) != 0)
				{
					UInt8 tmpVal[4];
					while (i-- > 0)
					{
						*(Int32*)tmpVal = *(Int32*)buff;
						buff[1] = tmpVal[1];
						buff[2] = tmpVal[0];
						*(Int16*)tmpVal = *(Int16*)&buff[4];
						buff[4] = tmpVal[3];
						buff[5] = tmpVal[2];
						*(Int16*)&tmpVal[2] = *(Int16*)&buff[8];
						buff[0] = tmpVal[2];
						buff[3] = tmpVal[3];
						*(Int16*)&tmpVal[2] = *(Int16*)&buff[6];
						buff[7] = tmpVal[1];
						buff[8] = tmpVal[0];
						*(Int16*)&tmpVal[0] = *(Int16*)&buff[10];
						buff[10] = tmpVal[3];
						buff[11] = tmpVal[2];
						buff[6] = tmpVal[0];
						buff[9] = tmpVal[1];
						buff += 12;
					}
				}
#endif
			}
		}
	}
	else if (this->format.other)
	{
		UInt32 blk = this->format.nChannels * (UInt32)(this->format.bitpersample >> 3) * this->format.other;
		readSize = blkSize / blk;
		if (readSize > 0)
		{
			UInt32 ofstPC = (UInt32)(this->format.bitpersample >> 3) * this->format.other;
			UInt8 *tmpBuff = MemAlloc(UInt8, blkSize * blk);
			UInt8 *tmpPtr;
			UInt8 *tmpPtr2;
			UOSInt cnt;
			UOSInt cnt2;
			readSize = this->data->GetRealData(this->readOfst, readSize * blk, tmpBuff);
			UOSInt sizeLeft = readSize;
			tmpPtr = tmpBuff;
			while (sizeLeft >= blk)
			{
				cnt = this->format.other;
				while (cnt-- > 0)
				{
					tmpPtr2 = tmpPtr;
					cnt2 = this->format.nChannels;
					while (cnt2-- > 0)
					{
						*(Int16*)buff = *(Int16*)tmpPtr2;
						buff += 2;
						tmpPtr2 += ofstPC;
					}
					tmpPtr += 2;
				}
				tmpPtr += blk - ofstPC;
				sizeLeft -= blk;
			}
			MemFree(tmpBuff);
		}
	}
	else
	{
		UInt32 blk = (this->format.nChannels * (UInt32)this->format.bitpersample >> 3);
		readSize = blkSize / blk;
		readSize = this->data->GetRealData(this->readOfst, readSize * blk, buff);
	}
	this->readOfst += readSize;
	if (this->readEvt)
		this->readEvt->Set();
	return readSize;
}

UOSInt Media::LPCMSource::GetMinBlockSize()
{
	return this->format.nChannels * (UOSInt)(this->format.bitpersample >> 3);
}

UInt32 Media::LPCMSource::GetCurrTime()
{
	UInt32 blk = (this->format.nChannels * (UInt32)this->format.bitpersample >> 3);
	return (UInt32)(this->readOfst * 1000 / this->format.frequency / blk);
}

Bool Media::LPCMSource::IsEnd()
{
	return this->readOfst >= this->data->GetDataSize();
}

Bool Media::LPCMSource::SupportSampleRead()
{
	return true;
}

UOSInt Media::LPCMSource::ReadSample(Int64 sampleOfst, UOSInt sampleCount, UInt8 *buff)
{
	UOSInt blk = (this->format.nChannels * (UInt32)this->format.bitpersample >> 3);
	if (sampleOfst < 0)
	{
		if (sampleOfst + (OSInt)sampleCount > 0)
		{
			MemClear(buff, (UOSInt)-sampleOfst * blk);
			return (UOSInt)((OSInt)(this->data->GetRealData(0, (UOSInt)((OSInt)sampleCount + sampleOfst) * blk, buff - sampleOfst * (OSInt)blk) / blk) - sampleOfst);
		}
		else
		{
			MemClear(buff, sampleCount * blk);
			return sampleCount;
		}
	}
	else
	{
		return this->data->GetRealData((UInt64)sampleOfst * blk, sampleCount * blk, buff) / blk;
	}
}

Int64 Media::LPCMSource::GetSampleCount()
{
	UInt32 blk = (this->format.nChannels * (UInt32)this->format.bitpersample >> 3);
	return (Int64)this->data->GetDataSize() / blk;
}
