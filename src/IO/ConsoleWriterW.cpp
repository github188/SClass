#include "Stdafx.h"
#include "MyMemory.h"
#include "IO/ConsoleWriter.h"
#include "IO/FileStream.h"
#include "Sync/Mutex.h"
#include "Sync/MutexUsage.h"
#include "Text/Encoding.h"
#include "Text/MyString.h"
#include "Text/MyStringW.h"

#include <windows.h>
#include <stdio.h>
#ifdef _WIN32_WCE
#define WriteConsoleW WriteFile
#define WriteConsoleA WriteFile
#endif

IO::ConsoleWriter::ConsoleWriter()
{
#ifdef _WIN32_WCE
	this->hand = CreateFile(L"CON0", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
#else
	this->hand = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
	NEW_CLASS(this->mut, Sync::Mutex());
	this->enc = 0;
	this->autoFlush = false;
	this->fileOutput = false;
	UInt32 mode;
#ifndef _WIN32_WCE
	BOOL ret = GetConsoleMode(this->hand, (LPDWORD)&mode);
	if (ret == 0)
	{
		fileOutput = true;
		NEW_CLASS(this->enc, Text::Encoding());
	}
#endif
}

IO::ConsoleWriter::~ConsoleWriter()
{
	if (this->enc)
	{
		DEL_CLASS(this->enc);
		this->enc = 0;
	}
#ifdef _WIN32_WCE
	CloseHandle(this->hand);
#endif
	DEL_CLASS(this->mut);
}
/*
Bool IO::ConsoleWriter::Write(const UTF8Char *str, UOSInt nChar)
{
	UOSInt strLen = Text::StrUTF8_WCharCnt(str, nChar);
	WChar *wstr = MemAlloc(WChar, strLen + 1);
	Text::StrUTF8_WChar(wstr, str, nChar, 0);
	wstr[strLen] = 0;
	Bool ret = this->WriteW(wstr);
	MemFree(wstr);
	return ret;
}

Bool IO::ConsoleWriter::Write(const UTF8Char *str)
{
	const WChar *wstr = Text::StrToWCharNew(str);
	Bool ret = this->WriteW(wstr);
	Text::StrDelNew(wstr);
	return ret;
}

Bool IO::ConsoleWriter::WriteLine(const UTF8Char *str, UOSInt nChar)
{
	UOSInt strLen = Text::StrUTF8_WCharCnt(str, nChar);
	WChar *wstr = MemAlloc(WChar, strLen + 1);
	Text::StrUTF8_WChar(wstr, str, nChar, 0);
	wstr[strLen] = 0;
	Bool ret = this->WriteLineW(wstr);
	MemFree(wstr);
	return ret;
}

Bool IO::ConsoleWriter::WriteLine(const UTF8Char *str)
{
	const WChar *wstr = Text::StrToWCharNew(str);
	Bool ret = this->WriteLineW(wstr);
	Text::StrDelNew(wstr);
	return ret;
}*/

Bool IO::ConsoleWriter::Write(const UTF8Char *s, UOSInt nUTF8Char)
{
	Bool ret;
	const UTF8Char *csptr = Text::StrCopyNewC(s, nUTF8Char);
	ret = this->Write(csptr);
	Text::StrDelNew(csptr);
	return ret;
}

Bool IO::ConsoleWriter::Write(const UTF8Char *s)
{
	UInt32 outChars = 0;
	UInt32 nChar;
	if (this->enc == 0)
	{
		const WChar *str = Text::StrToWCharNew(s);
		WriteConsoleW((HANDLE)this->hand, str, nChar = (UInt32)Text::StrCharCnt(str), (LPDWORD)&outChars, 0);
		Text::StrDelNew(str);
		if (outChars == nChar)
		{
			if (this->autoFlush)
			{
				FlushFileBuffers((HANDLE)this->hand);
			}
			return true;
		}
		return false;
	}
	else
	{
		UOSInt nBytes;
		UInt8 *tmpBuff;
		nChar = (UInt32)Text::StrCharCnt(s);
		nBytes = this->enc->UTF8CountBytesC(s, nChar);
		tmpBuff = MemAlloc(UInt8, nBytes + 1);
		this->enc->UTF8ToBytesC(tmpBuff, s, nChar);
		if (fileOutput)
		{
			WriteFile((HANDLE)this->hand, tmpBuff, (UInt32)nBytes, (LPDWORD)&outChars, 0);
		}
		else
		{
			WriteConsoleA((HANDLE)this->hand, tmpBuff, (UInt32)nBytes, (LPDWORD)&outChars, 0);
		}
		MemFree(tmpBuff);
		if (outChars == (UInt32)nBytes)
		{
			if (this->autoFlush)
			{
				FlushFileBuffers((HANDLE)this->hand);
			}
			return true;
		}
		return false;
	}
}

Bool IO::ConsoleWriter::WriteLine(const UTF8Char *s, UOSInt nUTF8Char)
{
	Bool ret;
	const UTF8Char *csptr = Text::StrCopyNewC(s, nUTF8Char);
	ret = this->WriteLine(csptr);
	Text::StrDelNew(csptr);
	return ret;
}

Bool IO::ConsoleWriter::WriteLine(const UTF8Char *s)
{
	UInt32 outChars = 0;
	UInt32 outChars2 = 0;
	UInt32 nChar;
	if (this->enc == 0)
	{
		const WChar *str = Text::StrToWCharNew(s);
		Sync::MutexUsage mutUsage(this->mut);
		WriteConsoleW((HANDLE)this->hand, str, nChar = (UInt32)Text::StrCharCnt(str), (LPDWORD)&outChars, 0);
		WriteConsoleW((HANDLE)this->hand, L"\n", 1, (LPDWORD)&outChars2, 0);
		mutUsage.EndUse();
		Text::StrDelNew(str);
		if (outChars == nChar && outChars2 == 1)
		{
			if (this->autoFlush)
			{
				FlushFileBuffers((HANDLE)this->hand);
			}
			return true;
		}
		return false;
	}
	else
	{
		UOSInt nBytes;
		UInt8 *tmpBuff;
		nChar = (UInt32)Text::StrCharCnt(s);
		nBytes = this->enc->UTF8CountBytesC(s, nChar);
		tmpBuff = MemAlloc(UInt8, nBytes + 2);
		this->enc->UTF8ToBytesC(tmpBuff, s, nChar);
		tmpBuff[nBytes++] = '\n';
		if (fileOutput)
		{
			WriteFile((HANDLE)this->hand, tmpBuff, (UInt32)nBytes, (LPDWORD)&outChars, 0);
		}
		else
		{
			WriteConsoleA((HANDLE)this->hand, tmpBuff, (UInt32)nBytes, (LPDWORD)&outChars, 0);
		}
		MemFree(tmpBuff);
		if (outChars == (UInt32)nBytes)
		{
			if (this->autoFlush)
			{
				FlushFileBuffers((HANDLE)this->hand);
			}
			return true;
		}
		return false;
	}
}

Bool IO::ConsoleWriter::WriteLine()
{
	UInt32 outChars = 0;
	if (fileOutput)
	{
		WriteFile((HANDLE)this->hand, "\n", 1, (LPDWORD)&outChars, 0);
	}
	else
	{
		WriteConsoleW((HANDLE)this->hand, L"\n", 1, (LPDWORD)&outChars, 0);
	}
	if (outChars == 1)
	{
		if (this->autoFlush)
		{
			FlushFileBuffers((HANDLE)this->hand);
		}
		return true;
	}
	return false;
}

void IO::ConsoleWriter::SetTextColor(IO::ConsoleWriter::ConsoleColor fgColor, IO::ConsoleWriter::ConsoleColor bgColor)
{
#ifndef _WIN32_WCE
	SetConsoleTextAttribute((HANDLE)this->hand, (UInt16)((fgColor & 0xf) | ((bgColor & 0xf) << 4)));
#endif
}

void IO::ConsoleWriter::ResetTextColor()
{
	SetTextColor(CC_GRAY, CC_BLACK);
}

UOSInt IO::ConsoleWriter::CalDisplaySize(const WChar *str)
{
	WChar c;
	UOSInt size = 0;
	while ((c = *str++) != 0)
	{
		if (c < 32)
		{
		}
		else if (c < 0x2000)
		{
			size += 1;
		}
		else if (c < 0xff60)
		{
			size += 2;
		}
		else if (c < 0xffA0)
		{
			size += 1;
		}
		else if (c < 0xffe8)
		{
			size += 2;
		}
		else
		{
			size += 1;
		}
	}
	return size;
}

WChar *IO::ConsoleWriter::ReadLine(WChar *sbuff, UOSInt nChar)
{
#if defined(__CYGWIN__)
	Char buff[512];
	if (fgets(buff, nChar, stdin) == 0)
		return 0;
	else
	{
		return Text::StrUTF8_WChar(sbuff, (UTF8Char*)buff, -1, 0);
	}
#else
	if (fgetws(sbuff, (int)nChar, stdin) == 0)
		return 0;
	else
		return &sbuff[Text::StrCharCnt(sbuff)];
#endif
}

void IO::ConsoleWriter::EnableCPFix(Bool isEnable)
{
#ifndef _WIN32_WCE
	if (isEnable || fileOutput)
	{
		if (this->enc == 0)
		{
			NEW_CLASS(this->enc, Text::Encoding(GetConsoleOutputCP()));
		}
	}
	else
	{
		if (this->enc)
		{
			DEL_CLASS(this->enc);
			this->enc = 0;
		}
	}
#endif
}

void IO::ConsoleWriter::SetAutoFlush(Bool autoFlush)
{
	this->autoFlush = autoFlush;
}

Bool IO::ConsoleWriter::GetConsoleState(IO::ConsoleWriter::ConsoleState *state)
{
#ifdef _WIN32_WCE
	return false;
#else
	CONSOLE_SCREEN_BUFFER_INFO info;
	if (GetConsoleScreenBufferInfo((HANDLE)this->hand, &info) == FALSE)
		return false;
	state->fgColor = (ConsoleColor)(info.wAttributes & 0xf);
	state->bgColor = (ConsoleColor)((info.wAttributes >> 4) & 0xf);
	state->currX = (UInt32)info.dwCursorPosition.X;
	state->currY = (UInt32)info.dwCursorPosition.Y;
	state->consoleWidth = (UInt32)info.dwSize.X;
	state->consoleHeight = (UInt32)info.dwSize.Y;
	return true;
#endif
}

Bool IO::ConsoleWriter::SetCursorPos(UInt32 x, Int32 y)
{
#ifdef _WIN32_WCE
	return false;
#else
	COORD tmp;
	tmp.X = (SHORT)x;
	tmp.Y = (SHORT)y;
	return SetConsoleCursorPosition((HANDLE)this->hand, tmp) != 0;
#endif
}

Bool IO::ConsoleWriter::IsFileOutput()
{
	return this->fileOutput;
}

void IO::ConsoleWriter::FixWrite(const WChar *str, UOSInt displayWidth)
{
	if (this->fileOutput || this->enc == 0)
	{
		return;
	}
	UOSInt width = GetDisplayWidth(str);
	if (width <= displayWidth)
	{
		const UTF8Char *csptr = Text::StrToUTF8New(str);
		this->Write(csptr);
		Text::StrDelNew(csptr);
		while (width < displayWidth)
		{
			width++;
			Write((const UTF8Char*)" ");
		}
		return;
	}

	if (displayWidth < 4)
	{
		WChar wbuff[2];
		width = 0;
		while (true)
		{
			if (width >= displayWidth)
				return ;
			width += GetDisplayCharWidth(*str);
			if (width > displayWidth)
			{
				Write((const UTF8Char*)" ", 1);
				return;
			}
			wbuff[0] = str[0];
			wbuff[1] = 0;
			const UTF8Char *csptr = Text::StrToUTF8New(wbuff);
			this->Write(csptr);
			Text::StrDelNew(csptr);
			str++;
		}
	}
	else
	{
		WChar wbuff[2];
		width = 0;
		while (width < displayWidth - 3)
		{
			width += GetDisplayCharWidth(*str);
			wbuff[0] = str[0];
			wbuff[1] = 0;
			const UTF8Char *csptr = Text::StrToUTF8New(wbuff);
			this->Write(csptr);
			Text::StrDelNew(csptr);
			str++;
		}
		while (width < displayWidth)
		{
			this->Write((const UTF8Char*)".");
			width++;
		}
	}
}

UOSInt IO::ConsoleWriter::GetDisplayWidth(const WChar *str)
{
	if (this->fileOutput || this->enc == 0)
	{
		return Text::StrCharCnt(str);
	}
	UOSInt size = 0;
	WChar c;
	while ((c = *str++) != 0)
	{
		if (c >= 32)
			size += GetDisplayCharWidth(c);
	}
	return size;
}

UOSInt IO::ConsoleWriter::GetDisplayCharWidth(WChar c)
{
	UInt8 buff[4];
	UOSInt size = this->enc->WToBytesC(buff, &c, 1);
	if (size == 1 && buff[0] < 128)
		return 1;

#ifdef _WIN32_WCE
	return 2;
#else
	CONSOLE_SCREEN_BUFFER_INFO info;
	if (GetConsoleScreenBufferInfo((HANDLE)this->hand, &info) == FALSE)
		return 1;
	CONSOLE_SCREEN_BUFFER_INFO info2;
	COORD tmp = {0, 0};
	SetConsoleCursorPosition((HANDLE)this->hand, tmp);
	DWORD nChar;
	WriteConsoleA((HANDLE)this->hand, buff, (DWORD)size, &nChar, 0);
	if (GetConsoleScreenBufferInfo((HANDLE)this->hand, &info2) == FALSE)
	{
		size = 1;
	}
	else
	{
		size = (UOSInt)info2.dwCursorPosition.X;
	}
	SetConsoleCursorPosition((HANDLE)this->hand, info.dwCursorPosition);
	return size;
#endif
}
