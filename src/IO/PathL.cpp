#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/DateTime.h"
#include "IO/Path.h"
#include "Text/MyString.h"
#include "Text/MyStringW.h"
#include "Text/StringBuilderUTF8.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(__FreeBSD__)
#include <limits.h>
#include <sys/sysctl.h>
#endif

#if !defined(PATH_MAX)
#define PATH_MAX 512
#endif

typedef struct
{
	const UTF8Char *searchPattern;
	const UTF8Char *searchDir;
	DIR *dirObj;
} FindFileSession;

Char IO::Path::PATH_SEPERATOR = '/';
const UTF8Char *IO::Path::ALL_FILES = (const UTF8Char*)"*";

UTF8Char *IO::Path::GetTempFile(UTF8Char *buff, const UTF8Char *fileName)
{
	return Text::StrConcat(Text::StrConcat(buff, (const UTF8Char*)"/tmp/"), fileName);
}

WChar *IO::Path::GetTempFileW(WChar *buff, const WChar *fileName)
{
	return Text::StrConcat(Text::StrConcat(buff, L"/tmp/"), fileName);
}

Bool IO::Path::IsDirectoryExist(const UTF8Char *dir)
{
	return GetPathType(dir) == PT_DIRECTORY;
}

Bool IO::Path::IsDirectoryExistW(const WChar *dir)
{
	return GetPathTypeW(dir) == PT_DIRECTORY;
}

