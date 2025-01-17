#ifndef _SM_SSWR_AVIREAD_AVIRDS18B20FORM
#define _SM_SSWR_AVIREAD_AVIRDS18B20FORM
#include "IO/IOPin.h"
#include "IO/Device/DS18B20.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUICheckBox.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRDS18B20Form : public UI::GUIForm
		{
		private:
			UI::GUILabel *lblSN;
			UI::GUITextBox *txtSN;
			UI::GUIButton *btnSN;
			UI::GUILabel *lblTemp;
			UI::GUITextBox *txtTemp;
			UI::GUIButton *btnRead;
			UI::GUICheckBox *chkAutoRead;
			UI::GUILabel *lblStatus;
			UI::GUITextBox *txtStatus;

			SSWR::AVIRead::AVIRCore *core;
			IO::IOPin *pin;
			IO::OneWireGPIO *oneWire;
			IO::Device::DS18B20 *ds18b20;

			static void __stdcall OnSNClicked(void *userObj);
			static void __stdcall OnReadClicked(void *userObj);
			static void __stdcall OnTimerTick(void *userObj);
			void ReadData();
		public:
			AVIRDS18B20Form(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, IO::IOPin *pin);
			virtual ~AVIRDS18B20Form();

			virtual void OnMonitorChanged();
		};
	};
};
#endif
