#include "Stdafx.h"
#include "Text/MyString.h"
#include "UtilUI/ColorDialog.h"
#include "UI/FontDialog.h"
#include "SSWR/AVIRead/AVIRGISLineForm.h"
#include "SSWR/AVIRead/AVIRGISLineEditForm.h"

#define MNU_SET_DEFAULT 101

void __stdcall SSWR::AVIRead::AVIRGISLineForm::OnThickChanged(void *userObj, OSInt newPos)
{
	UTF8Char sbuff[16];
	SSWR::AVIRead::AVIRGISLineForm *me = (SSWR::AVIRead::AVIRGISLineForm*)userObj;
	Text::StrOSInt(sbuff, newPos);
	me->lineThick = newPos;
	me->lblThickV->SetText(sbuff);
	me->UpdatePreview();
}

Bool __stdcall SSWR::AVIRead::AVIRGISLineForm::OnColorDown(void *userObj, OSInt x, OSInt y, MouseButton btn)
{
	SSWR::AVIRead::AVIRGISLineForm *me = (SSWR::AVIRead::AVIRGISLineForm*)userObj;
	UtilUI::ColorDialog *dlg;
	if (btn == UI::GUIControl::MBTN_LEFT)
	{
		Media::ColorProfile color(Media::ColorProfile::CPT_SRGB);
		NEW_CLASS(dlg, UtilUI::ColorDialog(0, me->ui, me->core->GetColorMgr(), me->core->GetDrawEngine(), UtilUI::ColorDialog::CCT_PHOTO, &color, me->core->GetMonitorMgr()));
		dlg->SetColor32(me->lineColor);
		if (dlg->ShowDialog(me) == UI::GUIForm::DR_OK)
		{
			me->lineColor = dlg->GetColor32();
			me->pbColor->SetBGColor(me->colorConv->ConvRGB8(me->lineColor));
			me->pbColor->Redraw();
			me->UpdatePreview();
		}
		DEL_CLASS(dlg);
	}
	return false;
}

void __stdcall SSWR::AVIRead::AVIRGISLineForm::OnOKClicked(void *userObj)
{
	SSWR::AVIRead::AVIRGISLineForm *me = (SSWR::AVIRead::AVIRGISLineForm*)userObj;
	me->SetDialogResult(UI::GUIForm::DR_OK);
}

void __stdcall SSWR::AVIRead::AVIRGISLineForm::OnCancelClicked(void *userObj)
{
	SSWR::AVIRead::AVIRGISLineForm *me = (SSWR::AVIRead::AVIRGISLineForm*)userObj;
	me->SetDialogResult(UI::GUIForm::DR_CANCEL);
}

void SSWR::AVIRead::AVIRGISLineForm::UpdatePreview()
{
	UOSInt w;
	UOSInt h;
	Media::DrawImage *dimg;
	this->pbPreview->GetSizeP(&w, &h);
	dimg = this->eng->CreateImage32(w, h, Media::AT_NO_ALPHA);
	dimg->SetHDPI(this->GetHDPI());
	dimg->SetVDPI(this->GetHDPI());
	this->core->GenLinePreview(dimg, this->eng, this->lineThick, this->lineColor, this->colorConv);
	SDEL_CLASS(this->prevImg);
	this->prevImg = dimg->ToStaticImage();
	this->eng->DeleteImage(dimg);
	this->pbPreview->SetImage(this->prevImg);
}

