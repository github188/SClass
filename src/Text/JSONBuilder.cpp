#include "Stdafx.h"
#include "MyMemory.h"
#include "Text/JSONBuilder.h"
#include "Text/MyStringFloat.h"
#include "Text/MyStringW.h"

void Text::JSONBuilder::AppendStrUTF8(const UTF8Char *val)
{
	UTF8Char sbuff[256];
	UTF8Char *sptr;
	UTF8Char c;
	sptr = sbuff;
	*sptr++ = '\"';
	while ((c = *val++) != 0)
	{
		if (c == '\\')
		{
			*sptr++ = '\\';
			*sptr++ = '\\';
		}
		else if (c == '\"')
		{
			*sptr++ = '\\';
			*sptr++ = '\"';
		}
		else if (c == '\r')
		{
			*sptr++ = '\\';
			*sptr++ = 'r';
		}
		else if (c == '\n')
		{
			*sptr++ = '\\';
			*sptr++ = 'n';
		}
		else
		{
			*sptr++ = c;
		}
		if (sptr - sbuff >= 254)
		{
			sb->AppendC(sbuff, (UOSInt)(sptr - sbuff));
			sptr = sbuff;
		}
	}
	*sptr++ = '\"';
	sb->AppendC(sbuff, (UOSInt)(sptr - sbuff));
}

void Text::JSONBuilder::AppendStrW(const WChar *val)
{
	UTF8Char sbuff[256];
	UTF8Char *sptr;
	UTF32Char c;
	sptr = sbuff;
	*sptr++ = '\"';
	while (true)
	{
		val = Text::StrReadChar(val, &c);
		if (c == 0)
		{
			break;
		}
		else if (c == '\"')
		{
			*sptr++ = '\\';
			*sptr++ = '\\';
		}
		else if (c == '\"')
		{
			*sptr++ = '\\';
			*sptr++ = '\"';
		}
		else if (c == '\r')
		{
			*sptr++ = '\\';
			*sptr++ = 'r';
		}
		else if (c == '\n')
		{
			*sptr++ = '\\';
			*sptr++ = 'n';
		}
		else
		{
			sptr = Text::StrWriteChar(sptr, c);
		}
		if (sptr - sbuff >= 250)
		{
			sb->AppendC(sbuff, (UOSInt)(sptr - sbuff));
			sptr = sbuff;
		}
	}
	*sptr++ = '\"';
	sb->AppendC(sbuff, (UOSInt)(sptr - sbuff));
}

Text::JSONBuilder::JSONBuilder(Text::StringBuilderUTF *sb, ObjectType rootType)
{
	NEW_CLASS(this->objTypes, Data::ArrayList<ObjectType>());
	this->sb = sb;
	this->currType = rootType;
	this->isFirst = true;
	if (rootType == OT_ARRAY)
	{
		sb->AppendChar('[', 1);
	}
	else
	{
		sb->AppendChar('{', 1);
	}
}

Text::JSONBuilder::~JSONBuilder()
{
	UOSInt i;
	if (this->currType == OT_ARRAY)
	{
		this->sb->AppendChar(']', 1);
	}
	else
	{
		this->sb->AppendChar('}', 1);
	}
	i = this->objTypes->GetCount();
	while (i-- > 0)
	{
		if (this->objTypes->GetItem(i) == OT_OBJECT)
		{
			this->sb->AppendChar('}', 1);
		}
		else
		{
			this->sb->AppendChar(']', 1);
		}
	}
	DEL_CLASS(this->objTypes);
}

Bool Text::JSONBuilder::ArrayAddFloat64(Double val)
{
	if (this->currType != OT_ARRAY)
		return false;
	if (this->isFirst)
		this->isFirst = false;
	else
	{
		this->sb->AppendC((const UTF8Char*)",", 1);
	}
	Text::SBAppendF64(this->sb, val);
	return true;
}

Bool Text::JSONBuilder::ArrayBeginObject()
{
	if (this->currType != OT_ARRAY)
		return false;
	if (this->isFirst)
		this->isFirst = false;
	else
	{
		this->sb->AppendC((const UTF8Char*)",", 1);
	}
	this->objTypes->Add(OT_ARRAY);
	this->currType = OT_OBJECT;
	this->isFirst = true;
	this->sb->AppendChar('{', 1);
	return true;
}

Bool Text::JSONBuilder::ArrayBeginArray()
{
	if (this->currType != OT_ARRAY)
		return false;
	if (this->isFirst)
		this->isFirst = false;
	else
	{
		this->sb->AppendC((const UTF8Char*)",", 1);
	}
	this->objTypes->Add(OT_ARRAY);
	this->currType = OT_ARRAY;
	this->isFirst = true;
	this->sb->AppendChar('[', 1);
	return true;
}

