#ifndef _SM_IO_VIEWFILEBUFFER
#define _SM_IO_VIEWFILEBUFFER
#include "IO/IFileBuffer.h"

namespace IO
{
	class ViewFileBuffer : public IO::IFileBuffer
	{
	private:
		UInt8 *filePtr;
		void *fileHandle;
		void *mapHandle;

	public:
		ViewFileBuffer(const UTF8Char *fileName);
		virtual ~ViewFileBuffer();

		virtual UInt8 *GetPointer();
		virtual Int64 GetLength();

	private:
		void FreePointer();
	};
};
#endif