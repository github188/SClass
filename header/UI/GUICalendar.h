#ifndef _SM_UI_GUICALENDAR
#define _SM_UI_GUICALENDAR
#include "Data/ArrayList.h"
#include "Data/DateTime.h"
#include "UI/GUIClientControl.h"

namespace UI
{
	class GUICalendar : public GUIControl
	{
	public:
		typedef void (__stdcall *DateChangedHandler)(void *userObj, Data::DateTime *newDate);
	private:
		static Int32 useCnt;

		Data::ArrayList<DateChangedHandler> *dateChangedHdlrs;
		Data::ArrayList<void*> *dateChangedObjs;
	public:
		GUICalendar(GUICore *ui, UI::GUIClientControl *parent);
		virtual ~GUICalendar();

		virtual const UTF8Char *GetObjectClass();
		virtual OSInt OnNotify(UInt32 code, void *lParam);

		void GetSelectedTime(Data::DateTime *dt);
		void HandleDateChange(DateChangedHandler hdlr, void *obj);
	};
};

#endif