Bool Text::JSONBuilder::ArrayEnd()
{
	if (this->currType != OT_ARRAY)
		return false;
	UOSInt i = this->objTypes->GetCount();
	if (i <= 0)
		return false;
	this->currType = this->objTypes->RemoveAt(i - 1);
	this->isFirst = false;
	this->sb->AppendChar(']', 1);
	return true;
}

Bool Text::JSONBuilder::ObjectAddFloat64(const UTF8Char *name, Double val)
{
	if (this->currType != OT_OBJECT)
		return false;
	if (this->isFirst)
		this->isFirst = false;
	else
	{
		this->sb->AppendC((const UTF8Char*)",", 1);
	}
	this->AppendStrUTF8(name);
	this->sb->AppendC((const UTF8Char*)":", 1);
	Text::SBAppendF64(this->sb, val);
	return true;
}

Bool Text::JSONBuilder::ObjectAddInt32(const UTF8Char *name, Int32 val)
{
	if (this->currType != OT_OBJECT)
		return false;
	if (this->isFirst)
		this->isFirst = false;
	else
	{
		this->sb->AppendC((const UTF8Char*)",", 1);
	}
	this->AppendStrUTF8(name);
	this->sb->AppendC((const UTF8Char*)":", 1);
	this->sb->AppendI32(val);
	return true;
}

Bool Text::JSONBuilder::ObjectAddInt64(const UTF8Char *name, Int64 val)
{
	if (this->currType != OT_OBJECT)
		return false;
	if (this->isFirst)
		this->isFirst = false;
	else
	{
		this->sb->AppendC((const UTF8Char*)",", 1);
	}
	this->AppendStrUTF8(name);
	this->sb->AppendC((const UTF8Char*)":", 1);
	this->sb->AppendI64(val);
	return true;
}

Bool Text::JSONBuilder::ObjectAddUInt64(const UTF8Char *name, UInt64 val)
{
	if (this->currType != OT_OBJECT)
		return false;
	if (this->isFirst)
		this->isFirst = false;
	else
	{
		this->sb->AppendC((const UTF8Char*)",", 1);
	}
	this->AppendStrUTF8(name);
	this->sb->AppendC((const UTF8Char*)":", 1);
	this->sb->AppendU64(val);
	return true;
}

Bool Text::JSONBuilder::ObjectAddStrUTF8(const UTF8Char *name, const UTF8Char *val)
{
	if (this->currType != OT_OBJECT)
		return false;
	if (this->isFirst)
		this->isFirst = false;
	else
	{
		this->sb->AppendC((const UTF8Char*)",", 1);
	}
	this->AppendStrUTF8(name);
	this->sb->AppendC((const UTF8Char*)":", 1);
	if (val == 0)
	{
		this->sb->AppendC((const UTF8Char*)"null", 4);
	}
	else
	{
		this->AppendStrUTF8(val);
	}
	return true;
}

Bool Text::JSONBuilder::ObjectAddStrW(const UTF8Char *name, const WChar *val)
{
	if (this->currType != OT_OBJECT)
		return false;
	if (this->isFirst)
		this->isFirst = false;
	else
	{
		this->sb->AppendC((const UTF8Char*)",", 1);
	}
	this->AppendStrUTF8(name);
	this->sb->AppendC((const UTF8Char*)":", 1);
	if (val == 0)
	{
		this->sb->AppendC((const UTF8Char*)"null", 4);
	}
	else
	{
		this->AppendStrW(val);
	}
	return true;
}

Bool Text::JSONBuilder::ObjectBeginArray(const UTF8Char *name)
{
	if (this->currType != OT_OBJECT)
		return false;
	if (this->isFirst)
		this->isFirst = false;
	else
	{
		this->sb->AppendC((const UTF8Char*)",", 1);
	}
	this->AppendStrUTF8(name);
	this->sb->AppendC((const UTF8Char*)":[", 2);
	this->objTypes->Add(OT_OBJECT);
	this->currType = OT_ARRAY;
	this->isFirst = true;
	return true;
}

Bool Text::JSONBuilder::ObjectBeginObject(const UTF8Char *name)
{
	if (this->currType != OT_OBJECT)
		return false;
	if (this->isFirst)
		this->isFirst = false;
	else
	{
		this->sb->AppendC((const UTF8Char*)",", 1);
	}
	this->AppendStrUTF8(name);
	this->sb->AppendC((const UTF8Char*)":{", 2);
	this->objTypes->Add(OT_OBJECT);
	this->currType = OT_OBJECT;
	this->isFirst = true;
	return true;
}

Bool Text::JSONBuilder::ObjectEnd()
{
	if (this->currType != OT_OBJECT)
		return false;
	UOSInt i = this->objTypes->GetCount();
	if (i <= 0)
		return false;
	this->currType = this->objTypes->RemoveAt(i - 1);
	this->isFirst = false;
	this->sb->AppendChar('}', 1);
	return true;
}
