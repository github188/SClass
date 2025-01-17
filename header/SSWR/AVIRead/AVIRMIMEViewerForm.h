#ifndef _SM_SSWR_AVIREAD_AVIRMIMEVIEWERFORM
#define _SM_SSWR_AVIREAD_AVIRMIMEVIEWERFORM
#include "SSWR/AVIRead/AVIRCore.h"
#include "SSWR/AVIRead/MIMEViewer/AVIRMIMEViewer.h"
#include "Text/IMIMEObj.h"
#include "UI/GUIForm.h"

namespace SSWR
{
	namespace AVIRead
	{
		class AVIRMIMEViewerForm : public UI::GUIForm
		{
		private:
			SSWR::AVIRead::AVIRCore *core;
			Text::IMIMEObj *obj;
			SSWR::AVIRead::MIMEViewer::AVIRMIMEViewer *viewer;
			Media::ColorManagerSess *sess;

		public:
			AVIRMIMEViewerForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, Text::IMIMEObj *obj);
			virtual ~AVIRMIMEViewerForm();

			virtual void OnMonitorChanged();
		};
	};
};
#endif
