#ifndef _SM_SSWR_AVIREAD_AVIRWIADEVFORM
#define _SM_SSWR_AVIREAD_AVIRWIADEVFORM
#include "Media/WIAManager.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUIComboBox.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUIListBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRWIADevForm : public UI::GUIForm
		{
		private:
			SSWR::AVIRead::AVIRCore *core;
			Media::WIAManager *wiaMgr;

			UI::GUILabel *lblDevice;
			UI::GUIListBox *lbDevice;

			UI::GUIButton *btnOK;
			UI::GUIButton *btnCancel;

			static void __stdcall OnOKClick(void *userObj);
			static void __stdcall OnCancelClick(void *userObj);

		public:
			AVIRWIADevForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRWIADevForm();

			virtual void OnMonitorChanged();
		};
	};
};
#endif
