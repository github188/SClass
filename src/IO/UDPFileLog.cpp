#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ByteTool.h"
#include "IO/UDPFileLog.h"
#include "Net/SocketFactory.h"
#include "Text/Encoding.h"

IO::UDPFileLog::UDPFileLog(IO::IStreamData *fd) : IO::UDPLog(fd->GetFullName())
{
	UInt8 *buff;

	this->fd = fd->GetPartialData(0, fd->GetDataSize());
	this->logBuff = MemAlloc(UInt8, 65544);
	NEW_CLASS(this->logPos, Data::ArrayListUInt64());
	NEW_CLASS(this->logSize, Data::ArrayListUInt32());

	UInt64 currPos;
	UInt64 buffPos;
	UInt64 fdSize;
	UOSInt buffSize;
	UOSInt endOfst;
	UOSInt readSize;
	UInt32 packetSize;
	UOSInt i;

	buff = MemAlloc(UInt8, 1048576);
	fdSize = this->fd->GetDataSize();
	buffSize = 0;
	currPos = 0;

	while (currPos < fdSize)
	{
		readSize = 1048576 - buffSize;
		if (readSize > fdSize - currPos)
		{
			readSize = (UOSInt)(fdSize - currPos);
		}
		this->fd->GetRealData(currPos, readSize, &buff[buffSize]);
		buffPos = currPos - buffSize;
		buffSize += readSize;
		endOfst = buffSize - 8;

		i = 0;
		while (i < endOfst)
		{
			if (buff[i] == 0xaa && buff[i + 1] == 0xbb)
			{
				packetSize = ReadUInt16(&buff[i + 2]);
				if (packetSize + 8 + i > buffSize)
					break;
				
				this->logPos->Add(buffPos + i);
				this->logSize->Add(packetSize + 8);

				i += packetSize + 8;
			}
			else
			{
				i++;
			}
		}
		if (i < buffSize)
		{
			MemCopyO(buff, &buff[i], buffSize - i);
			buffSize -= i;
		}
		else
		{
			buffSize = 0;
		}

		currPos += readSize;
	}
	

	MemFree(buff);
}

IO::UDPFileLog::~UDPFileLog()
{
	MemFree(this->logBuff);
	DEL_CLASS(this->fd);
	DEL_CLASS(this->logPos);
	DEL_CLASS(this->logSize);
}

UOSInt IO::UDPFileLog::GetCount(IO::ILogHandler::LogLevel logLevel)
{
	if (logLevel == IO::ILogHandler::LOG_LEVEL_RAW)
	{
		return this->logPos->GetCount() << 1;
	}
/*	if (logLevel == IO::ILogHandler::LOG_LEVEL_RAW)
	{
		return this->logPos->GetCount();
	}*/
	else
	{
		return this->logPos->GetCount();
	}
}

Bool IO::UDPFileLog::GetLogMessage(IO::ILogHandler::LogLevel logLevel, UOSInt index, Data::DateTime *dt, Text::StringBuilderUTF *sb, Text::LineBreakType lineBreak)
{
	if (logLevel == IO::ILogHandler::LOG_LEVEL_RAW)
	{
		if (index >= (this->logPos->GetCount() << 1))
			return false;
		if (index & 1)
		{
			index = index >> 1;

			UInt64 pos = this->logPos->GetItem(index);
			UInt32 size = this->logSize->GetItem(index);
			this->fd->GetRealData(pos, size, this->logBuff);
			dt->SetUnixTimestamp(*(UInt32*)&this->logBuff[4]);
			return ParseLog(&this->logBuff[8], size - 8, sb, false);
		}
		index = index >> 1;
	}
/*	if (logLevel == IO::ILogHandler::LOG_LEVEL_RAW)
	{
		if (index >= this->logPos->GetCount())
			return false;
		Int64 pos = this->logPos->GetItem(index);
		Int32 size = this->logSize->GetItem(index);
		this->fd->GetRealData(pos, size, this->logBuff);
		dt->SetUnixTimestamp(*(UInt32*)&this->logBuff[4]);
		return ParseLog(&this->logBuff[8], size - 8, sb, false);
	}*/
	else
	{
		if (index >= this->logPos->GetCount())
			return false;
	}
	UInt64 pos = this->logPos->GetItem(index);
	UInt32 size = this->logSize->GetItem(index);
	this->fd->GetRealData(pos, size, this->logBuff);
	dt->SetUnixTimestamp(*(UInt32*)&this->logBuff[4]);
	sb->AppendHexBuff(&this->logBuff[8], size - 8, ' ', lineBreak);
	return true;
}

Bool IO::UDPFileLog::GetLogDescription(IO::ILogHandler::LogLevel logLevel, UOSInt index, Text::StringBuilderUTF *sb)
{
	if (logLevel == IO::ILogHandler::LOG_LEVEL_RAW)
	{
		index = index >> 1;
	}
	if (index >= this->logPos->GetCount())
		return false;
	Data::DateTime dt;
	UInt64 pos = this->logPos->GetItem(index);
	UInt32 size = this->logSize->GetItem(index);
	this->fd->GetRealData(pos, size, this->logBuff);
	dt.SetUnixTimestamp(*(UInt32*)&this->logBuff[4]);
	return this->ParseLog(&this->logBuff[8], size - 8, sb, true);
}
