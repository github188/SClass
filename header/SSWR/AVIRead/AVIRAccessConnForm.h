#ifndef _SM_SSWR_AVIREAD_AVIRACCESSCONNFORM
#define _SM_SSWR_AVIREAD_AVIRACCESSCONNFORM
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRAccessConnForm : public UI::GUIForm
		{
		private:
			UI::GUILabel *lblFileName;
			UI::GUITextBox *txtFileName;
			UI::GUIButton *btnBrowse;
			UI::GUIButton *btnOK;
			UI::GUIButton *btnCancel;

			SSWR::AVIRead::AVIRCore *core;
			DB::DBConn *conn;

			static void __stdcall OnBrowseClicked(void *userObj);
			static void __stdcall OnOKClicked(void *userObj);
			static void __stdcall OnCancelClicked(void *userObj);
		public:
			AVIRAccessConnForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRAccessConnForm();

			virtual void OnMonitorChanged();

			DB::DBConn *GetDBConn();
		};
	}
}
#endif
