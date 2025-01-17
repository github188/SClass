#include "Stdafx.h"
#include "IO/StmData/FileData.h"
#include "Math/Math.h"
#include "Media/IMediaPlayer.h"
#include "SSWR/AVIRead/AVIRPlaylistForm.h"
#include "Sync/Thread.h"
#include "Text/MyString.h"
#include "Text/StringBuilderUTF8.h"
#include "UI/FileDialog.h"

typedef enum
{
	MNU_PB_START = 101,
	MNU_PB_STOP,
	MNU_PB_FWD,
	MNU_PB_BWD,
	MNU_PB_CHAP_PREV,
	MNU_PB_CHAP_NEXT,
	MNU_VIDEO_ORISIZE,
	MNU_VIDEO_FULLSCN,
	MNU_VIDEO_CROP,
	MNU_VIDEO_DEINT_AUTO,
	MNU_VIDEO_DEINT_PROG,
	MNU_VIDEO_DEINT_TFF,
	MNU_VIDEO_DEINT_BFF,
	MNU_POPV_REMOVE,
	MNU_POPV_SAVE_TIMECODE,
	MNU_POPA_REMOVE,

	MNU_PB_CHAPTERS = 1000
} MenuItems;

void __stdcall SSWR::AVIRead::AVIRPlaylistForm::OnFileDrop(void *userObj, const UTF8Char **files, UOSInt nFiles)
{
	SSWR::AVIRead::AVIRPlaylistForm *me = (SSWR::AVIRead::AVIRPlaylistForm*)userObj;
	Bool changed = false;
	UOSInt i;
	i = 0;
	while (i < nFiles)
	{
		if (me->playlist->AddFile(files[i]))
		{
			changed = true;
		}

		i++;
	}
	if (changed)
	{
		me->UpdatePlaylist();
	}
}

void __stdcall SSWR::AVIRead::AVIRPlaylistForm::OnPlaylistDblClk(void *userObj, UOSInt itemIndex)
{
	SSWR::AVIRead::AVIRPlaylistForm *me = (SSWR::AVIRead::AVIRPlaylistForm*)userObj;
	me->playlist->OpenItem(itemIndex);
	if (!me->playlist->IsPlaying())
	{
		me->playlist->StartPlayback();
	}
}

void __stdcall SSWR::AVIRead::AVIRPlaylistForm::OnStartClicked(void *userObj)
{
	SSWR::AVIRead::AVIRPlaylistForm *me = (SSWR::AVIRead::AVIRPlaylistForm*)userObj;
	me->playlist->StartPlayback();
}

void __stdcall SSWR::AVIRead::AVIRPlaylistForm::OnEndClicked(void *userObj)
{
	SSWR::AVIRead::AVIRPlaylistForm *me = (SSWR::AVIRead::AVIRPlaylistForm*)userObj;
	me->playlist->StopPlayback();
}

void __stdcall SSWR::AVIRead::AVIRPlaylistForm::OnFSClicked(void *userObj)
{
	SSWR::AVIRead::AVIRPlaylistForm *me = (SSWR::AVIRead::AVIRPlaylistForm*)userObj;
	me->vbdMain->SwitchFullScreen(true, true);
}

void SSWR::AVIRead::AVIRPlaylistForm::UpdatePlaylist()
{
	UOSInt i;
	UOSInt j;
	UOSInt k;
	const UTF8Char *csptr;
	this->lvPlaylist->ClearItems();
	i = 0;
	j = this->playlist->GetCount();
	while (i < j)
	{
		k = this->lvPlaylist->AddItem(this->playlist->GetTitle(i), (void*)i);
		csptr = this->playlist->GetArtist(i);
		if (csptr)
		{
			this->lvPlaylist->SetSubItem(k, 1, csptr);
		}
		i++;
	}
}

SSWR::AVIRead::AVIRPlaylistForm::AVIRPlaylistForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, Media::Playlist *playlist) : UI::GUIForm(parent, 1024, 768, ui)
{
	this->SetFont(0, 8.25, false);
	this->SetText((const UTF8Char*)"Playlist Form");

	this->core = core;
	this->colorSess = this->core->GetColorMgr()->CreateSess(this->GetHMonitor());
	this->playlist = playlist;
	this->currFileName = 0;
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));

	NEW_CLASS(this->lvPlaylist, UI::GUIListView(ui, this, UI::GUIListView::LVSTYLE_TABLE, 2));
	this->lvPlaylist->SetRect(0, 0, 160, 100, false);
	this->lvPlaylist->SetDockType(UI::GUIControl::DOCK_LEFT);
	this->lvPlaylist->AddColumn((const UTF8Char*)"Title", 100);
	this->lvPlaylist->AddColumn((const UTF8Char*)"Artist", 60);
	this->lvPlaylist->HandleDblClk(OnPlaylistDblClk, this);
	NEW_CLASS(this->hsplit, UI::GUIHSplitter(ui, this, 3, false));
	NEW_CLASS(this->pnlCtrl, UI::GUIPanel(ui, this));
	this->pnlCtrl->SetRect(0, 0, 100, 56, false);
	this->pnlCtrl->SetDockType(UI::GUIControl::DOCK_BOTTOM);
	NEW_CLASS(this->btnStart, UI::GUIButton(ui, this->pnlCtrl, (const UTF8Char*)"&Start"));
	this->btnStart->SetRect(16, 16, 75, 23, false);
	this->btnStart->HandleButtonClick(OnStartClicked, this);
	NEW_CLASS(this->btnEnd, UI::GUIButton(ui, this->pnlCtrl, (const UTF8Char*)"&End"));
	this->btnEnd->SetRect(96, 16, 75, 23, false);
	this->btnEnd->HandleButtonClick(OnEndClicked, this);
	NEW_CLASS(this->btnFS, UI::GUIButton(ui, this->pnlCtrl, (const UTF8Char*)"&Full Screen"));
	this->btnFS->SetRect(176, 16, 75, 23, false);
	this->btnFS->HandleButtonClick(OnFSClicked, this);
	NEW_CLASS(this->vbdMain, UI::GUIVideoBoxDD(ui, this, this->colorSess, 5, Sync::Thread::GetThreadCnt()));
	this->vbdMain->SetDockType(UI::GUIControl::DOCK_FILL);
	this->vbdMain->SetUserFSMode(UI::GUIDDrawControl::SM_VFS);

	this->HandleDropFiles(OnFileDrop, this);
	this->UpdatePlaylist();

	NEW_CLASS(this->player, SSWR::AVIRead::AVIRMediaPlayer(this->vbdMain, this->core));
	this->playlist->SetPlayer(this->player);
}

SSWR::AVIRead::AVIRPlaylistForm::~AVIRPlaylistForm()
{
	DEL_CLASS(this->playlist);
	DEL_CLASS(this->player);
	this->ClearChildren();
	this->core->GetColorMgr()->DeleteSess(this->colorSess);
}


void SSWR::AVIRead::AVIRPlaylistForm::EventMenuClicked(UInt16 cmdId)
{
/*	switch (cmdId)
	{
	default:
		break;
	}*/
}

void SSWR::AVIRead::AVIRPlaylistForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}
