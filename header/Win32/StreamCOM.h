#ifndef _SM_WIN32_STREAMCOM
#define _SM_WIN32_STREAMCOM
#include "IO/SeekableStream.h"
#include "Data/DateTime.h"

namespace Win32
{
	class StreamCOM : public IO::SeekableStream
	{
	private:
		IStream *stm;
		Bool toRelease;

	public:
		StreamCOM(IStream *stm, Bool toRelease);
		virtual ~StreamCOM();

		virtual UOSInt Read(UInt8 *buff, UOSInt size);
		virtual UOSInt Write(const UInt8 *buff, UOSInt size);

		virtual Int32 Flush();
		virtual void Close();
		virtual Bool Recover();
		virtual UInt64 SeekFromBeginning(UInt64 position);
		virtual UInt64 SeekFromCurrent(Int64 position);
		virtual UInt64 SeekFromEnd(Int64 position);
		virtual UInt64 GetPosition();
		virtual UInt64 GetLength();
	};
}
#endif
