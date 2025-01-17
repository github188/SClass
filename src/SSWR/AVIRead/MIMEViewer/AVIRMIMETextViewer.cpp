#include "Stdafx.h"
#include "SSWR/AVIRead/MIMEViewer/AVIRMIMETextViewer.h"

SSWR::AVIRead::MIMEViewer::AVIRMIMETextViewer::AVIRMIMETextViewer(SSWR::AVIRead::AVIRCore *core, UI::GUICore *ui, UI::GUIClientControl *ctrl, Media::ColorManagerSess *sess, Text::MIMEObj::TextMIMEObj *txt) : SSWR::AVIRead::MIMEViewer::AVIRMIMEViewer(core, ctrl, txt)
{
	this->txt = txt;

	Text::StringBuilderUTF8 sb;
	txt->GetText(&sb);
	NEW_CLASS(this->txtContent, UI::GUITextBox(ui, ctrl, sb.ToString(), true));
	this->txtContent->SetDockType(UI::GUIControl::DOCK_FILL);
	this->txtContent->SetReadOnly(true);
}

SSWR::AVIRead::MIMEViewer::AVIRMIMETextViewer::~AVIRMIMETextViewer()
{
}
