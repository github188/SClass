#include "Stdafx.h"
#include "SSWR/SHPConv/SHPConvAddFilterForm.h"
#include "SSWR/SHPConv/SHPConvElevationFilterForm.h"
#include "SSWR/SHPConv/SHPConvRangeFilterForm.h"
#include "SSWR/SHPConv/SHPConvValueFilterForm.h"

void __stdcall SSWR::SHPConv::SHPConvAddFilterForm::OnOKClicked(void *userObj)
{
	SSWR::SHPConv::SHPConvAddFilterForm *me = (SSWR::SHPConv::SHPConvAddFilterForm*)userObj;
	UOSInt i = me->cboFilter->GetSelectedIndex();
	if (i == 0)
	{
		SSWR::SHPConv::SHPConvValueFilterForm *frm;
		NEW_CLASS(frm, SSWR::SHPConv::SHPConvValueFilterForm(0, me->ui, me->dbf));
		if (frm->ShowDialog(me) == UI::GUIForm::DR_OK)
		{
			me->filter = frm->GetFilter();
			me->SetDialogResult(UI::GUIForm::DR_OK);
		}
		DEL_CLASS(frm);
	}
	else if (i == 1)
	{
		SSWR::SHPConv::SHPConvRangeFilterForm *frm;
		NEW_CLASS(frm, SSWR::SHPConv::SHPConvRangeFilterForm(0, me->ui, me->deng));
		if (frm->ShowDialog(me) == UI::GUIForm::DR_OK)
		{
			me->filter = frm->GetFilter();
			me->SetDialogResult(UI::GUIForm::DR_OK);
		}
		DEL_CLASS(frm);
	}
	else if (i == 2)
	{
		SSWR::SHPConv::SHPConvElevationFilterForm *frm;
		NEW_CLASS(frm, SSWR::SHPConv::SHPConvElevationFilterForm(0, me->ui, me->dbf));
		if (frm->ShowDialog(me) == UI::GUIForm::DR_OK)
		{
			me->filter = frm->GetFilter();
			me->SetDialogResult(UI::GUIForm::DR_OK);
		}
		DEL_CLASS(frm);
	}
}

void __stdcall SSWR::SHPConv::SHPConvAddFilterForm::OnCancelClicked(void *userObj)
{
	SSWR::SHPConv::SHPConvAddFilterForm *me = (SSWR::SHPConv::SHPConvAddFilterForm*)userObj;
	me->SetDialogResult(UI::GUIForm::DR_CANCEL);
}

SSWR::SHPConv::SHPConvAddFilterForm::SHPConvAddFilterForm(UI::GUIClientControl *parent, UI::GUICore *ui, DB::DBFFile *dbf, Media::DrawEngine *deng) : UI::GUIForm(parent, 222, 118, ui)
{
	this->SetText((const UTF8Char*)"Add Filter");
	this->SetFont(0, 8.25, false);
	this->SetNoResize(true);
	NEW_CLASS(this->monMgr, Media::MonitorMgr());
	this->SetDPI(this->monMgr->GetMonitorHDPI(this->GetHMonitor()), this->monMgr->GetMonitorDDPI(this->GetHMonitor()));
	this->deng = deng;
	this->dbf = dbf;
	this->filter = 0;
	
	NEW_CLASS(this->lblFilter, UI::GUILabel(ui, this, (const UTF8Char*)"Filter Type"));
	this->lblFilter->SetRect(8, 16, 64, 23, false);
	NEW_CLASS(this->cboFilter, UI::GUIComboBox(ui, this, false));
	this->cboFilter->SetRect(80, 16, 121, 21, false);
	this->cboFilter->AddItem((const UTF8Char*)"Value Filter", 0);
	this->cboFilter->AddItem((const UTF8Char*)"Range Filter", 0);
	this->cboFilter->AddItem((const UTF8Char*)"Elevation Filter", 0);
	this->cboFilter->SetSelectedIndex(0);
	NEW_CLASS(this->btnOK, UI::GUIButton(ui, this, (const UTF8Char*)"OK"));
	this->btnOK->SetRect(8, 56, 75, 23, false);
	this->btnOK->HandleButtonClick(OnOKClicked, this);
	NEW_CLASS(this->btnCancel, UI::GUIButton(ui, this, (const UTF8Char*)"Cancel"));
	this->btnCancel->SetRect(120, 56, 75, 23, false);
	this->btnCancel->HandleButtonClick(OnCancelClicked, this);
}

SSWR::SHPConv::SHPConvAddFilterForm::~SHPConvAddFilterForm()
{
	DEL_CLASS(this->monMgr);
}

void SSWR::SHPConv::SHPConvAddFilterForm::OnMonitorChanged()
{
	this->SetDPI(this->monMgr->GetMonitorHDPI(this->GetHMonitor()), this->monMgr->GetMonitorDDPI(this->GetHMonitor()));
}

SSWR::SHPConv::MapFilter *SSWR::SHPConv::SHPConvAddFilterForm::GetFilter()
{
	return this->filter;
}
