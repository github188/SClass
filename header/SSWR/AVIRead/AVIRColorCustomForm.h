#ifndef _SM_SSWR_AVIREAD_AVIRCOLORCUSTOMFORM
#define _SM_SSWR_AVIREAD_AVIRCOLORCUSTOMFORM
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUIComboBox.h"
#include "UI/GUIForm.h"
#include "UI/GUIGroupBox.h"
#include "UI/GUIHScrollBar.h"
#include "UI/GUILabel.h"
#include "UI/GUIListBox.h"
#include "UI/GUIPanel.h"
#include "UI/GUIRadioButton.h"
#include "UI/GUITabControl.h"
#include "UI/GUITabPage.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRColorCustomForm : public UI::GUIForm
		{
		private:
			SSWR::AVIRead::AVIRCore *core;
			Media::MonitorColorManager *monColor;

			UI::GUILabel *lblRX;
			UI::GUITextBox *txtRX;
			UI::GUIButton *btnRXDown;
			UI::GUIButton *btnRXUp;
			UI::GUILabel *lblRY;
			UI::GUITextBox *txtRY;
			UI::GUIButton *btnRYDown;
			UI::GUIButton *btnRYUp;
			UI::GUILabel *lblGX;
			UI::GUITextBox *txtGX;
			UI::GUIButton *btnGXDown;
			UI::GUIButton *btnGXUp;
			UI::GUILabel *lblGY;
			UI::GUITextBox *txtGY;
			UI::GUIButton *btnGYDown;
			UI::GUIButton *btnGYUp;
			UI::GUILabel *lblBX;
			UI::GUITextBox *txtBX;
			UI::GUIButton *btnBXDown;
			UI::GUIButton *btnBXUp;
			UI::GUILabel *lblBY;
			UI::GUITextBox *txtBY;
			UI::GUIButton *btnBYDown;
			UI::GUIButton *btnBYUp;
			UI::GUILabel *lblWX;
			UI::GUITextBox *txtWX;
			UI::GUIButton *btnWXDown;
			UI::GUIButton *btnWXUp;
			UI::GUILabel *lblWY;
			UI::GUITextBox *txtWY;
			UI::GUIButton *btnWYDown;
			UI::GUIButton *btnWYUp;
			UI::GUIButton *btnClose;

			static void __stdcall OnRXDownClicked(void *userObj);
			static void __stdcall OnRXUpClicked(void *userObj);
			static void __stdcall OnRYDownClicked(void *userObj);
			static void __stdcall OnRYUpClicked(void *userObj);
			static void __stdcall OnGXDownClicked(void *userObj);
			static void __stdcall OnGXUpClicked(void *userObj);
			static void __stdcall OnGYDownClicked(void *userObj);
			static void __stdcall OnGYUpClicked(void *userObj);
			static void __stdcall OnBXDownClicked(void *userObj);
			static void __stdcall OnBXUpClicked(void *userObj);
			static void __stdcall OnBYDownClicked(void *userObj);
			static void __stdcall OnBYUpClicked(void *userObj);
			static void __stdcall OnWXDownClicked(void *userObj);
			static void __stdcall OnWXUpClicked(void *userObj);
			static void __stdcall OnWYDownClicked(void *userObj);
			static void __stdcall OnWYUpClicked(void *userObj);
			static void __stdcall OnCloseClicked(void *userObj);
		public:
			AVIRColorCustomForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, Media::MonitorColorManager *monColor);
			virtual ~AVIRColorCustomForm();

			virtual void OnMonitorChanged();
		};
	};
};
#endif
