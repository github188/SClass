#ifndef _SM_SSWR_AVIREAD_AVIRGSMMODEMFORM
#define _SM_SSWR_AVIREAD_AVIRGSMMODEMFORM
#include "IO/GSMModemController.h"
#include "UI/GUIButton.h"
#include "UI/GUIComboBox.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUIListView.h"
#include "UI/GUIPanel.h"
#include "UI/GUITabControl.h"
#include "UI/GUITextBox.h"
#include "SSWR/AVIRead/AVIRCore.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRGSMModemForm : public UI::GUIForm
		{
		private:
			SSWR::AVIRead::AVIRCore *core;
			IO::GSMModemController *modem;
			IO::ATCommandChannel *channel;
			IO::Stream *port;

			UI::GUITabControl *tcMain;
			UI::GUITabPage *tpInfo;
			UI::GUITabPage *tpPhoneBook;
			UI::GUITabPage *tpSMS;

			UI::GUILabel *lblModemManu;
			UI::GUITextBox *txtModemManu;
			UI::GUILabel *lblModemModel;
			UI::GUITextBox *txtModemModel;
			UI::GUILabel *lblModemVer;
			UI::GUITextBox *txtModemVer;
			UI::GUILabel *lblModemIMEI;
			UI::GUITextBox *txtModemIMEI;
			UI::GUILabel *lblOperator;
			UI::GUITextBox *txtOperator;
			UI::GUILabel *lblSignalQuality;
			UI::GUITextBox *txtSignalQuality;

			UI::GUIPanel *pnlPhone;
			UI::GUIComboBox *cboPhoneStorage;
			UI::GUIButton *btnPhoneRead;
			UI::GUIListView *lvPhone;

			UI::GUIPanel *pnlSMS;
			UI::GUIComboBox *cboSMSStorage;
			UI::GUIButton *btnSMSRead;
			UI::GUILabel *lblSMSC;
			UI::GUITextBox *txtSMSC;
			UI::GUIButton *btnSMSSave;
			UI::GUIButton *btnSMSDelete;
			UI::GUIButton *btnSMSSaveAll;
			UI::GUIListView *lvSMS;

			Bool initStrs;
			const UTF8Char *initModemManu;
			const UTF8Char *initModemModel;
			const UTF8Char *initModemVer;
			const UTF8Char *initIMEI;
			Data::ArrayList<IO::GSMModemController::SMSMessage *> *msgList;

			IO::GSMModemController::RSSI signalQuality;
			Bool operUpdated;
			const UTF8Char *operName;
			Data::DateTime *operNextTime;

			Bool toStop;
			Bool running;
			Sync::Event *modemEvt;

			static UInt32 __stdcall ModemThread(void *userObj);
			static void __stdcall OnTimerTick(void *userObj);
			static void __stdcall OnPhoneRClick(void *userObj);
			static void __stdcall OnSMSRClick(void *userObj);
			static void __stdcall OnSMSSaveClick(void *userObj);
			static void __stdcall OnSMSDeleteClick(void *userObj);
			static void __stdcall OnSMSSaveAllClick(void *userObj);

			void LoadPhoneBook();
			void LoadSMS();
		public:
			AVIRGSMModemForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, IO::GSMModemController *modem, IO::ATCommandChannel *channel, IO::Stream *port);
			virtual ~AVIRGSMModemForm();

			virtual void OnMonitorChanged();
		};
	};
};
#endif
