#ifndef _SM_IO_STMDATA_BLOCKSTREAMDATA
#define _SM_IO_STMDATA_BLOCKSTREAMDATA
#include "IO/IStreamData.h"
#include "IO/FileStream.h"
#include "Data/ArrayListInt32.h"
#include "Data/ArrayListInt64.h"

namespace IO
{
	namespace StmData
	{
		class BlockStreamData : public IO::IStreamData
		{
		private:
			IO::IStreamData *sd;
			Data::ArrayListInt64 *dataOfstList;
			Data::ArrayListInt64 *stmOfstList;
			Data::ArrayListInt32 *lengthList;
			UInt64 totalSize;

		public:
			BlockStreamData(IO::IStreamData *sd);
			virtual ~BlockStreamData();

			virtual UOSInt GetRealData(UInt64 offset, UOSInt length, UInt8 *buffer);
			virtual const UTF8Char *GetFullName();
			virtual const UTF8Char *GetShortName();
			virtual UInt64 GetDataSize();
			virtual const UInt8 *GetPointer();

			virtual IO::IStreamData *GetPartialData(UInt64 offset, UInt64 length);
			virtual Bool IsFullFile();
			virtual Bool IsLoading();
			virtual OSInt GetSeekCount();

			void Append(UInt64 ofst, UInt32 length);
		};
	}
}
#endif