#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ArrayList.h"
#include "UI/GUICoreWin.h"
#include "UI/GUITrackBar.h"

#include <windows.h>
#include <commctrl.h>

void UI::GUITrackBar::EventScrolled()
{
	UOSInt i;
	Int32 pos;
	pos = (Int32)SendMessage((HWND)this->hwnd, TBM_GETPOS, 0, 0);
	i = this->scrollHandlers->GetCount();
	while (i-- > 0)
	{
		this->scrollHandlers->GetItem(i)(this->scrollHandlersObj->GetItem(i), pos);
	}
}

UI::GUITrackBar::GUITrackBar(UI::GUICore *ui, UI::GUIClientControl *parent, Int32 minVal, Int32 maxVal, Int32 currVal) : UI::GUIControl(ui, parent)
{
    INITCOMMONCONTROLSEX icex;

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

	NEW_CLASS(this->scrollHandlers, Data::ArrayList<ScrollEvent>());
	NEW_CLASS(this->scrollHandlersObj, Data::ArrayList<void *>());

	UInt32 style = WS_CHILD | TBS_NOTICKS;
	if (parent->IsChildVisible())
	{
		style = style | WS_VISIBLE;
	}
	this->InitControl(((UI::GUICoreWin*)ui)->GetHInst(), parent, TRACKBAR_CLASSW, (const UTF8Char*)"", style, 0, 0, 0, 200, 24);
	SendMessage((HWND)this->hwnd, TBM_SETRANGEMIN, 0, minVal);
	SendMessage((HWND)this->hwnd, TBM_SETRANGEMAX, 0, maxVal);
	SendMessage((HWND)this->hwnd, TBM_SETPOS, 0, currVal);
}

UI::GUITrackBar::~GUITrackBar()
{
	DEL_CLASS(this->scrollHandlersObj);
	DEL_CLASS(this->scrollHandlers);
}

const UTF8Char *UI::GUITrackBar::GetObjectClass()
{
	return (const UTF8Char*)"TrackBar";
}

OSInt UI::GUITrackBar::OnNotify(UInt32 code, void *lParam)
{
	switch (code)
	{
	case WM_HSCROLL:
	case WM_VSCROLL:
		break;
	case NM_CUSTOMDRAW:
		break;
	case NM_RELEASEDCAPTURE:
		EventScrolled();
		break;
	case TB_PAGEUP:
		break;
	default:
		EventScrolled();
		break;
	}
	return 0;
}

void UI::GUITrackBar::SetPos(Int32 pos)
{
	SendMessage((HWND)this->hwnd, TBM_SETPOS, TRUE, pos);
	EventScrolled();
//	InvalidateRect((HWND)this->hwnd, 0, false);
}

void UI::GUITrackBar::SetRange(Int32 minVal, Int32 maxVal)
{
	SendMessage((HWND)this->hwnd, TBM_SETRANGEMIN, 0, minVal);
	SendMessage((HWND)this->hwnd, TBM_SETRANGEMAX, 0, maxVal);
}

Int32 UI::GUITrackBar::GetPos()
{
	return (Int32)SendMessage((HWND)this->hwnd, TBM_GETPOS, 0, 0);
}

void UI::GUITrackBar::HandleScrolled(ScrollEvent hdlr, void *userObj)
{
	this->scrollHandlers->Add(hdlr);
	this->scrollHandlersObj->Add(userObj);
}
