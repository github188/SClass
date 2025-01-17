#ifndef _SM_SSWR_SHPCONV_SHPCONVELEVATIONFILTERFORM
#define _SM_SSWR_SHPCONV_SHPCONVELEVATIONFILTERFORM
#include "DB/DBFFile.h"
#include "Media/MonitorMgr.h"
#include "SSWR/SHPConv/MapFilter.h"
#include "UI/GUIButton.h"
#include "UI/GUIComboBox.h"
#include "UI/GUIForm.h"
#include "UI/GUILabel.h"
#include "UI/GUITextBox.h"

namespace SSWR
{
	namespace SHPConv
	{
		class SHPConvElevationFilterForm : public UI::GUIForm
		{
		private:
			UI::GUILabel *lblCol;
			UI::GUIComboBox *cboCol;
			UI::GUILabel *lblHeight;
			UI::GUITextBox *txtHeight;
			UI::GUIButton *btnOK;
			UI::GUIButton *btnCancel;
			DB::DBFFile *dbf;
			Media::MonitorMgr *monMgr;
			MapFilter *filter;

			static void __stdcall OnOKClicked(void *userObj);
			static void __stdcall OnCancelClicked(void *userObj);
		public:
			SHPConvElevationFilterForm(UI::GUIClientControl *parent, UI::GUICore *ui, DB::DBFFile *dbf);
			virtual ~SHPConvElevationFilterForm();

			virtual void OnMonitorChanged();
			MapFilter *GetFilter();
		};
	};
};
#endif
