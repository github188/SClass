#include "Stdafx.h"
#include "MyMemory.h"
#include "IO/BitReaderLSB.h"
#include "Data/ByteTool.h"

IO::BitReaderLSB::BitReaderLSB(IO::Stream *stm)
{
	if (stm)
	{
		this->buff = MemAlloc(UInt8, 1024);
		this->buffSize = 0;
		this->currBytePos = 0;
		this->currBitPos = 0;
		this->stm = stm;
	}
	else
	{
		this->buff = 0;
		this->buffSize = 0;
		this->currBytePos = 0;
		this->currBitPos = 0;
		this->stm = 0;
	}
}

IO::BitReaderLSB::BitReaderLSB(const UInt8 *buff, OSInt buffSize)
{
	this->buff = (UInt8*)buff;
	this->buffSize = buffSize;
	this->currBytePos = 0;
	this->currBitPos = 0;
	this->stm = 0;
}

IO::BitReaderLSB::~BitReaderLSB()
{
	if (this->stm)
	{
		MemFree(this->buff);
	}
}

Bool IO::BitReaderLSB::ReadBits(Int32 *code, OSInt bitCount)
{
#ifdef _DEBUG
	if (bitCount > 32 || bitCount <= 0)
		return false;
#endif
	if (((this->buffSize - this->currBytePos) << 3) - this->currBitPos < bitCount)
	{
		if (this->stm)
		{
			if (this->buffSize != this->currBytePos)
			{
				this->buffSize -= this->currBytePos;
				MemCopyO(this->buff, &this->buff[this->currBytePos], this->buffSize);
				this->currBytePos = 0;
			}
			else
			{
				this->buffSize = 0;
				this->currBytePos = 0;
			}
			OSInt readSize = this->stm->Read(&this->buff[this->buffSize], 1024 - this->buffSize);
			if (readSize <= 0)
				return false;
			this->buffSize += readSize;
			if (((this->buffSize - this->currBytePos) << 3) - this->currBitPos < bitCount)
				return false;
		}
		else
		{
			return false;
		}
	}
	OSInt bits = bitCount + this->currBitPos;
	OSInt bits2 = bits;
	if (bits2 & 7)
		bits2 += 8;
	Int32 retCode = 0;
	switch (bits2 >> 3)
	{
	case 1:
		retCode = buff[this->currBytePos] >> this->currBitPos;
		break;
	case 2:
		retCode = (buff[this->currBytePos] | (buff[this->currBytePos + 1] << 8)) >> this->currBitPos;
		break;
	case 3:
		retCode = (buff[this->currBytePos] | (buff[this->currBytePos + 1] << 8) | (buff[this->currBytePos + 2] << 16)) >> this->currBitPos;
		break;
	case 4:
		retCode = (Int32)(((UInt32)ReadInt32(&buff[this->currBytePos])) >> this->currBitPos);
		break;
	case 5:
		retCode = (((UInt32)ReadInt32(&buff[this->currBytePos])) >> this->currBitPos) | (buff[this->currBytePos + 4] << (32 - this->currBitPos));
		break;
	}
	*code = retCode & ((1 << bitCount) - 1);
	this->currBytePos += bits >> 3;
	this->currBitPos = bits & 7;
	return true;
}

Bool IO::BitReaderLSB::ByteAlign()
{
	if (this->currBitPos)
	{
		this->currBytePos++;
		this->currBitPos = 0;
	}
	return true;
}

OSInt IO::BitReaderLSB::ReadBytes(UInt8 *buff, OSInt cnt)
{
	this->ByteAlign();
	if (this->buffSize - this->currBytePos >= cnt)
	{
		MemCopyNO(buff, &this->buff[this->currBytePos], cnt);
		this->currBytePos += cnt;
		return cnt;
	}
	OSInt ret = this->buffSize - this->currBytePos;
	if (ret > 0)
	{
		MemCopyNO(buff, &this->buff[this->currBytePos], ret);
		cnt -= ret;
		this->currBytePos = this->buffSize;
	}
	if (cnt > 0)
	{
		OSInt readSize = this->stm->Read(&buff[ret], cnt);
		if (readSize > 0)
		{
			ret += cnt;
		}
	}
	return ret;
}