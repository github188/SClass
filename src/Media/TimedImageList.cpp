#include "Stdafx.h"
#include "Data/ByteTool.h"
#include "Media/TimedImageList.h"

void Media::TimedImageList::ScanFile()
{
	Int64 currOfst = 16;
	UInt8 indexBuff[32];
	this->flags &= ~2;
	this->changed = true;
	this->fs->Seek(IO::SeekableStream::ST_BEGIN, 16);
	while (this->fs->Read(indexBuff, 32) == 32)
	{
		if (ReadInt64(&indexBuff[16]) == currOfst + 32)
		{
			Int64 imgSize = ReadInt64(&indexBuff[24]);
			this->indexStm->Write(indexBuff, 32);
			currOfst += 32 + imgSize;
			this->fs->Seek(IO::SeekableStream::ST_CURRENT, (OSInt)imgSize);
		}
		else
		{
			break;
		}
	}
	this->fs->Seek(IO::SeekableStream::ST_BEGIN, currOfst);
	this->currFileOfst = currOfst;
}

Media::TimedImageList::TimedImageList(const UTF8Char *fileName)
{
	UInt8 hdr[16];
	NEW_CLASS(this->indexStm, IO::MemoryStream((const UTF8Char*)"Media.TimedImageLIst.indexStm"));
	NEW_CLASS(this->fs, IO::FileStream(fileName, IO::FileStream::FILE_MODE_APPEND, IO::FileStream::FILE_SHARE_DENY_NONE, IO::FileStream::BT_NO_WRITE_BUFFER));
	if (this->IsError())
	{
		DEL_CLASS(this->fs);
		this->fs = 0;
		return;
	}
	Int64 fileSize = this->fs->GetLength();
	if (fileSize > 0 && fileSize < 16)
	{
		DEL_CLASS(this->fs);
		this->fs = 0;
		return;
	}
	else if (fileSize == 0)
	{
		hdr[0] = 'S';
		hdr[1] = 'T';
		hdr[2] = 'i';
		hdr[3] = 'l';
		WriteInt32(&hdr[4], 0x1);
		WriteInt64(&hdr[8], 16);
		this->fs->Write(hdr, 16);
		this->flags = 1;
		this->currFileOfst = 16;
		this->changed = false;
	}
	else
	{
		this->fs->Seek(IO::SeekableStream::ST_BEGIN, 0);
		this->fs->Read(hdr, 16);
		if (hdr[0] != 'S' || hdr[1] != 'T' || hdr[2] != 'i' || hdr[3] != 'l')
		{
			DEL_CLASS(this->fs);
			this->fs = 0;
			return;
		}
		this->flags = ReadInt32(&hdr[4]);
		this->currFileOfst = ReadInt64(&hdr[8]);
		if (this->flags & 2)
		{
			Int64 indexSize = fileSize - this->currFileOfst;
			if (indexSize >= 0 && (indexSize & 31) == 0)
			{
				UInt8 *indexBuff = MemAlloc(UInt8, (OSInt)indexSize);
				this->fs->Seek(IO::SeekableStream::ST_BEGIN, this->currFileOfst);
				this->fs->Read(indexBuff, (OSInt)indexSize);
				this->indexStm->Write(indexBuff, (OSInt)indexSize);
				MemFree(indexBuff);
				this->fs->Seek(IO::SeekableStream::ST_BEGIN, this->currFileOfst);
			}
			else if (this->flags & 1)
			{
				ScanFile();
			}
			else
			{
				this->fs->Seek(IO::SeekableStream::ST_BEGIN, 16);
				this->currFileOfst = 16;
				this->changed = true;
			}
		}
		else if (this->flags & 1)
		{
			ScanFile();
		}
		else
		{
			this->fs->Seek(IO::SeekableStream::ST_BEGIN, 16);
			this->currFileOfst = 16;
			this->changed = true;
		}
	}
}

Media::TimedImageList::~TimedImageList()
{
	UInt8 hdr[16];
	if (this->fs)
	{
		if (this->changed)
		{
			UOSInt indexSize;
			UInt8 *indexBuff;
			indexBuff = this->indexStm->GetBuff(&indexSize);
			this->fs->Write(indexBuff, indexSize);

			this->fs->Seek(IO::SeekableStream::ST_BEGIN, 0);
			hdr[0] = 'S';
			hdr[1] = 'T';
			hdr[2] = 'i';
			hdr[3] = 'l';
			WriteInt32(&hdr[4], this->flags | 2);
			WriteInt64(&hdr[8], this->currFileOfst);
			this->fs->Write(hdr, 16);
		}
		DEL_CLASS(this->fs);
		this->fs = 0;
	}
	DEL_CLASS(this->indexStm);
}

Bool Media::TimedImageList::IsError()
{
	return this->fs == 0;
}

Bool Media::TimedImageList::AddImage(Int64 captureTimeTicks, const UInt8 *imgBuff, UOSInt imgSize, ImageFormat imgFmt)
{
	UInt8 indexBuff[32];
	Bool succ = true;
	if (!this->changed)
	{
		this->fs->Seek(IO::SeekableStream::ST_BEGIN, 0);
		this->flags &= ~2;
		indexBuff[0] = 'S';
		indexBuff[1] = 'T';
		indexBuff[2] = 'i';
		indexBuff[3] = 'l';
		WriteInt32(&indexBuff[4], this->flags);
		WriteInt64(&indexBuff[8], this->currFileOfst);
		this->fs->Write(indexBuff, 16);
		this->fs->Seek(IO::SeekableStream::ST_BEGIN, this->currFileOfst);
		this->changed = true;
	}
	WriteInt64(&indexBuff[0], captureTimeTicks);
	WriteInt32(&indexBuff[8], 0); //flags
	WriteInt32(&indexBuff[12], imgFmt);
	WriteInt64(&indexBuff[16], this->currFileOfst + 32);
	WriteInt64(&indexBuff[24], imgSize);
	succ = succ && (this->fs->Write(indexBuff, 32) == 32);
	this->indexStm->Write(indexBuff, 32);
	succ = succ && (this->fs->Write(imgBuff, imgSize) == imgSize);
	this->currFileOfst += 32 + imgSize;
	return succ;
}