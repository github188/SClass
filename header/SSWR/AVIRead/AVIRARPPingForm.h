#ifndef _SM_SSWR_AVIREAD_AVIRARPPINGFORM
#define _SM_SSWR_AVIREAD_AVIRARPPINGFORM

#include "IO/LogTool.h"
#include "Manage/HiResClock.h"
#include "Net/ARPHandler.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "Sync/Event.h"
#include "UI/ListBoxLogger.h"
#include "UI/GUIButton.h"
#include "UI/GUICheckBox.h"
#include "UI/GUIComboBox.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUIListBox.h"
#include "UI/GUIPanel.h"
#include "UI/GUIRealtimeLineChart.h"
#include "UI/GUITextBox.h"
#include "UI/GUIVSplitter.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRARPPingForm : public UI::GUIForm
		{
		private:
			typedef struct
			{
				const UTF8Char *ifName;
				UInt32 ipAddr;
				UInt8 hwAddr[6];
			} AdapterInfo;
			
		private:
			SSWR::AVIRead::AVIRCore *core;
			Net::SocketFactory *sockf;
			Net::ARPHandler *arpHdlr;
			Net::SocketUtil::AddressInfo targetAddr;
			Manage::HiResClock *clk;
			Bool requested;
			Sync::Event *reqEvt;
			IO::LogTool *log;
			UI::ListBoxLogger *logger;
			Data::ArrayList<AdapterInfo*> *adapters;

			UI::GUIPanel *pnlRequest;
			UI::GUIVSplitter *vspRequest;
			UI::GUILabel *lblAdapter;
			UI::GUIComboBox *cboAdapter;
			UI::GUILabel *lblTarget;
			UI::GUITextBox *txtTarget;
			UI::GUILabel *lblRepeat;
			UI::GUICheckBox *chkRepeat;
			UI::GUIButton *btnPing;
			UI::GUIRealtimeLineChart *rlcPing;
			UI::GUIVSplitter *vspPing;
			UI::GUIListBox *lbLog;

			static void __stdcall OnARPHandler(const UInt8 *hwAddr, UInt32 ipAddr, void *userObj);
			static void __stdcall OnPingClicked(void *userObj);
			static void __stdcall OnTimerTick(void *userObj);
		public:
			AVIRARPPingForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRARPPingForm();

			virtual void OnMonitorChanged();
		};
	}
}
#endif
