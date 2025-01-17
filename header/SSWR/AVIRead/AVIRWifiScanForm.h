#ifndef _SM_SSWR_AVIREAD_AVIRWIFISCANFORM
#define _SM_SSWR_AVIREAD_AVIRWIFISCANFORM
#include "Net/WirelessLAN.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUIListView.h"
#include "UI/GUIPanel.h"
#include "UI/GUITextBox.h"
#include "UI/GUIVSplitter.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRWifiScanForm : public UI::GUIForm
		{
		private:
			SSWR::AVIRead::AVIRCore *core;
			Net::WirelessLAN *wlan;
			Net::WirelessLAN::Interface *wlanInterf;

			UI::GUIPanel *pnlControl;
			UI::GUIButton *btnScan;
			UI::GUILabel *lblScanTime;
			UI::GUITextBox *txtScanTime;
			UI::GUILabel *lblResultTime;
			UI::GUITextBox *txtResultTime;
			UI::GUIListView *lvWifi;
			UI::GUIVSplitter *vspWifi;
			UI::GUITextBox *txtWifi;

			static void __stdcall OnScanClicked(void *userObj);
			static void __stdcall OnWifiSelChg(void *userObj);
			void WifiScan();
			void WifiClear();
		public:
			AVIRWifiScanForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRWifiScanForm();

			virtual void OnMonitorChanged();
		};
	};
};
#endif
