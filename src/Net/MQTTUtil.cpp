#include "Stdafx.h"
#include "Net/MQTTUtil.h"
#include "Text/MyString.h"
#include "Text/StringBuilderUTF8.h"

Bool Net::MQTTUtil::TopicValid(const UTF8Char *topic)
{
	UOSInt i;
	UOSInt j;
	i = Text::StrIndexOf(topic, '#');
	if (i != INVALID_INDEX)
	{
		if (topic[i + 1])
		{
			return false;
		}
		if (i > 0 && topic[i - 1] != '/')
		{
			return false;
		}
	}
	i = 0;
	while (true)
	{
		j = Text::StrIndexOf(&topic[i], '+');
		if (j == INVALID_INDEX)
			break;
		if (i + j > 0 && topic[i + j - 1] != '/')
		{
			return false;
		}
		if (topic[i + j + 1] != '/' && topic[i + j + 1] != 0)
		{
			return false;
		}
		i += j + 2;
	}
	return true;
}

Bool Net::MQTTUtil::TopicMatch(const UTF8Char *topic, const UTF8Char *subscribeTopic)
{
	if (subscribeTopic[0] == '#' && subscribeTopic[1] == 0)
	{
		if (!Text::StrStartsWith(topic, (const UTF8Char*)"$SYS/"))
		{
			return true;
		}
		return false;
	}
	UOSInt i;
	Text::StringBuilderUTF8 sb;
	while (true)
	{
		i = Text::StrIndexOf(subscribeTopic, '+');
		if (i == INVALID_INDEX)
			break;
		if (i > 0)
		{
			sb.ClearStr();
			sb.AppendC(subscribeTopic, (UOSInt)i);
			if (!Text::StrStartsWith(topic, sb.ToString()))
			{
				return false;
			}
			topic += i;
			subscribeTopic += i;
		}
		i = Text::StrIndexOf(topic, '/');
		if (subscribeTopic[1] == 0)
		{
			return (i == INVALID_INDEX);
		}
		else if (i == INVALID_INDEX)
		{
			return false;
		}
		subscribeTopic++;
		topic += i;
	}
	i = Text::StrIndexOf(subscribeTopic, '#');
	if (i == INVALID_INDEX)
	{
		return Text::StrEquals(topic, subscribeTopic);
	}
	else if (i == 0)
	{
		return true;
	}

	sb.ClearStr();
	sb.AppendC(subscribeTopic, (UOSInt)i);
	if (!Text::StrStartsWith(topic, sb.ToString()))
	{
		return false;
	}
	return true;
}
