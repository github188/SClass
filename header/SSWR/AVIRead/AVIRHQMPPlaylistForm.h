#ifndef _SM_SSWR_AVIREAD_AVIRHQMPPLAYLISTFORM
#define _SM_SSWR_AVIREAD_AVIRHQMPPLAYLISTFORM
#include "Media/IMediaPlayer.h"
#include "Media/Playlist.h"
#include "SSWR/AVIRead/AVIRMediaPlayer.h"
#include "UI/GUIButton.h"
#include "UI/GUIForm.h"
#include "UI/GUIListBox.h"
#include "UI/GUIPanel.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRHQMPPlaylistForm : public UI::GUIForm 
		{
		private:
			UI::GUIPanel *pnlCtrl;
			UI::GUIListBox *lbPlaylist;
			UI::GUIButton *btnAdd;
			UI::GUIButton *btnAddDir;
			UI::GUIButton *btnClear;
			UI::GUIButton *btnOK;
			UI::GUIButton *btnCancel;

			SSWR::AVIRead::AVIRCore *core;
			Media::Playlist *playlist;
		private:
			static void __stdcall OnFileDrop(void *userObj, const UTF8Char **files, UOSInt nFiles);
			static void __stdcall OnAddClicked(void *userObj);
			static void __stdcall OnAddDirClicked(void *userObj);
			static void __stdcall OnClearClicked(void *userObj);
			static void __stdcall OnOKClicked(void *userObj);
			static void __stdcall OnCancelClicked(void *userObj);
			Bool AddFolder(UTF8Char *folderBuff, UTF8Char *folderBuffEnd);
			void UpdatePlaylist();
		public:
			AVIRHQMPPlaylistForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, Media::Playlist *playlist);
			virtual ~AVIRHQMPPlaylistForm();

			virtual void OnMonitorChanged();
			Media::Playlist *GetPlaylist();
		};
	};
};
#endif
