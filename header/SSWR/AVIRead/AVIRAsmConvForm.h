#ifndef _SM_SSWR_AVIREAD_AVIRASMCONVFORM
#define _SM_SSWR_AVIREAD_AVIRASMCONVFORM
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUIForm.h"
#include "UI/GUIHSplitter.h"
#include "UI/GUIPanel.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRAsmConvForm : public UI::GUIForm
		{
		private:
			SSWR::AVIRead::AVIRCore *core;

			UI::GUIPanel *pnlCtrl;
			UI::GUIButton *btnConv;
			UI::GUIButton *btnConv2;
			UI::GUITextBox *txtIntelAsm;
			UI::GUIHSplitter *hspAsm;
			UI::GUITextBox *txtGNUAsm;

			static void __stdcall OnConvClicked(void *userObj);
			static void __stdcall OnConv2Clicked(void *userObj);
			void ConvAsm();
		public:
			AVIRAsmConvForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRAsmConvForm();

			virtual void OnMonitorChanged();
		};
	}
}
#endif
