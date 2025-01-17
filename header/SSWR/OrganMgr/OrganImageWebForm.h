#ifndef _SM_SSWR_ORGANMGR_ORGANIMAGEWEBFORM
#define _SM_SSWR_ORGANMGR_ORGANIMAGEWEBFORM

#include "SSWR/OrganMgr/OrganEnv.h"
#include "UI/GUIButton.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace OrganMgr
	{
		class OrganImageWebForm : public UI::GUIForm
		{
		private:
			OrganEnv *env;
			const UTF8Char *srcURL;
			const UTF8Char *location;

			UI::GUILabel *lblId;
			UI::GUITextBox *txtId;
			UI::GUILabel *lblFileName;
			UI::GUITextBox *txtFileName;
			UI::GUILabel *lblImageURL;
			UI::GUITextBox *txtImageURL;
			UI::GUILabel *lblSourceURL;
			UI::GUITextBox *txtSourceURL;
			UI::GUILabel *lblLocation;
			UI::GUITextBox *txtLocation;
			UI::GUIButton *btnOK;
			UI::GUIButton *btnCancel;

			static void __stdcall OnOKClicked(void *userObj);
			static void __stdcall OnCancelClicked(void *userObj);
		public:
			OrganImageWebForm(UI::GUIClientControl *parent, UI::GUICore *ui, OrganEnv *env, OrganImageItem *userFile, SSWR::OrganMgr::WebFileInfo *wfile);
			virtual ~OrganImageWebForm();

			virtual void OnMonitorChanged();

			const UTF8Char *GetSrcURL();
			const UTF8Char *GetLocation();
		};
	}
}
#endif
