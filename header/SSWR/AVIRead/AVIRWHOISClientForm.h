#ifndef _SM_SSWR_AVIREAD_AVIRWHOISCLIENTFORM
#define _SM_SSWR_AVIREAD_AVIRWHOISCLIENTFORM

#include "Net/WhoisClient.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUIListBox.h"
#include "UI/GUIPanel.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRWHOISClientForm : public UI::GUIForm
		{
		private:
			SSWR::AVIRead::AVIRCore *core;

			UI::GUIPanel *pnlRequest;
			UI::GUILabel *lblIP;
			UI::GUITextBox *txtIP;
			UI::GUILabel *lblRespTime;
			UI::GUITextBox *txtRespTime;
			UI::GUIButton *btnRequest;
			UI::GUIListBox *lbResponse;

			Net::SocketFactory *sockf;

			static void __stdcall OnRequestClicked(void *userObj);
		public:
			AVIRWHOISClientForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRWHOISClientForm();

			virtual void OnMonitorChanged();
		};
	};
};
#endif
