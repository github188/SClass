#include "Stdafx.h"
#include "MyMemory.h"
#include "IO/FileStream.h"
#include "IO/IniFile.h"
#include "IO/Path.h"
#include "IO/StreamReader.h"
#include "IO/SystemInfo.h"
#include "Manage/Process.h"
#include "Text/MyString.h"
#include "Text/StringBuilderUTF8.h"
#include <stdio.h>
#include <sys/sysinfo.h>

typedef struct
{
	const UTF8Char *platformName;
	const UTF8Char *platformSN;
} SystemData;

IO::SystemInfo::SystemInfo()
{
	IO::FileStream *fs;
	IO::StreamReader *reader;
	OSInt i;
	SystemData *data = MemAlloc(SystemData, 1);
	data->platformName = 0;
	data->platformSN = 0;
	this->clsData = data;
	IO::ConfigFile *cfg = IO::IniFile::Parse((const UTF8Char*)"/etc/model.conf", 65001);
	if (cfg)
	{
		const UTF8Char *csptr = cfg->GetValue((const UTF8Char*)"System Enclosure", (const UTF8Char*)"VENDOR");
		const UTF8Char *csptr2 = cfg->GetValue((const UTF8Char*)"System Enclosure", (const UTF8Char*)"MODEL");
		if (csptr && csptr2)
		{
			Text::StringBuilderUTF8 sb;
			sb.Append(csptr);
			sb.AppendChar(' ', 1);
			sb.Append(csptr2);
			data->platformName = Text::StrCopyNew(sb.ToString());
		}
		DEL_CLASS(cfg);
	}
}

IO::SystemInfo::~SystemInfo()
{
	SystemData *data = (SystemData*)this->clsData;
	SDEL_TEXT(data->platformName);
	SDEL_TEXT(data->platformSN);
	MemFree(data);
}

UTF8Char *IO::SystemInfo::GetPlatformName(UTF8Char *sbuff)
{
	SystemData *data = (SystemData*)this->clsData;
	if (data->platformName)
	{
		return Text::StrConcat(sbuff, data->platformName);
	}
	return 0;
}

UTF8Char *IO::SystemInfo::GetPlatformSN(UTF8Char *sbuff)
{
	SystemData *data = (SystemData*)this->clsData;
	if (data->platformSN)
	{
		return Text::StrConcat(sbuff, data->platformSN);
	}
	return 0;
}

Int64 IO::SystemInfo::GetTotalMemSize()
{
	struct sysinfo info;
	if (sysinfo(&info) == 0)
	{
		return info.totalram * (Int64)info.mem_unit;
	}
	return 0;
}

Int64 IO::SystemInfo::GetTotalUsableMemSize()
{
	struct sysinfo info;
	if (sysinfo(&info) == 0)
	{
		return info.totalram * (Int64)info.mem_unit;
	}
	return 0;
}

IO::SystemInfo::ChassisType IO::SystemInfo::GetChassisType()
{
	return IO::SystemInfo::CT_RAID;
}

OSInt IO::SystemInfo::GetRAMInfo(Data::ArrayList<RAMInfo*> *ramList)
{
	return 0;
}

void IO::SystemInfo::FreeRAMInfo(Data::ArrayList<RAMInfo*> *ramList)
{
}
