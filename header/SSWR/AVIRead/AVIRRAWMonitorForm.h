#ifndef _SM_SSWR_AVIREAD_AVIRRAWMONITORFORM
#define _SM_SSWR_AVIREAD_AVIRRAWMONITORFORM

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
#include "UI/GUIListView.h"
#include "UI/GUIPanel.h"
#include "UI/GUITabControl.h"
#include "UI/GUITabPage.h"
#include "UI/GUITextBox.h"
#include "UI/ListBoxLogger.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRRAWMonitorForm : public UI::GUIForm
		{
		private:
			typedef struct
			{
				UInt32 ip;
				Net::EthernetAnalyzer::IPTranStatus *sendStatus;
				Net::EthernetAnalyzer::IPTranStatus *recvStatus;
			} IPTranInfo;

			typedef struct
			{
				UInt32 ip;
				Int64 count;
				const UTF8Char *name;
				const UTF8Char *country;
			} PingIPInfo;

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
			UInt32 adapterIP;
			Bool adapterChanged;
			Bool dataUpdated;
			Sync::Mutex *pingIPMut;
			Data::UInt32Map<PingIPInfo*> *pingIPMap;
			Bool pingIPListUpdated;
			Bool pingIPContUpdated;
			PingIPInfo *currPingIP;
			Data::UInt32Map<IPTranInfo*> *ipTranMap;
			UOSInt ipTranCnt;

			UI::GUIPanel *pnlControl;
			UI::GUILabel *lblInfo;
			UI::GUITextBox *txtInfo;
			UI::GUIButton *btnInfo;
			UI::GUILabel *lblIP;
			UI::GUIComboBox *cboIP;
			UI::GUIButton *btnStart;
			UI::GUITabControl *tcMain;

			UI::GUITabPage *tpDevice;
			UI::GUIListView *lvDevice;

			UI::GUITabPage *tpIPTran;
			UI::GUIListBox *lbIPTran;
			UI::GUIHSplitter *hspIPTran;
			UI::GUITabControl *tcIPTran;
			UI::GUITabPage *tpIPTranInfo;
			UI::GUIListView *lvIPTranInfo;
			UI::GUITabPage *tpIPTranWhois;
			UI::GUITextBox *txtIPTranWhois;

			UI::GUITabPage *tpDNSReqv4;
			UI::GUIListBox *lbDNSReqv4;
			UI::GUIHSplitter *hspDNSReqv4;
			UI::GUIPanel *pnlDNSReqv4;
			UI::GUILabel *lblDNSReqv4Name;
			UI::GUITextBox *txtDNSReqv4Name;
			UI::GUILabel *lblDNSReqv4ReqTime;
			UI::GUITextBox *txtDNSReqv4ReqTime;
			UI::GUILabel *lblDNSReqv4TTL;
			UI::GUITextBox *txtDNSReqv4TTL;
			UI::GUIListView *lvDNSReqv4;

			UI::GUITabPage *tpDNSReqv6;
			UI::GUIListBox *lbDNSReqv6;
			UI::GUIHSplitter *hspDNSReqv6;
			UI::GUIPanel *pnlDNSReqv6;
			UI::GUILabel *lblDNSReqv6Name;
			UI::GUITextBox *txtDNSReqv6Name;
			UI::GUILabel *lblDNSReqv6ReqTime;
			UI::GUITextBox *txtDNSReqv6ReqTime;
			UI::GUILabel *lblDNSReqv6TTL;
			UI::GUITextBox *txtDNSReqv6TTL;
			UI::GUIListView *lvDNSReqv6;

			UI::GUITabPage *tpDNSReqOth;
			UI::GUIListBox *lbDNSReqOth;
			UI::GUIHSplitter *hspDNSReqOth;
			UI::GUIPanel *pnlDNSReqOth;
			UI::GUILabel *lblDNSReqOthName;
			UI::GUITextBox *txtDNSReqOthName;
			UI::GUILabel *lblDNSReqOthReqTime;
			UI::GUITextBox *txtDNSReqOthReqTime;
			UI::GUILabel *lblDNSReqOthTTL;
			UI::GUITextBox *txtDNSReqOthTTL;
			UI::GUIListView *lvDNSReqOth;

			UI::GUITabPage *tpDNSTarget;
			UI::GUIListBox *lbDNSTarget;
			UI::GUIHSplitter *hspDNSTarget;
			UI::GUITabControl *tcDNSTarget;
			UI::GUITabPage *tpDNSTargetDomain;
			UI::GUIListBox *lbDNSTargetDomain;
			UI::GUITabPage *tpDNSTargetWhois;
			UI::GUITextBox *txtDNSTargetWhois;

			UI::GUITabPage *tpDNSClient;
			UI::GUIListBox *lbDNSClient;
			UI::GUIHSplitter *hspDNSClient;
			UI::GUIListView *lvDNSClient;

			UI::GUITabPage *tpDHCP;
			UI::GUIListView *lvDHCP;
			
			UI::GUITabPage *tpIPLog;
			UI::GUIListBox *lbIPLog;
			UI::GUIHSplitter *hspIPLog;
			UI::GUIListBox *lbIPLogVal;

			UI::GUITabPage *tpPingIP;
			UI::GUIListBox *lbPingIP;
			UI::GUIHSplitter *hspPingIP;
			UI::GUITabControl *tcPingIP;
			UI::GUITabPage *tpPingIPInfo;
			UI::GUILabel *lblPingIPCount;
			UI::GUITextBox *txtPingIPCount;
			UI::GUILabel *lblPingIPName;
			UI::GUITextBox *txtPingIPName;
			UI::GUILabel *lblPingIPCountry;
			UI::GUITextBox *txtPingIPCountry;
			UI::GUITabPage *tpPingIPWhois;
			UI::GUITextBox *txtPingIPWhois;

			UI::GUITabPage *tpLog;
			UI::GUITextBox *txtLog;
			UI::GUIListBox *lbLog;

			static void __stdcall OnPingPacket(void *userData, UInt32 srcIP, UInt32 destIP, UInt8 ttl, UOSInt packetSize);
			static void __stdcall OnRAWData(void *userData, const UInt8 *rawData, UOSInt packetSize);
			static void __stdcall OnIPv4Data(void *userData, const UInt8 *rawData, UOSInt packetSize);
			static void __stdcall OnInfoClicked(void *userObj);
			static void __stdcall OnStartClicked(void *userObj);
			static void __stdcall OnIPSelChg(void *userObj);
			static void __stdcall OnIPTranSelChg(void *userObj);
			static void __stdcall OnLogSelChg(void *userObj);
			static void __stdcall OnPingIPSelChg(void *userObj);
			static void __stdcall OnDNSReqv4SelChg(void *userObj);
			static void __stdcall OnDNSReqv6SelChg(void *userObj);
			static void __stdcall OnDNSReqOthSelChg(void *userObj);
			static void __stdcall OnDNSTargetSelChg(void *userObj);
			static void __stdcall OnDNSClientSelChg(void *userObj);
			static void __stdcall OnIPLogSelChg(void *userObj);
			static void __stdcall OnTimerTick(void *userObj);

		public:
			AVIRRAWMonitorForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, Net::EthernetAnalyzer *analyzer);
			virtual ~AVIRRAWMonitorForm();

			virtual void OnMonitorChanged();
		};
	};
};
#endif
