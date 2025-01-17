#include "Stdafx.h"
#include "MyMemory.h"
#include "IO/Path.h"
#include "Text/MyStringW.h"

#if !defined(PATH_MAX)
#define PATH_MAX 512
#endif

UTF8Char IO::Path::PATH_SEPERATOR = (UTF8Char)'/';
const UTF8Char *IO::Path::ALL_FILES = (const UTF8Char*)"*";

WChar *IO::Path::GetTempFileW(WChar *buff, const WChar *fileName)
{
	return 0;
}

Bool IO::Path::IsDirectoryExistW(const WChar *dir)
{
	return false;
}

Bool IO::Path::CreateDirectoryW(const WChar *dirInput)
{
	return false;
}

Bool IO::Path::RemoveDirectoryW(const WChar *dir)
{
	return false;
}

Bool IO::Path::DeleteFileW(const WChar *fileName)
{
	return false;
}

OSInt IO::Path::FileNameCompareW(const WChar *file1, const WChar *file2)
{
	return Text::StrCompare(file1, file2);
}

WChar *IO::Path::GetFileDirectoryW(WChar *buff, const WChar *fileName)
{
	return Text::StrConcat(buff, fileName);
}

UTF8Char *IO::Path::GetProcessFileName(UTF8Char *buff)
{
	return 0;
}

WChar *IO::Path::GetProcessFileNameW(WChar *buff)
{
	return 0;
}

Bool IO::Path::GetProcessFileName(Text::StringBuilderUTF *sb)
{
	return false;
}

UTF8Char *IO::Path::ReplaceExt(UTF8Char *fileName, const UTF8Char *ext)
{
	UTF8Char *oldExt = 0;
	UTF8Char c;
	while ((c = *fileName++) != 0)
	{
		if (c == '\\' || c == '/')
		{
			oldExt = 0;
		}
		else if (c == '.')
		{
			oldExt = fileName;
		}
	}
	if (oldExt == 0)
	{
		oldExt = fileName;
		oldExt[-1] = '.';
	}
	return Text::StrConcat(oldExt, ext);
}

WChar *IO::Path::ReplaceExtW(WChar *fileName, const WChar *ext)
{
	WChar *oldExt = 0;
	WChar c;
	while ((c = *fileName++) != 0)
	{
		if (c == '\\' || c == '/')
		{
			oldExt = 0;
		}
		else if (c == '.')
		{
			oldExt = fileName;
		}
	}
	if (oldExt == 0)
	{
		oldExt = fileName;
		oldExt[-1] = '.';
	}
	return Text::StrConcat(oldExt, ext);
}

WChar *IO::Path::GetFileExtW(WChar *fileBuff, const WChar *path)
{
	UOSInt i = Text::StrLastIndexOf(path, '/');
	if (i != INVALID_INDEX)
	{
		path = &path[i + 1];
	}
	i = Text::StrLastIndexOf(path, '.');
	if (i != INVALID_INDEX)
	{
		return Text::StrConcat(fileBuff, &path[i + 1]);
	}
	else
	{
		return Text::StrConcat(fileBuff, L"");
	}
}

WChar *IO::Path::AppendPathW(WChar *path, const WChar *toAppend)
{
	if (toAppend[0] == '/')
		return Text::StrConcat(path, toAppend);
	UOSInt i = Text::StrLastIndexOf(path, '/');
	if (GetPathTypeW(path) == PT_FILE && i != INVALID_INDEX)
	{
		path[i] = 0;
		i = Text::StrLastIndexOf(path, '/');
	}
	while (Text::StrStartsWith(toAppend, L"../"))
	{
		if (i != INVALID_INDEX)
		{
			path[i] = 0;
			i = Text::StrLastIndexOf(path, '/');
		}
		toAppend += 3;
	}
	i = Text::StrCharCnt(path);
	path[i] = '/';
	return Text::StrConcat(&path[i + 1], toAppend);
}

Bool IO::Path::AppendPath(Text::StringBuilderUTF8 *sb, const UTF8Char *toAppend)
{
	if (toAppend[0] == '/')
	{
		sb->ClearStr();
		sb->Append(toAppend);
		return true;
	}
	UOSInt strLen = Text::StrCharCnt(toAppend);
	UOSInt i = Text::StrLastIndexOf(sb->ToString(), '/');
	if (GetPathType(sb->ToString()) == PT_FILE && i != INVALID_INDEX)
	{
		sb->RemoveChars(sb->GetLength() - i);
		i = Text::StrLastIndexOf(sb->ToString(), '/');
	}
	while (Text::StrStartsWith(toAppend, (const UTF8Char*)"../"))
	{
		if (i != INVALID_INDEX)
		{
			sb->RemoveChars(sb->GetLength() - i);
			i = Text::StrLastIndexOf(sb->ToString(), '/');
		}
		toAppend += 3;
	}
	sb->AppendChar('/', 1);
	sb->Append(toAppend);
	return true;
}

IO::Path::FindFileSession *IO::Path::FindFile(const UTF8Char *path)
{
	return 0;
}

IO::Path::FindFileSession *IO::Path::FindFileW(const WChar *path)
{
	return 0;
}

