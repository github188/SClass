#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ArrayList.h"
#include "IO/FileStream.h"
#include "IO/UnixConfigFile.h"
#include "Sync/Event.h"
#include "Text/MyString.h"

IO::ConfigFile *IO::UnixConfigFile::Parse(const UTF8Char *fileName)
{
	IO::ConfigFile *cfg;
	IO::FileStream *fstm;
	Text::UTF8Reader *reader;
	NEW_CLASS(fstm, IO::FileStream(fileName, IO::FileStream::FILE_MODE_READONLY, IO::FileStream::FILE_SHARE_DENY_NONE, IO::FileStream::BT_SEQUENTIAL));
	if (fstm->IsError())
	{
		cfg = 0;
	}
	else
	{
		NEW_CLASS(reader, Text::UTF8Reader(fstm));
		cfg = ParseReader(reader);
		DEL_CLASS(reader);
	}
	DEL_CLASS(fstm);
	return cfg;
}

IO::ConfigFile *IO::UnixConfigFile::ParseReader(Text::UTF8Reader *reader)
{
	UTF8Char buff[1024];
	UTF8Char *name;
	UTF8Char *value;
	UTF8Char *src;
	UTF8Char c;
	IO::ConfigFile *cfg;
	NEW_CLASS(cfg, IO::ConfigFile());
	while ((name = reader->ReadLine(buff, 1023)) != 0)
	{
		Text::StrTrim(buff);
		if (buff[0] != 0 && buff[0] != '#')
		{
			name = buff;
			value = 0;
			src = buff;
			while (*src != '=')
			{
				if (*src == 0)
					break;
				src++;
			}
			if (*src == '=')
			{
				*src++ = 0;
				
				if (src[0] == '\"')
				{
					src++;
					value = src;
					while ((c = *src++) != 0)
					{
						if (c == '\"')
						{
							src[-1] = 0;
							cfg->SetValue(0, name, value);
							break;
						}
					}
				}
				else
				{
					cfg->SetValue(0, name, src);
				}
			}
		}
	}
	if (cfg->GetCateCount() == 0)
	{
		DEL_CLASS(cfg);
		return 0;
	}
	return cfg;
}

