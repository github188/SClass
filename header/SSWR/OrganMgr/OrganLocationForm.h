#ifndef _SM_SSWR_ORGANMGR_ORGANLOCATIONFORM
#define _SM_SSWR_ORGANMGR_ORGANLOCATIONFORM

#include "SSWR/OrganMgr/OrganEnv.h"
#include "UI/GUIButton.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUIListBox.h"
#include "UI/GUIPanel.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace OrganMgr
	{
		class OrganLocationForm : public UI::GUIForm
		{
		public:
			typedef enum
			{
				SM_NONE,
				SM_CHILD,
				SM_ANY
			} SelectMode;
		private:
			UI::GUIListBox *lbLocation;
			UI::GUIListBox *lbSublocations;
			UI::GUIPanel *pnlLocation;
			UI::GUILabel *lblID;
			UI::GUILabel *lblEName;
			UI::GUILabel *lblCName;
			UI::GUITextBox *txtID;
			UI::GUITextBox *txtEName;
			UI::GUITextBox *txtCName;
			UI::GUIButton *btnAdd;
			UI::GUIButton *btnOk;
			UI::GUIButton *btnCancel;

			OrganEnv *env;
			SelectMode selMode;
			Int32 initId;
			Bool sublocUpdating;
			Location *currLoc;
			UOSInt currLocInd;
			Location *selVal;
			
			void DispId(Int32 id);
			void UpdateSubloc();
			Bool ToSave();
			Location *GetParentLoc();

			static void __stdcall OnLocSelChg(void *userObj);
			static void __stdcall OnSubLocSelChg(void *userObj);
			static void __stdcall OnSubLocDblClk(void *userObj);
			static void __stdcall OnAddClicked(void *userObj);
			static void __stdcall OnOkClicked(void *userObj);
			static void __stdcall OnCancelClicked(void *userObj);
		public:
			OrganLocationForm(UI::GUIClientControl *parent, UI::GUICore *ui, OrganEnv *env, SelectMode selMode, Int32 initId);
			virtual ~OrganLocationForm();

			virtual void OnMonitorChanged();

			Location *GetSelVal();
		};
	}
}
#endif