SSWR::AVIRead::AVIRGISLineForm::AVIRGISLineForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, Media::DrawEngine *eng, OSInt lineThick, Int32 lineColor) : UI::GUIForm(parent, 462, 334, ui)
{
	UTF8Char sbuff[16];
	this->core = core;
	this->eng = eng;
	this->lineThick = lineThick;
	this->lineColor = lineColor;
	this->prevImg = 0;
	this->colorSess = this->core->GetColorMgr()->CreateSess(this->GetHMonitor());
	this->colorSess->AddHandler(this);
	Media::ColorProfile srcColor(Media::ColorProfile::CPT_SRGB);
	Media::ColorProfile destColor(Media::ColorProfile::CPT_PDISPLAY);
	NEW_CLASS(this->colorConv, Media::ColorConv(&srcColor, &destColor, this->colorSess));
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));

	this->SetText((const UTF8Char*)"Line Modify");
	this->SetFont(0, 8.25, false);
	this->SetNoResize(true);

	NEW_CLASS(this->pbPreview, UI::GUIPictureBox(ui, this, this->eng, true, false));
	this->pbPreview->SetRect(0, 0, 100, 60, false);
	this->pbPreview->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->pnlMain, UI::GUIPanel(ui, this));
	this->pnlMain->SetDockType(UI::GUIControl::DOCK_FILL);
	NEW_CLASS(this->lblThick, UI::GUILabel(ui, this->pnlMain, (const UTF8Char*)"Thick"));
	this->lblThick->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->hsbThick, UI::GUIHScrollBar(ui, this->pnlMain, 16));
	this->hsbThick->InitScrollBar(0, 60, this->lineThick, 10);
	this->hsbThick->SetRect(104, 4, 150, 20, false);
	this->hsbThick->HandlePosChanged(OnThickChanged, this);
	Text::StrOSInt(sbuff, this->lineThick);
	NEW_CLASS(this->lblThickV, UI::GUILabel(ui, this->pnlMain, sbuff));
	this->lblThickV->SetRect(254, 4, 100, 23, false);
	NEW_CLASS(this->lblColor, UI::GUILabel(ui, this->pnlMain, (const UTF8Char*)"Color"));
	this->lblColor->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->pbColor, UI::GUIPictureBox(ui, this->pnlMain, this->eng, true, false));
	this->pbColor->SetRect(104, 28, 100, 20, false);
	this->pbColor->HandleMouseDown(OnColorDown, this);
	this->pbColor->SetBGColor(this->colorConv->ConvRGB8(this->lineColor));

	NEW_CLASS(this->btnOK, UI::GUIButton(ui, this->pnlMain, (const UTF8Char*)"OK"));
	this->btnOK->SetRect(104, 64, 75, 23, false);
	this->btnOK->HandleButtonClick(OnOKClicked, this);
	NEW_CLASS(this->btnCancel, UI::GUIButton(ui, this->pnlMain, (const UTF8Char*)"Cancel"));
	this->btnCancel->SetRect(184, 64, 75, 23, false);
	this->btnCancel->HandleButtonClick(OnCancelClicked, this);
	this->SetDefaultButton(this->btnOK);
	this->SetCancelButton(this->btnCancel);

	this->UpdatePreview();
}

SSWR::AVIRead::AVIRGISLineForm::~AVIRGISLineForm()
{
	SDEL_CLASS(this->prevImg);
	DEL_CLASS(this->colorConv);
	this->colorSess->RemoveHandler(this);
	this->ClearChildren();
	this->core->GetColorMgr()->DeleteSess(this->colorSess);
}

void SSWR::AVIRead::AVIRGISLineForm::OnMonitorChanged()
{
	this->colorSess->ChangeMonitor(this->GetHMonitor());
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}

void SSWR::AVIRead::AVIRGISLineForm::YUVParamChanged(const Media::IColorHandler::YUVPARAM *yuvParam)
{
}

void SSWR::AVIRead::AVIRGISLineForm::RGBParamChanged(const Media::IColorHandler::RGBPARAM2 *rgbParam)
{
	this->colorConv->RGBParamChanged(rgbParam);
	this->pbColor->SetBGColor(this->colorConv->ConvRGB8(this->lineColor));
	this->pbColor->Redraw();
	this->UpdatePreview();
}

OSInt SSWR::AVIRead::AVIRGISLineForm::GetLineThick()
{
	return this->lineThick;
}

Int32 SSWR::AVIRead::AVIRGISLineForm::GetLineColor()
{
	return this->lineColor;
}