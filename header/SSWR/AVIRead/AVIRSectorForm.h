#ifndef _SM_SSWR_AVIREAD_AVIRSECTORFORM
#define _SM_SSWR_AVIREAD_AVIRSECTORFORM
#include "IO/ISectorData.h"
#include "UI/GUIButton.h"
#include "UI/GUIForm.h"
#include "UI/GUIPanel.h"
#include "SSWR/AVIRead/AVIRCore.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRSectorForm : public UI::GUIForm
		{
		private:
			UI::GUIPanel *pnlCtrl;
			UI::GUIButton *btnParse;

			SSWR::AVIRead::AVIRCore *core;
			IO::ISectorData *data;

			UInt8 *sectorData;
			UOSInt sectorSize;

			static void __stdcall OnParseClicked(void *userObj);
		public:
			AVIRSectorForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, IO::ISectorData *data);
			virtual ~AVIRSectorForm();

			virtual void OnMonitorChanged();
		};
	}
}
#endif
