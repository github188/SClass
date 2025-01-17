#include "Stdafx.h"
#include "MyMemory.h"
#include "Text/MyString.h"
#include "Text/MyStringW.h"
#include "UI/FolderDialog.h"
#include <windows.h>
#include <shlobj.h>

#define MAXFILENAMESIZE 512

/*		IO::Registry *reg;
		WChar *dialogName;
		WChar *lastName;
		WChar *fileName;
		Data::ArrayList *patterns;
		Data::ArrayList *names;*/

Int32 __stdcall UI::FolderDialog::BrowseCB(void *hwnd, UInt32 uMsg, OSInt lParam, OSInt lpData)
{
	UI::FolderDialog *me = (UI::FolderDialog*)lpData;
	switch (uMsg)
	{
	case BFFM_INITIALIZED:
		if (me->dirName)
		{
			SendMessage((HWND)hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)me->dirName);
		}
		break;
	case BFFM_VALIDATEFAILED:
		return 1;
	}
	return 0;
}

UI::FolderDialog::FolderDialog(const WChar *compName, const WChar *appName, const WChar *dialogName)
{
	this->reg = 0;
	this->dirName = 0;
	this->message = 0;
	CoInitializeEx(0, COINIT_APARTMENTTHREADED);

/*	OSInt i;
	WChar buff[256];
	WChar *sptr;
	this->reg = IO::Registry::OpenSoftware(IO::Registry::REG_USER_THIS, compName, appName);
	this->isSave = isSave;
	i = Text::StrCharCnt(dialogName);
	this->dialogName = MemAlloc(WChar, i + 7);
	sptr = Text::StrConcat(this->dialogName, dialogName);
	sptr = Text::StrConcat(sptr, L"Dialog");

	this->fileName = 0;
	this->lastName = 0;
	sptr = this->reg->GetValueStr(this->dialogName, buff);
	if (sptr)
	{
		this->lastName = Text::StrCopyNew(buff);
	}
	NEW_CLASS(this->names, Data::ArrayListStr());
	NEW_CLASS(this->patterns, Data::ArrayListStr());*/
}

UI::FolderDialog::~FolderDialog()
{
//	IO::Registry::CloseRegistry(this->reg);
	SDEL_TEXT(this->dirName);
	SDEL_TEXT(this->message);
	CoUninitialize();
}

void UI::FolderDialog::SetFolder(const UTF8Char *dirName)
{
	SDEL_TEXT(this->dirName);
	this->dirName = Text::StrCopyNew(dirName);
}

const UTF8Char *UI::FolderDialog::GetFolder()
{
	return this->dirName;
}

void UI::FolderDialog::SetMessage(const UTF8Char *message)
{
	SDEL_TEXT(this->message);
	this->message = Text::StrCopyNew(message);
}

Bool UI::FolderDialog::ShowDialog(void *ownerHandle)
{
	WChar sbuff[MAX_PATH];
	BROWSEINFOW info;
	info.hwndOwner = (HWND)ownerHandle;
	info.pidlRoot = 0;
	if (this->dirName)
	{
		Text::StrUTF8_WChar(sbuff, this->dirName, 0);
	}
	else
	{
		sbuff[0] = 0;
	}
	info.pszDisplayName = sbuff;
	const WChar *wptr = 0;
	if (this->message)
	{
		wptr = Text::StrToWCharNew(this->message);
		info.lpszTitle = wptr;
	}
	else
	{
		info.lpszTitle = L"";
	}
	info.ulFlags = BIF_USENEWUI | BIF_EDITBOX;
	info.lpfn = (BFFCALLBACK)BrowseCB;
	info.lParam = (LPARAM)this;
	info.iImage = 0;

	PIDLIST_ABSOLUTE idList = SHBrowseForFolderW(&info);
	SDEL_TEXT(wptr);
	if (info.pidlRoot)
	{
		CoTaskMemFree((LPVOID)info.pidlRoot);
	}

	if (idList == 0)
	{
		return false;
	}
	else
	{
		SHGetPathFromIDListW(idList, sbuff);
		CoTaskMemFree(idList);
		if (this->dirName)
		{
			Text::StrDelNew(this->dirName);
			this->dirName = 0;
		}
		this->dirName = Text::StrToUTF8New(sbuff);
		return true;
	}
}
