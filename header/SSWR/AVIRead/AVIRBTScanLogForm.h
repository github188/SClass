#ifndef _SM_SSWR_AVIREAD_AVIRBTSCANLOGFORM
#define _SM_SSWR_AVIREAD_AVIRBTSCANLOGFORM
#include "IO/BTScanLog.h"
#include "Net/MACInfoList.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUICheckBox.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUIListView.h"
#include "UI/GUIPanel.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRBTScanLogForm : public UI::GUIForm
		{
		private:
			SSWR::AVIRead::AVIRCore *core;

			UI::GUIPanel *pnlControl;
			UI::GUIButton *btnFile;
			UI::GUIButton *btnStore;
			UI::GUILabel *lblInfo;
			UI::GUIListView *lvContent;

			IO::BTScanLog *btLog;
			Net::MACInfoList *macList;

			static void __stdcall OnFileClicked(void *userObj);
			static void __stdcall OnStoreClicked(void *userObj);
			static void __stdcall OnContentDblClicked(void *userObj, UOSInt index);
			static void __stdcall OnContentSelChg(void *userObj);
			Bool LogFileStore();
			void LogUIUpdate();

			void UpdateStatus();
		public:
			AVIRBTScanLogForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, IO::BTScanLog *btLog);
			virtual ~AVIRBTScanLogForm();

			virtual void OnMonitorChanged();
		};
	}
}
#endif
