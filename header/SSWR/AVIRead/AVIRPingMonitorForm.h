#ifndef _SM_SSWR_AVIREAD_AVIRPINGMONITORFORM
#define _SM_SSWR_AVIREAD_AVIRPINGMONITORFORM

#include "IO/LogTool.h"
#include "Net/EthernetAnalyzer.h"
#include "Net/EthernetWebHandler.h"
#include "Net/SocketMonitor.h"
#include "Net/WhoisHandler.h"
#include "Net/WebServer/WebListener.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUIComboBox.h"
#include "UI/GUIForm.h"
#include "UI/GUIHSplitter.h"
#include "UI/GUILabel.h"
#include "UI/GUIListBox.h"
#include "UI/GUIPanel.h"
#include "UI/GUITabControl.h"
#include "UI/GUITabPage.h"
#include "UI/GUITextBox.h"
#include "UI/ListBoxLogger.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRPingMonitorForm : public UI::GUIForm
		{
		private:
			typedef struct
			{
				UInt32 ip;
				Int64 count;
				const UTF8Char *name;
				const UTF8Char *country;
			} IPInfo;

		private:
			SSWR::AVIRead::AVIRCore *core;
			Net::SocketFactory *sockf;
			IO::LogTool *log;
			UI::ListBoxLogger *logger;
			Net::WebServer::WebListener *listener;
			Net::EthernetWebHandler *webHdlr;
			Net::SocketMonitor *socMon;
			Net::EthernetAnalyzer *analyzer;
			Net::WhoisHandler *whois;
			Sync::Mutex *ipMut;
			Data::UInt32Map<IPInfo*> *ipMap;
			Bool ipListUpdated;
			Bool ipContUpdated;
			IPInfo *currIP;

			UI::GUIPanel *pnlControl;
			UI::GUILabel *lblInfo;
			UI::GUITextBox *txtInfo;
			UI::GUIButton *btnInfo;
			UI::GUILabel *lblIP;
			UI::GUIComboBox *cboIP;
			UI::GUIButton *btnStart;
			UI::GUITabControl *tcMain;

			UI::GUITabPage *tpIP;
			UI::GUIListBox *lbIP;
			UI::GUIHSplitter *hspIP;
			UI::GUITabControl *tcIP;
			UI::GUITabPage *tpIPInfo;
			UI::GUILabel *lblIPCount;
			UI::GUITextBox *txtIPCount;
			UI::GUILabel *lblIPName;
			UI::GUITextBox *txtIPName;
			UI::GUILabel *lblIPCountry;
			UI::GUITextBox *txtIPCountry;
			UI::GUITabPage *tpIPWhois;
			UI::GUITextBox *txtIPWhois;

			UI::GUITabPage *tpLog;
			UI::GUITextBox *txtLog;
			UI::GUIListBox *lbLog;

			static void __stdcall OnPingPacket(void *userData, UInt32 srcIP, UInt32 destIP, UInt8 ttl, UOSInt packetSize);
			static void __stdcall OnRAWData(void *userData, const UInt8 *rawData, UOSInt packetSize);
			static void __stdcall OnInfoClicked(void *userObj);
			static void __stdcall OnStartClicked(void *userObj);
			static void __stdcall OnLogSelChg(void *userObj);
			static void __stdcall OnIPSelChg(void *userObj);
			static void __stdcall OnTimerTick(void *userObj);

		public:
			AVIRPingMonitorForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRPingMonitorForm();

			virtual void OnMonitorChanged();
		};
	}
}
#endif
