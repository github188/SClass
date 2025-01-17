#ifndef _SM_SSWR_AVIREAD_AVIRHIDDEVICEFORM
#define _SM_SSWR_AVIREAD_AVIRHIDDEVICEFORM
#include "IO/HIDInfo.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIHSplitter.h"
#include "UI/GUILabel.h"
#include "UI/GUIListBox.h"
#include "UI/GUIPanel.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRHIDDeviceForm : public UI::GUIForm
		{
		private:
			SSWR::AVIRead::AVIRCore *core;
			Data::ArrayList<IO::HIDInfo*> *hidList;

			UI::GUIListBox *lbDevices;
			UI::GUIHSplitter *hspDevices;
			UI::GUIPanel *pnlDevices;
			UI::GUILabel *lblVendorId;
			UI::GUITextBox *txtVendorId;
			UI::GUILabel *lblProductId;
			UI::GUITextBox *txtProductId;
			UI::GUILabel *lblDevPath;
			UI::GUITextBox *txtDevPath;
			
			static void __stdcall OnDevicesSelChg(void *userObj);
			static OSInt __stdcall ItemCompare(void *item1, void *item2);
		public:
			AVIRHIDDeviceForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRHIDDeviceForm();

			virtual void OnMonitorChanged();
		};
	};
};
#endif
