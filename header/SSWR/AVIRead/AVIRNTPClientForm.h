#ifndef _SM_SSWR_AVIREAD_AVIRNTPCLIENTFORM
#define _SM_SSWR_AVIREAD_AVIRNTPCLIENTFORM
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUIComboBox.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRNTPClientForm : public UI::GUIForm
		{
		private:
			UI::GUILabel *lblServer;
			UI::GUIComboBox *cboServer;
			UI::GUILabel *lblServerTime;
			UI::GUITextBox *txtServerTime;
			UI::GUILabel *lblStatus;
			UI::GUITextBox *txtStatus;
			UI::GUIButton *btnGetTime;
			UI::GUIButton *btnSyncTime;

			SSWR::AVIRead::AVIRCore *core;

			static void __stdcall OnGetTimeClicked(void *userObj);
			static void __stdcall OnSyncTimeClicked(void *userObj);
		public:
			AVIRNTPClientForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRNTPClientForm();

			virtual void OnMonitorChanged();
		};
	};
};
#endif
