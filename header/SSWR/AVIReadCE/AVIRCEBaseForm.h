#ifndef _SM_SSWR_AVIREADCE_AVIRCEBASEFORM
#define _SM_SSWR_AVIREADCE_AVIRCEBASEFORM
#include "Data/Int32Map.h"
#include "SSWR/AVIRead/AVIRCore.h"
#include "UI/GUIForm.h"
#include "UI/GUIHSplitter.h"
#include "UI/GUIListBox.h"

namespace SSWR
{
	namespace AVIReadCE
	{
		class AVIRCEBaseForm : public UI::GUIForm
		{
		private:
			typedef struct
			{
				const WChar *name;
				Int32 item;
			} MenuInfo;
		private:
			SSWR::AVIRead::AVIRCore *core;
			UI::GUIListBox *lbCategory;
			UI::GUIHSplitter *hspMain;
			UI::GUIListBox *lbContent;
			Data::Int32Map<Data::ArrayList<MenuInfo*>*> *menuItems;

		private:
			static void __stdcall FileHandler(void *userObj, const UTF8Char **files, OSInt nFiles);
			static void __stdcall OnCategoryChg(void *userObj);
			static void __stdcall OnContentClick(void *userObj);

			static MenuInfo *__stdcall NewMenuItem(const WChar *name, Int32 item);
		public:
			AVIRCEBaseForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core);
			virtual ~AVIRCEBaseForm();

			virtual void EventMenuClicked(UInt16 cmdId);
		};
	};
};
#endif
