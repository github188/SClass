#include "Stdafx.h"
#include "MyMemory.h"
#include "Text/MyString.h"
#include "Data/ArrayListStrUTF8.h"

Data::ArrayListStrUTF8::ArrayListStrUTF8() : Data::SortableArrayList<const UTF8Char*>()
{
}

Data::ArrayListStrUTF8::ArrayListStrUTF8(UOSInt capacity) : Data::SortableArrayList<const UTF8Char*>(capacity)
{
}

Data::ArrayList<const UTF8Char*> *Data::ArrayListStrUTF8::Clone()
{
	Data::ArrayListStrUTF8 *newArr;
	NEW_CLASS(newArr, Data::ArrayListStrUTF8(this->capacity));
	newArr->AddRange(this);
	return newArr;
}

OSInt Data::ArrayListStrUTF8::CompareItem(const UTF8Char* obj1, const UTF8Char* obj2)
{
	return Text::StrCompare(obj1, obj2);
}

const UTF8Char *Data::ArrayListStrUTF8::JoinNewStr()
{
	UOSInt j;
	UOSInt k;
	k = j = this->objCnt;
	UOSInt strSize = 0;
	while (j-- > 0)
	{
		strSize += Text::StrCharCnt(arr[j]);
	}
	UTF8Char *sbuff = MemAlloc(UTF8Char, strSize + 1);
	UTF8Char *sptr = sbuff;
	j = 0;
	while (j < k)
	{
		sptr = Text::StrConcat(sptr, arr[j]);
		j++;
	}
	return sbuff;
}
