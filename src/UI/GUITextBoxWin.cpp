#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ArrayList.h"
#include "Text/MyString.h"
#include "Text/MyStringW.h"
#include "UI/GUICoreWin.h"
#include "UI/GUITextBox.h"

#include <windows.h>
#ifndef _WIN32_WCE
#include <richedit.h>
#endif

UI::GUITextBox::GUITextBox(UI::GUICore *ui, UI::GUIClientControl *parent, const UTF8Char *initText) : UI::GUIControl(ui, parent)
{
	NEW_CLASS(this->txtChgHdlrs, Data::ArrayList<UI::UIEvent>());
	NEW_CLASS(this->txtChgObjs, Data::ArrayList<void*>());

	UInt32 style = WS_BORDER | WS_TABSTOP | WS_CHILD | ES_AUTOHSCROLL;
	if (parent->IsChildVisible())
	{
		style = style | WS_VISIBLE;
	}
	this->InitControl(((UI::GUICoreWin*)ui)->GetHInst(), parent, L"EDIT", initText, style, WS_EX_CLIENTEDGE, 0, 0, 200, 28);
}

UI::GUITextBox::GUITextBox(UI::GUICore *ui, UI::GUIClientControl *parent, const UTF8Char *initText, Bool isMultiline) : UI::GUIControl(ui, parent)
{
	NEW_CLASS(this->txtChgHdlrs, Data::ArrayList<UI::UIEvent>());
	NEW_CLASS(this->txtChgObjs, Data::ArrayList<void*>());

	UInt32 style = WS_BORDER | WS_TABSTOP | WS_CHILD | ES_AUTOHSCROLL;
	if (parent->IsChildVisible())
	{
		style = style | WS_VISIBLE;
	}
	if (isMultiline)
	{
		style |= ES_WANTRETURN | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL;
	}
	this->InitControl(((UI::GUICoreWin*)ui)->GetHInst(), parent, L"EDIT", initText, style, WS_EX_CLIENTEDGE, 0, 0, 200, 28);
}

UI::GUITextBox::~GUITextBox()
{
	DEL_CLASS(this->txtChgObjs);
	DEL_CLASS(this->txtChgHdlrs);
}

void UI::GUITextBox::EventTextChange()
{
	UOSInt i = this->txtChgHdlrs->GetCount();
	while (i-- > 0)
	{
		this->txtChgHdlrs->GetItem(i)(this->txtChgObjs->GetItem(i));
	}
}

void UI::GUITextBox::SetReadOnly(Bool isReadOnly)
{
	SendMessage((HWND)this->hwnd, EM_SETREADONLY, isReadOnly?TRUE:FALSE, 0);
}

void UI::GUITextBox::SetPasswordChar(UTF32Char c)
{
	SendMessage((HWND)this->hwnd, EM_SETPASSWORDCHAR, c, 0);
}

void UI::GUITextBox::SetText(const UTF8Char *txt)
{
	const WChar *wptr = Text::StrToWCharNew(txt);
	SetWindowTextW((HWND)hwnd, wptr);
	Text::StrDelNew(wptr);
}

UTF8Char *UI::GUITextBox::GetText(UTF8Char *buff)
{
	UOSInt leng = (UOSInt)GetWindowTextLengthW((HWND)hwnd);
	WChar *sptr = MemAlloc(WChar, leng + 1);
	GetWindowTextW((HWND)hwnd, sptr, (int)(leng + 1));
	buff = Text::StrWChar_UTF8(buff, sptr);
	MemFree(sptr);
	return buff;
}

Bool UI::GUITextBox::GetText(Text::StringBuilderUTF *sb)
{
	UOSInt leng = (UOSInt)GetWindowTextLengthW((HWND)hwnd);
	WChar *wptr = MemAlloc(WChar, leng + 1);
	GetWindowTextW((HWND)hwnd, wptr, (int)leng + 1);
	const UTF8Char *csptr = Text::StrToUTF8New(wptr);
	sb->Append(csptr);
	Text::StrDelNew(csptr);
	MemFree(wptr);
	return true;
}

const UTF8Char *UI::GUITextBox::GetObjectClass()
{
	return (const UTF8Char*)"TextBox";
}

OSInt UI::GUITextBox::OnNotify(UInt32 code, void *lParam)
{
	switch (code)
	{
	case EN_CHANGE:
		EventTextChange();
		break;
	}
	return 0;
}

void UI::GUITextBox::HandleTextChanged(UI::UIEvent hdlr, void *userObj)
{
	this->txtChgHdlrs->Add(hdlr);
	this->txtChgObjs->Add(userObj);
}

void UI::GUITextBox::SelectAll()
{
	SendMessage((HWND)this->hwnd, EM_SETSEL, 0, -1);
}