UTF8Char *IO::Path::FindNextFile(UTF8Char *buff, IO::Path::FindFileSession *session, Data::DateTime *modTime, IO::Path::PathType *pt, UInt64 *fileSize)
{
	return 0;
}

WChar *IO::Path::FindNextFileW(WChar *buff, IO::Path::FindFileSession *session, Data::DateTime *modTime, IO::Path::PathType *pt, UInt64 *fileSize)
{
	return 0;
}

void IO::Path::FindFileClose(IO::Path::FindFileSession *session)
{
}

IO::Path::PathType IO::Path::GetPathTypeW(const WChar *path)
{
	return PT_UNKNOWN;
}

WChar *IO::Path::GetFullPathW(WChar *buff, const WChar *path)
{
	return Text::StrConcat(buff, path);
}

Bool IO::Path::FileNameMatchW(const WChar *path, const WChar *searchPattern)
{
	WChar sbuff[256];
	UOSInt i = Text::StrLastIndexOf(path, '/');
	const WChar *fileName = &path[i + 1];
	Text::StrConcat(sbuff, searchPattern);
	Bool isWC = false;
	WChar *patternStart = 0;
	WChar *currPattern = sbuff;
	WChar c;
	while (true)
	{
		c = *currPattern;
		if (c == 0)
		{
			if (isWC)
			{
				if (patternStart == 0)
					return true;
				return Text::StrEndsWith(fileName, patternStart);
			}
			else if (patternStart)
			{
				return Text::StrCompareICase(fileName, patternStart) == 0;
			}
			else
			{
				return *fileName == 0;
			}
		}
		else if (c == '?' || c == '*')
		{
			if (isWC)
			{
				if (patternStart == 0)
					return false;
				*currPattern = 0;
				if ((i = Text::StrIndexOf(fileName, patternStart)) == INVALID_INDEX)
					return false;
				fileName += i + currPattern - patternStart;
				patternStart = 0;
				isWC = false;
			}
			else if (patternStart)
			{
				*currPattern = 0;
				if (!Text::StrStartsWithICase(fileName, patternStart))
					return false;
				fileName += currPattern - patternStart;
				patternStart = 0;
				isWC = false;
			}
			if (c == '?')
			{
				if (*fileName == 0)
					return false;
				fileName++;
				currPattern++;
			}
			else
			{
				isWC = true;
				patternStart = 0;
				currPattern++;
			}
		}
		else
		{
			if (patternStart == 0)
			{
				patternStart = currPattern;
			}
			currPattern++;
		}
	}

}

Bool IO::Path::FileNameMatch(const UTF8Char *path, const UTF8Char *searchPattern)
{
	UTF8Char sbuff[256];
	UOSInt i = Text::StrLastIndexOf(path, '/');
	const UTF8Char *fileName = &path[i + 1];
	Text::StrConcat(sbuff, searchPattern);
	Bool isWC = false;
	UTF8Char *patternStart = 0;
	UTF8Char *currPattern = sbuff;
	UTF8Char c;
	while (true)
	{
		c = *currPattern;
		if (c == 0)
		{
			if (isWC)
			{
				if (patternStart == 0)
					return true;
				return Text::StrEndsWith(fileName, patternStart);
			}
			else if (patternStart)
			{
				return Text::StrCompareICase(fileName, patternStart) == 0;
			}
			else
			{
				return *fileName == 0;
			}
		}
		else if (c == '?' || c == '*')
		{
			if (isWC)
			{
				if (patternStart == 0)
					return false;
				*currPattern = 0;
				if ((i = Text::StrIndexOf(fileName, patternStart)) == INVALID_INDEX)
					return false;
				fileName += i + currPattern - patternStart;
				patternStart = 0;
				isWC = false;
			}
			else if (patternStart)
			{
				*currPattern = 0;
				if (!Text::StrStartsWithICase(fileName, patternStart))
					return false;
				fileName += currPattern - patternStart;
				patternStart = 0;
				isWC = false;
			}
			if (c == '?')
			{
				if (*fileName == 0)
					return false;
				fileName++;
				currPattern++;
			}
			else
			{
				isWC = true;
				patternStart = 0;
				currPattern++;
			}
		}
		else
		{
			if (patternStart == 0)
			{
				patternStart = currPattern;
			}
			currPattern++;
		}
	}

}

UInt64 IO::Path::GetFileSizeW(const WChar *path)
{
	return 0;
}

WChar *IO::Path::GetSystemProgramPathW(WChar *buff)
{
	return 0;
}

WChar *IO::Path::GetLocAppDataPathW(WChar *buff)
{
	return 0;
}

WChar *IO::Path::GetOSPathW(WChar *buff)
{
	return 0;
}


Bool IO::Path::GetFileTime(const UTF8Char *path, Data::DateTime *modTime, Data::DateTime *createTime, Data::DateTime *accessTime)
{
	return false;
}

WChar *IO::Path::GetCurrDirectoryW(WChar *buff)
{
	return 0;
}

Bool IO::Path::SetCurrDirectoryW(const WChar *path)
{
	return 0;
}

Bool IO::Path::IsSearchPattern(const UTF8Char *path)
{
	Bool isSrch = false;
	UTF8Char c;
	while ((c = *path++) != 0)
	{
		if (c == '*' || c == '?')
		{
			isSrch = true;
			break;
		}
	}
	return isSrch;
}
