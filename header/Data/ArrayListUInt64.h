#ifndef _SM_DATA_ARRAYLISTUINT64
#define _SM_DATA_ARRAYLISTUINT64
#include "Data/SortableArrayList.h"

namespace Data
{
	class ArrayListUInt64 : public Data::SortableArrayList<UInt64>
	{
	public:
		ArrayListUInt64();
		ArrayListUInt64(UOSInt capacity);

		virtual ArrayList<UInt64> *Clone();

		virtual UOSInt SortedInsert(UInt64 val);
		virtual OSInt SortedIndexOf(UInt64 val);
		void AddRangeU32(UInt32 *arr, UOSInt cnt);
		void AddRangeU32(Data::ArrayList<UInt32> *arr);
	};
}
#endif