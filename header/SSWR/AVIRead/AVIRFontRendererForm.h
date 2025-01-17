#ifndef _SM_SSWR_AVIREAD_AVIRFONTRENDERERFORM
#define _SM_SSWR_AVIREAD_AVIRFONTRENDERERFORM
#include "Media/FontRenderer.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIForm.h"
#include "UI/GUIHSplitter.h"
#include "UI/GUIListBox.h"
#include "UI/GUIPanel.h"
#include "UI/GUIPictureBoxDD.h"
#include "UI/GUITabControl.h"
#include "UI/GUITabPage.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRFontRendererForm : public UI::GUIForm
		{
		private:
			UI::GUITabControl *tcMain;

			UI::GUITabPage *tpChar;
			UI::GUIListBox *lbChar;
			UI::GUIHSplitter *hspChar;
			UI::GUIPictureBoxDD *pbChar;

			SSWR::AVIRead::AVIRCore *core;
			Media::ColorManagerSess *colorSess;
			Media::FontRenderer *font;
			Media::StaticImage *currImg;

			static void __stdcall OnCharSelChg(void *userObj);
		public:
			AVIRFontRendererForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, Media::FontRenderer *font);
			virtual ~AVIRFontRendererForm();

			virtual void OnMonitorChanged();
		};
	};
};
#endif
