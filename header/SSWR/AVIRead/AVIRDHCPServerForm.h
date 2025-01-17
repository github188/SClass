#ifndef _SM_SSWR_AVIREAD_AVIRDHCPSERVERFORM
#define _SM_SSWR_AVIREAD_AVIRDHCPSERVERFORM
#include "Net/DHCPServer.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUIComboBox.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUIListView.h"
#include "UI/GUIPanel.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRDHCPServerForm : public UI::GUIForm
		{
		private:
			SSWR::AVIRead::AVIRCore *core;
			Net::SocketFactory *sockf;
			Net::DHCPServer *svr;

			UI::GUIPanel *pnlControl;
			UI::GUILabel *lblIP;
			UI::GUIComboBox *cboIP;
			UI::GUILabel *lblSubnet;
			UI::GUITextBox *txtSubnet;
			UI::GUILabel *lblFirstIP;
			UI::GUITextBox *txtFirstIP;
			UI::GUILabel *lblDevCount;
			UI::GUITextBox *txtDevCount;
			UI::GUILabel *lblGateway;
			UI::GUITextBox *txtGateway;
			UI::GUILabel *lblDNS1;
			UI::GUITextBox *txtDNS1;
			UI::GUILabel *lblDNS2;
			UI::GUITextBox *txtDNS2;
			UI::GUIButton *btnStart;
			UI::GUIListView *lvDevices;

			static void __stdcall OnStartClicked(void *userObj);
			static void __stdcall OnTimerTick(void *userObj);
		public:
			AVIRDHCPServerForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRDHCPServerForm();

			virtual void OnMonitorChanged();
		};
	};
};
#endif
