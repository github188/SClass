#ifndef _SM_SSWR_AVIREAD_MIMEVIEWER_AVIRUNKNOWNVIEWER
#define _SM_SSWR_AVIREAD_MIMEVIEWER_AVIRUNKNOWNVIEWER
#include "SSWR/AVIRead/AVIRCore.h"
#include "SSWR/AVIRead/MIMEViewer/AVIRMIMEViewer.h"
#include "Text/IMIMEObj.h"
#include "UI/GUILabel.h"

namespace SSWR
{
	namespace AVIRead
	{
		namespace MIMEViewer
		{
			class AVIRUnknownViewer : public AVIRMIMEViewer
			{
			private:
				UI::GUILabel *lblMessage;

				Text::IMIMEObj *obj;

			public:
				AVIRUnknownViewer(SSWR::AVIRead::AVIRCore *core, UI::GUICore *ui, UI::GUIClientControl *ctrl, Media::ColorManagerSess *sess, Text::IMIMEObj *obj);
				virtual ~AVIRUnknownViewer();
			};
		}
	}
}
#endif