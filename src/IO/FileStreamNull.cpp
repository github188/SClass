#include "Stdafx.h"
#include "MyMemory.h"
#include "IO/FileStream.h"

IO::FileStream::FileStream(const WChar *fileName, IO::FileStream::FileMode mode, FileShare share, IO::FileStream::BufferType buffType) : IO::SeekableStream((const UTF8Char*)"FileStream")
{
	this->handle = 0;
	this->currPos = -1;
	this->handle = 0;
	return;
}

IO::FileStream::FileStream(const UTF8Char *fileName, FileMode mode, FileShare share, IO::FileStream::BufferType buffType) : IO::SeekableStream(fileName)
{
	this->handle = 0;
	this->currPos = -1;
	this->handle = 0;
}

IO::FileStream::~FileStream()
{
}

Bool IO::FileStream::IsError()
{
	return true;
}

OSInt IO::FileStream::Read(UInt8 *buff, OSInt size)
{
	return 0;
}

OSInt IO::FileStream::Write(const UInt8 *buff, OSInt size)
{
	return 0;
}

Int32 IO::FileStream::Flush()
{
	return 0;
}

void IO::FileStream::Close()
{
}

Bool IO::FileStream::Recover()
{
	return false;
}

Int64 IO::FileStream::Seek(IO::SeekableStream::SeekType origin, Int64 position)
{
	return -1;
}

Int64 IO::FileStream::GetPosition()
{
	return this->currPos;
}

Int64 IO::FileStream::GetLength()
{
	return -1;
}

void IO::FileStream::SetLength(Int64 newLength)
{
}

Int32 IO::FileStream::GetErrCode()
{
}

void IO::FileStream::GetFileTimes(Data::DateTime *creationTime, Data::DateTime *lastAccessTime, Data::DateTime *lastWriteTime)
{
}

void IO::FileStream::SetFileTimes(Data::DateTime *creationTime, Data::DateTime *lastAccessTime, Data::DateTime *lastWriteTime)
{
}

