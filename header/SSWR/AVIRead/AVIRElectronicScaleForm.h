#ifndef _SM_SSWR_AVIREAD_AVIRELECTRONICSCALEFORM
#define _SM_SSWR_AVIREAD_AVIRELECTRONICSCALEFORM
#include "Math/Unit/Mass.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUIGroupBox.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUIPanel.h"
#include "UI/GUIPictureBoxSimple.h"
#include "UI/GUIRealtimeLineChart.h"
#include "UI/GUITabControl.h"
#include "UI/GUITabPage.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRElectronicScaleForm : public UI::GUIForm
		{
		private:
			SSWR::AVIRead::AVIRCore *core;
			Media::DrawEngine *eng;
			Media::DrawImage *dimg;

			UI::GUIPanel *pnlCtrl;
			UI::GUIGroupBox *grpStream;
			UI::GUILabel *lblStream;
			UI::GUITextBox *txtStream;
			UI::GUIButton *btnStream;
			UI::GUILabel *lblWeight;
			UI::GUITextBox *txtWeight;
			UI::GUITabControl *tcMain;

			UI::GUITabPage *tpHistory;
			UI::GUIRealtimeLineChart *rlcHistory;

			UI::GUITabPage *tpDisplay;
			UI::GUIPictureBoxSimple *pbsDisplay;

			IO::Stream *stm;
			Bool threadRunning;
			Bool threadToStop;
			Bool remoteClosed;

			Double currWeight;
			Math::Unit::Mass::MassUnit currWeightUnit;
			Bool currWeightUpd;

		private:
			static void __stdcall OnStreamClicked(void *userObj);
			static void __stdcall OnTimerTick(void *userObj);
			static UInt32 __stdcall RecvThread(void *userObj);

			void StopStream();
		public:
			AVIRElectronicScaleForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRElectronicScaleForm();

			virtual void OnMonitorChanged();
		};
	};
};
#endif
