#ifndef _SM_SSWR_AVIREAD_AVIRCPUINFOFORM
#define _SM_SSWR_AVIREAD_AVIRCPUINFOFORM
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIForm.h"
#include "UI/GUIListView.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRTableMsgForm : public UI::GUIForm
		{
		private:
			UI::GUIListView *lvTable;
			SSWR::AVIRead::AVIRCore *core;
			UOSInt colCnt;

		public:
			AVIRTableMsgForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, const UTF8Char *title, UOSInt colCnt, const UTF8Char **colName);
			virtual ~AVIRTableMsgForm();

			virtual void OnMonitorChanged();

			void AddRow(const UTF8Char **row);
		};
	}
}
#endif
