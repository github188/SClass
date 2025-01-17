#ifndef _SM_SSWR_AVIREAD_AVIRWEBSITE48IDOLFORM
#define _SM_SSWR_AVIREAD_AVIRWEBSITE48IDOLFORM
#include "Net/WebSite/WebSite48IdolControl.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIButton.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUIListView.h"
#include "UI/GUIPanel.h"
#include "UI/GUITabControl.h"
#include "UI/GUITabPage.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRWebSite48IdolForm : public UI::GUIForm
		{
		private:
			UI::GUITabControl *tcMain;
			
			UI::GUITabPage *tpItems;
			UI::GUIPanel *pnlRequest;
			UI::GUILabel *lblPageNo;
			UI::GUITextBox *txtPageNo;
			UI::GUIButton *btnRequestPage;
			UI::GUIListView *lvItems;

			UI::GUITabPage *tpDownloadLink;
			UI::GUILabel *lblVideoId;
			UI::GUITextBox *txtVideoId;
			UI::GUIButton *btnDownloadLink;
			UI::GUILabel *lblDownloadLink;
			UI::GUITextBox *txtDownloadLink;

			UI::GUITabPage *tpVideoName;
			UI::GUILabel *lblNameVideoId;
			UI::GUITextBox *txtNameVideoId;
			UI::GUIButton *btnVideoName;
			UI::GUILabel *lblVideoName;
			UI::GUITextBox *txtVideoName;

			Net::WebSite::WebSite48IdolControl *ctrl;
			SSWR::AVIRead::AVIRCore *core;

			static void __stdcall OnRequestPageClicked(void *userObj);
			static void __stdcall OnDownloadLinkClicked(void *userObj);
			static void __stdcall OnVideoNameClicked(void *userObj);
		public:
			AVIRWebSite48IdolForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRWebSite48IdolForm();

			virtual void OnMonitorChanged();
		};
	}
}
#endif
