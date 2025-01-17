#ifndef _SM_SSWR_AVIREAD_AVIRSYSLOGSERVERFORM
#define _SM_SSWR_AVIREAD_AVIRSYSLOGSERVERFORM
#include "Data/UInt32Map.h"
#include "Net/SyslogServer.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/ListBoxLogger.h"
#include "UI/GUIButton.h"
#include "UI/GUIForm.h"
#include "UI/GUIPanel.h"
#include "UI/GUIHSplitter.h"
#include "UI/GUILabel.h"
#include "UI/GUIListBox.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRSyslogServerForm : public UI::GUIForm
		{
		private:
			typedef struct
			{
				UInt32 ip;
				Data::ArrayList<const UTF8Char*> *logMessage;
			} IPLog;
		private:
			SSWR::AVIRead::AVIRCore *core;
			Net::SyslogServer *svr;

			UI::GUIPanel *pnlControl;
			UI::GUILabel *lblPort;
			UI::GUITextBox *txtPort;
			UI::GUIButton *btnStart;
			UI::GUIListBox *lbClient;
			UI::GUIHSplitter *hspClient;
			UI::GUITextBox *txtLog;
			UI::GUIListBox *lbLog;

			UInt32 currIP;
			Bool ipListUpd;
			Bool msgListUpd;
			Sync::Mutex *ipMut;
			Data::UInt32Map<IPLog*> *ipMap;

		private:
			static void __stdcall OnStartClick(void *userObj);
			static void __stdcall OnClientSelChg(void *userObj);
			static void __stdcall OnLogSelChg(void *userObj);
			static void __stdcall OnClientLog(void *userObj, UInt32 ip, const UTF8Char *message);
			static void __stdcall OnTimerTick(void *userObj);
		public:
			AVIRSyslogServerForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRSyslogServerForm();

			virtual void OnMonitorChanged();
		};
	}
}
#endif