Bool IO::Path::CreateDirectory(const UTF8Char *dirInput)
{
	OSInt i = Text::StrLastIndexOf(dirInput, '/');
	if (i > 0)
	{
		Text::StringBuilderUTF8 sb;
		sb.AppendC(dirInput, i);
		if (GetPathType(sb.ToString()) == PT_UNKNOWN)
		{
			CreateDirectory(sb.ToString());
		}
	}
	int status = mkdir((const Char*)dirInput, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	return status == 0;
}

Bool IO::Path::CreateDirectoryW(const WChar *dirInput)
{
	OSInt i = Text::StrLastIndexOf(dirInput, '/');
	if (i > 0)
	{
		const WChar *wptr = Text::StrCopyNewC(dirInput, i);
		const UTF8Char *csptr = Text::StrToUTF8New(wptr);
		if (GetPathType(csptr) == PT_UNKNOWN)
		{
			CreateDirectory(csptr);
		}
		Text::StrDelNew(csptr);
		Text::StrDelNew(wptr);
	}
	const UTF8Char *utfPath = Text::StrToUTF8New(dirInput);
	int status = mkdir((const Char*)utfPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	Text::StrDelNew(utfPath);
	return status == 0;
}

Bool IO::Path::RemoveDirectory(const UTF8Char *dir)
{
	int status = rmdir((const Char*)dir);
	return status == 0;
}

Bool IO::Path::RemoveDirectoryW(const WChar *dir)
{
	const UTF8Char *utfPath = Text::StrToUTF8New(dir);
	int status = rmdir((const Char*)utfPath);
	Text::StrDelNew(utfPath);
	return status == 0;
}

Bool IO::Path::DeleteFile(const UTF8Char *fileName)
{
	int status = unlink((const Char*)fileName);
	return status == 0;
}

Bool IO::Path::DeleteFileW(const WChar *fileName)
{
	const UTF8Char *utfPath = Text::StrToUTF8New(fileName);
	int status = unlink((const Char*)utfPath);
	Text::StrDelNew(utfPath);
	return status == 0;
}

OSInt IO::Path::FileNameCompare(const UTF8Char *file1, const UTF8Char *file2)
{
	return Text::StrCompare(file1, file2);
}

OSInt IO::Path::FileNameCompareW(const WChar *file1, const WChar *file2)
{
	return Text::StrCompare(file1, file2);
}

UTF8Char *IO::Path::GetFileDirectory(UTF8Char *buff, const UTF8Char *fileName)
{
	return Text::StrConcat(buff, fileName);
}

WChar *IO::Path::GetFileDirectoryW(WChar *buff, const WChar *fileName)
{
	return Text::StrConcat(buff, fileName);
}

UTF8Char *IO::Path::GetProcessFileName(UTF8Char *buff)
{
#if defined(__FreeBSD__)
	int mib[4];
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PATHNAME;
	mib[3] = -1;
	size_t size = 512;
	if (sysctl(mib, 4, (Char*)buff, &size, 0, 0) != 0)
		return 0;
#else
	ssize_t size = readlink("/proc/self/exe", (Char*)buff, 1024);
	if (size == -1)
		return 0;
#endif
	buff[size] = 0;
	return &buff[size];
}

WChar *IO::Path::GetProcessFileNameW(WChar *buff)
{
	Char sbuff[1024];
#if defined(__FreeBSD__)
	int mib[4];
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PATHNAME;
	mib[3] = -1;
	size_t size = sizeof(sbuff);
	if (sysctl(mib, 4, sbuff, &size, 0, 0) != 0)
		return 0;
#else
	ssize_t size = readlink("/proc/self/exe", sbuff, 1024);
	if (size == -1)
		return 0;
#endif
	return Text::StrUTF8_WChar(buff, (UInt8*)sbuff, size, 0);
}

Bool IO::Path::GetProcessFileName(Text::StringBuilderUTF *sb)
{
	Char sbuff[512];
	ssize_t size = readlink("/proc/self/exe", sbuff, 512);
	if (size == -1)
		return 0;
	sb->AppendC((const UTF8Char*)sbuff, size);
	return true;
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

UTF8Char *IO::Path::GetFileExt(UTF8Char *fileBuff, const UTF8Char *path)
{
	OSInt i = Text::StrLastIndexOf(path, '/');
	if (i >= 0)
	{
		path = &path[i + 1];
	}
	i = Text::StrLastIndexOf(path, '.');
	if (i >= 0)
	{
		return Text::StrConcat(fileBuff, &path[i + 1]);
	}
	else
	{
		return Text::StrConcat(fileBuff, (const UTF8Char*)"");
	}
}

WChar *IO::Path::GetFileExtW(WChar *fileBuff, const WChar *path)
{
	OSInt i = Text::StrLastIndexOf(path, '/');
	if (i >= 0)
	{
		path = &path[i + 1];
	}
	i = Text::StrLastIndexOf(path, '.');
	if (i >= 0)
	{
		return Text::StrConcat(fileBuff, &path[i + 1]);
	}
	else
	{
		return Text::StrConcat(fileBuff, L"");
	}
}

UTF8Char *IO::Path::AppendPath(UTF8Char *path, const UTF8Char *toAppend)
{
	if (toAppend[0] == '/')
		return Text::StrConcat(path, toAppend);
	OSInt i = Text::StrLastIndexOf(path, '/');
	OSInt j = Text::StrCharCnt(path);
	IO::Path::PathType pt = GetPathType(path);
	if (pt == PT_FILE && i >= 0)
	{
		path[i] = 0;
		i = Text::StrLastIndexOf(path, '/');
	}
	else if (i == j - 1)
	{
		path[i] = 0;
		i = Text::StrLastIndexOf(path, '/');
	}
	while (Text::StrStartsWith(toAppend, (const UTF8Char*)"../"))
	{
		if (i != -1)
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

WChar *IO::Path::AppendPathW(WChar *path, const WChar *toAppend)
{
	if (toAppend[0] == '/')
		return Text::StrConcat(path, toAppend);
	OSInt i = Text::StrLastIndexOf(path, '/');
	if (GetPathTypeW(path) == PT_FILE && i >= 0)
	{
		path[i] = 0;
		i = Text::StrLastIndexOf(path, '/');
	}
	while (Text::StrStartsWith(toAppend, L"../"))
	{
		if (i != -1)
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
	OSInt i = Text::StrLastIndexOf(sb->ToString(), '/');
	if (GetPathType(sb->ToString()) == PT_FILE && i >= 0)
	{
		sb->RemoveChars(sb->GetLength() - i);
		i = Text::StrLastIndexOf(sb->ToString(), '/');
	}
	else if (i == (OSInt)sb->GetCharCnt() - 1)
	{
		sb->RemoveChars(1);
		i = Text::StrLastIndexOf(sb->ToString(), '/');
	}
	while (Text::StrStartsWith(toAppend, (const UTF8Char*)"../"))
	{
		if (i != -1)
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

void *IO::Path::FindFile(const UTF8Char *utfPath)
{
	FindFileSession *sess = 0;
	const UTF8Char *searchPattern;
	const UTF8Char *searchDir;
	OSInt i = Text::StrLastIndexOf(utfPath, '/');
	DIR *dirObj;
	if (i == -1)
	{
		dirObj = opendir(".");
		searchPattern = utfPath;
		searchDir = Text::StrCopyNew((const UTF8Char*)"./");
	}
	else if (i == 0)
	{
		dirObj = opendir("/");
		searchPattern = utfPath + 1;
		searchDir = Text::StrCopyNew((const UTF8Char*)"/");
	}
	else
	{
		UTF8Char *tmpBuff = MemAlloc(UTF8Char, i + 2);
		Text::StrConcatC(tmpBuff, utfPath, i);
		dirObj = opendir((const Char*)tmpBuff);
		searchPattern = utfPath + i + 1;
		tmpBuff[i + 1] = 0;
		tmpBuff[i] = '/';
		searchDir = Text::StrCopyNew(tmpBuff);
		MemFree(tmpBuff);
	}
	if (dirObj)
	{
		sess = MemAlloc(FindFileSession, 1);
		sess->searchPattern = Text::StrCopyNew(searchPattern);
		sess->dirObj = dirObj;
		sess->searchDir = searchDir;
	}
	else
	{
		Text::StrDelNew(searchDir);
	}
	return sess;
}

void *IO::Path::FindFileW(const WChar *path)
{
	FindFileSession *sess = 0;
	const UTF8Char *utfPath = Text::StrToUTF8New(path);
	const UTF8Char *searchPattern;
	const UTF8Char *searchDir;
	OSInt i = Text::StrLastIndexOf(utfPath, '/');
	DIR *dirObj;
	if (i == -1)
	{
		dirObj = opendir(".");
		searchPattern = utfPath;
		searchDir = Text::StrCopyNew((const UTF8Char*)"./");
	}
	else if (i == 0)
	{
		dirObj = opendir("/");
		searchPattern = utfPath + 1;
		searchDir = Text::StrCopyNew((const UTF8Char*)"/");
	}
	else
	{
		*(UTF8Char*)&utfPath[i] = 0;
		dirObj = opendir((const Char*)utfPath);
		searchPattern = utfPath + i + 1;
		Char c = utfPath[i + 1];
		*(UTF8Char*)&utfPath[i + 1] = 0;
		*(UTF8Char*)&utfPath[i] = '/';
		searchDir = Text::StrCopyNew(utfPath);
		*(UTF8Char*)&utfPath[i + 1] = c;
	}
	if (dirObj)
	{
		sess = MemAlloc(FindFileSession, 1);
		sess->searchPattern = Text::StrCopyNew(searchPattern);
		sess->dirObj = dirObj;
		sess->searchDir = searchDir;
	}
	else
	{
		Text::StrDelNew(searchDir);
	}
	Text::StrDelNew(utfPath);
	return sess;
}

UTF8Char *IO::Path::FindNextFile(UTF8Char *buff, void *session, Data::DateTime *modTime, IO::Path::PathType *pt, Int64 *fileSize)
{
	FindFileSession *sess = (FindFileSession*)session;
	struct dirent *ent;
	while ((ent = readdir(sess->dirObj)) != 0)
	{
		if (FileNameMatch((const UTF8Char*)ent->d_name, sess->searchPattern))
		{
			Text::StringBuilderUTF8 sb;
			sb.Append(sess->searchDir);
			sb.Append((const UTF8Char*)ent->d_name);
#if defined(__USE_LARGEFILE64)
			struct stat64 s;
			int status = lstat64((const Char*)sb.ToString(), &s);
#else
			struct stat s;
			int status = lstat((const Char*)sb.ToString(), &s);
#endif
			if (status == 0)
			{
				if (modTime)
				{
					modTime->SetUnixTimestamp(s.st_mtime);
				}
				if (pt)
				{
					if (S_ISREG(s.st_mode))
						*pt = PT_FILE;
					else if (S_ISDIR(s.st_mode))
						*pt = PT_DIRECTORY;
					else
						*pt = PT_UNKNOWN;
				}
				if (fileSize)
				{
					*fileSize = s.st_size;
				}
				return Text::StrConcat(buff, (const UTF8Char*)ent->d_name);
			}
		}
		
	}
	return 0;
}

WChar *IO::Path::FindNextFileW(WChar *buff, void *session, Data::DateTime *modTime, IO::Path::PathType *pt, Int64 *fileSize)
{
	FindFileSession *sess = (FindFileSession*)session;
	struct dirent *ent;
	while ((ent = readdir(sess->dirObj)) != 0)
	{
		if (FileNameMatch((const UTF8Char*)ent->d_name, sess->searchPattern))
		{
			Text::StringBuilderUTF8 sb;
			sb.Append(sess->searchDir);
			sb.Append((const UTF8Char*)ent->d_name);
#if defined(__USE_LARGEFILE64)
			struct stat64 s;
			int status = lstat64((const Char*)sb.ToString(), &s);
#else
			struct stat s;
			int status = lstat((const Char*)sb.ToString(), &s);
#endif
			if (status == 0)
			{
				if (modTime)
				{
					modTime->SetUnixTimestamp(s.st_mtime);
				}
				if (pt)
				{
					if (S_ISREG(s.st_mode))
						*pt = PT_FILE;
					else if (S_ISDIR(s.st_mode))
						*pt = PT_DIRECTORY;
					else
						*pt = PT_UNKNOWN;
				}
				if (fileSize)
				{
					*fileSize = s.st_size;
				}
				return Text::StrUTF8_WChar(buff, (const UTF8Char*)ent->d_name, -1, 0);
			}
		}
	}
	return 0;
}

void IO::Path::FindFileClose(void *session)
{
	FindFileSession *sess = (FindFileSession*)session;
	Text::StrDelNew(sess->searchPattern);
	Text::StrDelNew(sess->searchDir);
	closedir(sess->dirObj);
	MemFree(sess);
}

IO::Path::PathType IO::Path::GetPathType(const UTF8Char *path)
{
#if defined(__USE_LARGEFILE64)
	struct stat64 s;
	int status = lstat64((const Char*)path, &s);
#else
	struct stat s;
	int status = lstat((const Char*)path, &s);
#endif
	if (status != 0)
	{
		return PT_UNKNOWN;
	}
	if (S_ISREG(s.st_mode))
	{
		return PT_FILE;
	}
	else if (S_ISDIR(s.st_mode))
	{
		return PT_DIRECTORY;
	}
	else if (S_ISLNK(s.st_mode))
	{
		UTF8Char pathBuff[512];
		Char cbuff[512];
		ssize_t size = readlink((const Char*)path, cbuff, 511);
		cbuff[size] = 0;
		Text::StrConcat(pathBuff, path);
		OSInt i = Text::StrLastIndexOf(pathBuff, '/');
		if (i >= 0)
		{
			pathBuff[i + 1] = 0;
			IO::Path::AppendPath(pathBuff, (const UTF8Char*)cbuff);
		}
		return GetPathType((const UTF8Char*)pathBuff);
	}
	return PT_UNKNOWN;
}

IO::Path::PathType IO::Path::GetPathTypeW(const WChar *path)
{
	const UTF8Char *utfPath = Text::StrToUTF8New(path);
	IO::Path::PathType pt = IO::Path::GetPathType(utfPath);
	Text::StrDelNew(utfPath);
	return pt;
}

WChar *IO::Path::GetFullPathW(WChar *buff, const WChar *path)
{
	Text::StringBuilderUTF8 sb;
	const UTF8Char *csptr = Text::StrToUTF8New(path);
	sb.Append(csptr);
	Text::StrDelNew(csptr);
	sb.AllocLeng(512);
	Char cbuff[512];
	UTF8Char *u8ptr = sb.ToString();
	int status;
	OSInt i;
	OSInt j;
//	printf("GetFullPath %ls\r\n", path);
#if defined(__USE_LARGEFILE64)
	struct stat64 s;
	status = lstat64((const Char*)u8ptr, &s);
#else
	struct stat s;
	status = lstat((const Char*)u8ptr, &s);
#endif
	if (status != 0)
	{
		i = Text::StrLastIndexOf(u8ptr, '/');
		while (true)
		{
//			printf("GetFullPath: Loop i = %d\r\n", i);
			if (i < 0)
			{
				return Text::StrUTF8_WChar(buff, u8ptr, -1, 0);
			}
			u8ptr[i] = 0;
#if defined(__USE_LARGEFILE64)
			status = lstat64((const Char*)u8ptr, &s);
#else
			status = lstat((const Char*)u8ptr, &s);
#endif
			if (status == 0)
			{
				if (S_ISLNK(s.st_mode))
				{
					ssize_t size = readlink((const Char*)u8ptr, cbuff, 511);
					cbuff[size] = 0;
//					printf("readlink %s -> %s\r\n", u8ptr, cbuff);
					//////////////////////////////
					return Text::StrUTF8_WChar(buff, u8ptr, -1, 0);
				}
				else
				{
					u8ptr[i] = '/';
					return Text::StrUTF8_WChar(buff, u8ptr, -1, 0);
				}
			}
			else
			{
				j = i;
				i = Text::StrLastIndexOf(u8ptr, '/');
				u8ptr[j] = '/';
			}
		}
		
	}
	else if (S_ISREG(s.st_mode))
	{
		return Text::StrConcat(buff, path);
	}
	else if (S_ISDIR(s.st_mode))
	{
		return Text::StrConcat(buff, path);
	}
	else if (S_ISLNK(s.st_mode))
	{
		ssize_t size = readlink((const Char*)u8ptr, cbuff, 511);
		cbuff[size] = 0;
		Text::StrConcat(buff, path);

		const WChar *wptr = Text::StrToWCharNew((const UTF8Char*)cbuff);
		WChar *ret = IO::Path::AppendPathW(buff, wptr);
		Text::StrDelNew(wptr);
		return ret;
	}
	else
	{
		////////////////////////////////
		return Text::StrConcat(buff, path);
	}
}

Bool IO::Path::FileNameMatch(const UTF8Char *path, const UTF8Char *searchPattern)
{
	UTF8Char sbuff[256];
	OSInt i = Text::StrLastIndexOf(path, '/');
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
				if ((i = Text::StrIndexOf(fileName, patternStart)) < 0)
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

Bool IO::Path::FileNameMatchW(const WChar *path, const WChar *searchPattern)
{
	WChar sbuff[256];
	OSInt i = Text::StrLastIndexOf(path, '/');
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
				if ((i = Text::StrIndexOf(fileName, patternStart)) < 0)
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

Int64 IO::Path::GetFileSize(const UTF8Char *path)
{
#if defined(__USE_LARGEFILE64)
	struct stat64 s;
	int status = lstat64((const Char*)path, &s);
#else
	struct stat s;
	int status = lstat((const Char*)path, &s);
#endif
	if (status != 0)
		return 0;
	return s.st_size;
}

Int64 IO::Path::GetFileSizeW(const WChar *path)
{
	const UTF8Char *utfPath = Text::StrToUTF8New(path);
#if defined(__USE_LARGEFILE64)
	struct stat64 s;
	int status = lstat64((const Char*)utfPath, &s);
#else
	struct stat s;
	int status = lstat((const Char*)utfPath, &s);
#endif
	Text::StrDelNew(utfPath);
	if (status != 0)
		return 0;
	return s.st_size;
}

WChar *IO::Path::GetSystemProgramPathW(WChar *buff)
{
/////////////////////////////////
	return 0;
}

UTF8Char *IO::Path::GetLocAppDataPath(UTF8Char *buff)
{
/////////////////////////////////
	return 0;
}

WChar *IO::Path::GetLocAppDataPathW(WChar *buff)
{
/////////////////////////////////
	return 0;
}

WChar *IO::Path::GetOSPathW(WChar *buff)
{
	return Text::StrConcat(buff, L"/");
}


Bool IO::Path::GetFileTime(const UTF8Char *path, Data::DateTime *modTime, Data::DateTime *createTime, Data::DateTime *accessTime)
{
#if defined(__USE_LARGEFILE64)
	struct stat64 s;
	int status = lstat64((const Char*)path, &s);
#else
	struct stat s;
	int status = lstat((const Char*)path, &s);
#endif
	if (status != 0)
		return false;
	if (modTime)
	{
		modTime->SetUnixTimestamp(s.st_mtime);
	}
	if (createTime)
	{
		createTime->SetUnixTimestamp(s.st_ctime);
	}
	if (accessTime)
	{
		accessTime->SetUnixTimestamp(s.st_atime);
	}
	return true;
}

UTF8Char *IO::Path::GetCurrDirectory(UTF8Char *buff)
{
	Char cbuff[PATH_MAX];
	if (getcwd(cbuff, PATH_MAX) == 0)
		return 0;
	return Text::StrConcat(buff, (const UTF8Char*)cbuff);
}

WChar *IO::Path::GetCurrDirectoryW(WChar *buff)
{
	Char cbuff[PATH_MAX];
	if (getcwd(cbuff, PATH_MAX) == 0)
		return 0;
	return Text::StrUTF8_WChar(buff, (const UTF8Char*)cbuff, -1, 0);
}

Bool IO::Path::SetCurrDirectoryW(const WChar *path)
{
	const UTF8Char *utfPath = Text::StrToUTF8New(path);
	int status = chdir((const Char*)utfPath);
	Text::StrDelNew(utfPath);
	return status == 0;
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