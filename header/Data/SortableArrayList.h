#ifndef _SM_DATA_SORTABLEARRAYLIST
#define _SM_DATA_SORTABLEARRAYLIST
#include "Data/ArrayList.h"

namespace Data
{
	template <class T> class SortableArrayList : public Data::ArrayList<T>
	{
	public:
		SortableArrayList() : ArrayList<T>() {};
		SortableArrayList(UOSInt capacity) : ArrayList<T>() {};

		virtual OSInt CompareItem(T obj1, T obj2) = 0;
		UOSInt SortedInsert(T val);
		OSInt SortedIndexOf(T val);
	};

	template <class T> UOSInt Data::SortableArrayList<T>::SortedInsert(T val)
	{
		OSInt i;
		OSInt j;
		OSInt k;
		OSInt l;
		i = 0;
		j = (OSInt)this->objCnt - 1;
		while (i <= j)
		{
			k = (i + j) >> 1;
			l = this->CompareItem(this->arr[k], val);
			if (l > 0)
			{
				j = k - 1;
			}
			else if (l < 0)
			{
				i = k + 1;
			}
			else
			{
				i = k + 1;
				break;
			}
		}
		this->Insert((UOSInt)i, val);
		return (UOSInt)i;
	}

	template <class T> OSInt Data::SortableArrayList<T>::SortedIndexOf(T val)
	{
		OSInt i;
		OSInt j;
		OSInt k;
		OSInt l;
		i = 0;
		j = (OSInt)this->objCnt - 1;
		while (i <= j)
		{
			k = (i + j) >> 1;
			l = this->CompareItem(this->arr[k], val);
			if (l > 0)
			{
				j = k - 1;
			}
			else if (l < 0)
			{
				i = k + 1;
			}
			else
			{
				return k;
			}
		}
		return -i - 1;
	}
}
#endif
