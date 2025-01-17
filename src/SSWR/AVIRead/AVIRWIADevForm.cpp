#include "Stdafx.h"
#include "IO/SerialPort.h"
#include "Text/StringBuilder.h"
#include "UI/MessageDialog.h"
#include "SSWR/AVIRead/AVIRWIADevForm.h"

void __stdcall SSWR::AVIRead::AVIRWIADevForm::OnOKClick(void *userObj)
{
	SSWR::AVIRead::AVIRWIADevForm *me = (SSWR::AVIRead::AVIRWIADevForm*)userObj;
/*	if (me->currCapture == 0)
	{
		UI::MessageDialog::ShowDialog(L"Please select a device", L"Select Capture Device");
		return;
	}
	CaptureFormat *fmt = (CaptureFormat*)me->cboFormat->GetItem(me->cboFormat->GetSelectedIndex());
	if (fmt == 0)
	{
		UI::MessageDialog::ShowDialog(L"Please select a format", L"Select Capture Device");
		return;
	}

	me->currCapture->SetPreferSize(fmt->width, fmt->height, fmt->fourcc, fmt->bpp);
	me->capture = me->currCapture;
	me->currCapture = 0;*/

	me->SetDialogResult(UI::GUIForm::DR_OK);
}

void __stdcall SSWR::AVIRead::AVIRWIADevForm::OnCancelClick(void *userObj)
{
	SSWR::AVIRead::AVIRWIADevForm *me = (SSWR::AVIRead::AVIRWIADevForm*)userObj;
	me->SetDialogResult(UI::GUIForm::DR_CANCEL);
}

SSWR::AVIRead::AVIRWIADevForm::AVIRWIADevForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core) : UI::GUIForm(parent, 456, 200, ui)
{
	this->SetText((const UTF8Char*)"Select WIA Device");
	this->SetFont(0, 8.25, false);
	this->SetNoResize(true);

	this->core = core;
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
	NEW_CLASS(this->lblDevice, UI::GUILabel(ui, this, (const UTF8Char*)"Capture Device"));
	this->lblDevice->SetRect(8, 8, 100, 23, false);
	NEW_CLASS(this->lbDevice, UI::GUIListBox(ui, this, false));
	this->lbDevice->SetRect(112, 8, 256, 112, false);

	NEW_CLASS(this->btnOK, UI::GUIButton(ui, this, (const UTF8Char*)"&OK"));
	this->btnOK->SetRect(112, 144, 75, 23, false);
	this->btnOK->HandleButtonClick(OnOKClick, this);
	NEW_CLASS(this->btnCancel, UI::GUIButton(ui, this, (const UTF8Char*)"&Cancel"));
	this->btnCancel->SetRect(192, 144, 75, 23, false);
	this->btnCancel->HandleButtonClick(OnCancelClick, this);
	this->SetDefaultButton(this->btnOK);
	this->SetCancelButton(this->btnCancel);

	NEW_CLASS(this->wiaMgr, Media::WIAManager());
	UOSInt i;
	UOSInt j;
	i = 0;
	j = this->wiaMgr->GetDeviceCount();
	while (i < j)
	{
		this->lbDevice->AddItem(this->wiaMgr->GetDeviceName(i), (void*)i);
		i++;
	}
}

SSWR::AVIRead::AVIRWIADevForm::~AVIRWIADevForm()
{
	DEL_CLASS(this->wiaMgr);
}

void SSWR::AVIRead::AVIRWIADevForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}
