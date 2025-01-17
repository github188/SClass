#ifndef _SM_SSWR_AVIREAD_AVIRTIMEDFILECOPYFORM
#define _SM_SSWR_AVIREAD_AVIRTIMEDFILECOPYFORM
#include "IO/ZIPBuilder.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUIDateTimePicker.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRTimedFileCopyForm : public UI::GUIForm
		{
		private:
			SSWR::AVIRead::AVIRCore *core;
			UI::GUILabel *lblFileDir;
			UI::GUITextBox *txtFileDir;
			UI::GUILabel *lblStartTime;
			UI::GUIDateTimePicker *dtpStartTime;
			UI::GUILabel *lblEndTime;
			UI::GUIDateTimePicker *dtpEndTime;
			UI::GUIButton *btnStart;

			static void __stdcall OnStartClicked(void *userObj);

			Bool CopyToZip(IO::ZIPBuilder *zip, const UTF8Char *buffStart, const UTF8Char *pathBase, UTF8Char *pathEnd, Data::DateTime *startTime, Data::DateTime *endTime, Bool monthDir);
		public:
			AVIRTimedFileCopyForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRTimedFileCopyForm();

			const UTF8Char *GetFormName();
			virtual void OnMonitorChanged();
		};
	}
}
#endif