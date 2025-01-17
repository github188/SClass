#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ArrayListInt64.h"
#include "Data/IComparable.h"

Data::ArrayListInt64::ArrayListInt64() : Data::SortableArrayList<Int64>()
{
}

Data::ArrayListInt64::ArrayListInt64(UOSInt capacity) : Data::SortableArrayList<Int64>(capacity)
{
}

Data::ArrayList<Int64> *Data::ArrayListInt64::Clone()
{
	Data::ArrayListInt64 *newArr;
	NEW_CLASS(newArr, Data::ArrayListInt64(this->capacity));
	newArr->AddRange(this);
	return newArr;
}

OSInt Data::ArrayListInt64::CompareItem(Int64 obj1, Int64 obj2)
{
	if (obj1 > obj2)
	{
		return 1;
	}
	else if (obj1 < obj2)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

void Data::ArrayListInt64::AddRangeI32(Int32 *arr, UOSInt cnt)
{
	UOSInt i;
	UOSInt j;
	while (this->objCnt + cnt >= this->capacity)
	{
		Int64 *newArr = MemAlloc(Int64, this->capacity * 2);
		i = objCnt;
		while (i-- > 0)
		{
			newArr[i] = this->arr[i];
		}
		this->capacity = this->capacity << 1;
		MemFree(this->arr);
		this->arr = newArr;
	}
	i = objCnt + cnt;
	j = cnt;
	while (j-- > 0)
	{
		this->arr[--i] = arr[j];
	}
	this->objCnt += cnt;
}

void Data::ArrayListInt64::AddRangeI32(Data::ArrayList<Int32> *arr)
{
	UOSInt i;
	UOSInt j;
	while (this->objCnt + arr->GetCount() >= this->capacity)
	{
		Int64 *newArr = MemAlloc(Int64, this->capacity * 2);
		i = objCnt;
		while (i-- > 0)
		{
			newArr[i] = this->arr[i];
		}
		this->capacity = this->capacity << 1;
		MemFree(this->arr);
		this->arr = newArr;
	}
	i = objCnt + arr->GetCount();
	j = arr->GetCount();
	while (j-- > 0)
	{
		this->arr[--i] = arr->GetItem(j);
	}
	this->objCnt += arr->GetCount();
}

