#ifndef _SM_SSWR_ORGANMGR_ORGANSELCATEGORYFORM
#define _SM_SSWR_ORGANMGR_ORGANSELCATEGORYFORM

#include "SSWR/OrganMgr/OrganEnv.h"
#include "UI/GUIButton.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUIListBox.h"

namespace SSWR
{
	namespace OrganMgr
	{
		class OrganSelCategoryForm : public UI::GUIForm
		{
		private:
			UI::GUILabel *lbl;
			UI::GUIListBox *lbCategory;
			UI::GUIButton *btnOK;
			UI::GUIButton *btnCancel;
			SSWR::OrganMgr::OrganEnv *env;
			
			static void __stdcall OnOKClicked(void *userObj);
			static void __stdcall OnCancelClicked(void *userObj);
		public:
			OrganSelCategoryForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::OrganMgr::OrganEnv *env);
			~OrganSelCategoryForm();

			virtual void OnMonitorChanged();

		private:
			void InitCategory();
		};
	}
}
#endif