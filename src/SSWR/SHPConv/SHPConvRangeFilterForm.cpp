#include "Stdafx.h"
#include "SSWR/SHPConv/RangeFilter.h"
#include "SSWR/SHPConv/SHPConvRangeFilterForm.h"
#include "Text/MyStringFloat.h"
#include "Text/StringBuilderUTF8.h"
#include "UI/MessageDialog.h"

void __stdcall SSWR::SHPConv::SHPConvRangeFilterForm::OnOKClicked(void *userObj)
{
	SSWR::SHPConv::SHPConvRangeFilterForm *me = (SSWR::SHPConv::SHPConvRangeFilterForm*)userObj;
	Double x1;
	Double y1;
	Double x2;
	Double y2;
	Text::StringBuilderUTF8 sb;

	sb.ClearStr();
	me->txtX1->GetText(&sb);
	if (!Text::StrToDouble(sb.ToString(), &x1))
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Invalid input values", (const UTF8Char*)"Error", me);
		return;
	}
	sb.ClearStr();
	me->txtY1->GetText(&sb);
	if (!Text::StrToDouble(sb.ToString(), &y1))
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Invalid input values", (const UTF8Char*)"Error", me);
		return;
	}
	sb.ClearStr();
	me->txtX2->GetText(&sb);
	if (!Text::StrToDouble(sb.ToString(), &x2))
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Invalid input values", (const UTF8Char*)"Error", me);
		return;
	}
	sb.ClearStr();
	me->txtY2->GetText(&sb);
	if (!Text::StrToDouble(sb.ToString(), &y2))
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Invalid input values", (const UTF8Char*)"Error", me);
		return;
	}

	if (x1 >= x2)
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"X value is not valid", (const UTF8Char*)"Error", me);
		return;
	}
	if (y1 >= y2)
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Y value is not valid", (const UTF8Char*)"Error", me);
		return;
	}
	Int32 typ;
	if (me->radInRange->IsSelected())
	{
		typ = 1;
	}
	else
	{
		typ = 0;
	}
	NEW_CLASS(me->filter, RangeFilter(x1, y1, x2, y2, typ));
	me->SetDialogResult(UI::GUIForm::DR_OK);
}

void __stdcall SSWR::SHPConv::SHPConvRangeFilterForm::OnCancelClicked(void *userObj)
{
	SSWR::SHPConv::SHPConvRangeFilterForm *me = (SSWR::SHPConv::SHPConvRangeFilterForm*)userObj;
	me->SetDialogResult(UI::GUIForm::DR_CANCEL);
}

SSWR::SHPConv::SHPConvRangeFilterForm::SHPConvRangeFilterForm(UI::GUIClientControl *parent, UI::GUICore *ui, Media::DrawEngine *deng) : UI::GUIForm(parent, 350, 232, ui)
{
	this->SetText((const UTF8Char*)"Range Filter");
	this->SetFont(0, 8.25, false);
	this->SetNoResize(true);
	NEW_CLASS(this->monMgr, Media::MonitorMgr());
	this->SetDPI(this->monMgr->GetMonitorHDPI(this->GetHMonitor()), this->monMgr->GetMonitorDDPI(this->GetHMonitor()));
	this->deng = deng;
	this->filter = 0;
	
	NEW_CLASS(this->radInRange, UI::GUIRadioButton(ui, this, (const UTF8Char*)"In Range", true));
	this->radInRange->SetRect(8, 8, 100, 23, false);
	NEW_CLASS(this->radOutRange, UI::GUIRadioButton(ui, this, (const UTF8Char*)"Out Range", false));
	this->radOutRange->SetRect(112, 8, 100, 23, false);
	NEW_CLASS(this->lblX1, UI::GUILabel(ui, this, (const UTF8Char*)"X"));
	this->lblX1->SetRect(8, 32, 16, 23, false);
	NEW_CLASS(this->txtX1, UI::GUITextBox(ui, this, (const UTF8Char*)"0"));
	this->txtX1->SetRect(24, 32, 80, 20, false);
	NEW_CLASS(this->lblY1, UI::GUILabel(ui, this, (const UTF8Char*)"Y"));
	this->lblY1->SetRect(112, 32, 16, 23, false);
	NEW_CLASS(this->txtY1, UI::GUITextBox(ui, this, (const UTF8Char*)"0"));
	this->txtY1->SetRect(128, 32, 80, 20, false);
	NEW_CLASS(this->pbBox, UI::GUIPictureBoxSimple(ui, this, this->deng, true));
	this->pbBox->SetRect(56, 64, 216, 96, false);
	NEW_CLASS(this->lblX2, UI::GUILabel(ui, this, (const UTF8Char*)"X"));
	this->lblX2->SetRect(144, 176, 16, 23, false);
	NEW_CLASS(this->txtX2, UI::GUITextBox(ui, this, (const UTF8Char*)"0"));
	this->txtX2->SetRect(160, 176, 80, 20, false);
	NEW_CLASS(this->lblY2, UI::GUILabel(ui, this, (const UTF8Char*)"Y"));
	this->lblY2->SetRect(248, 176, 16, 23, false);
	NEW_CLASS(this->txtY2, UI::GUITextBox(ui, this, (const UTF8Char*)"0"));
	this->txtY2->SetRect(264, 176, 80, 20, false);
	NEW_CLASS(this->btnOK, UI::GUIButton(ui, this, (const UTF8Char*)"OK"));
	this->btnOK->SetRect(264, 8, 75, 23, false);
	this->btnOK->HandleButtonClick(OnOKClicked, this);
	NEW_CLASS(this->btnCancel, UI::GUIButton(ui, this, (const UTF8Char*)"Cancel"));
	this->btnCancel->SetRect(264, 32, 75, 23, false);
	this->btnCancel->HandleButtonClick(OnCancelClicked, this);
	this->SetDefaultButton(this->btnOK);
	this->SetCancelButton(this->btnCancel);
}

SSWR::SHPConv::SHPConvRangeFilterForm::~SHPConvRangeFilterForm()
{
	DEL_CLASS(this->monMgr);
}

void SSWR::SHPConv::SHPConvRangeFilterForm::OnMonitorChanged()
{
	this->SetDPI(this->monMgr->GetMonitorHDPI(this->GetHMonitor()), this->monMgr->GetMonitorDDPI(this->GetHMonitor()));
}

SSWR::SHPConv::MapFilter *SSWR::SHPConv::SHPConvRangeFilterForm::GetFilter()
{
	return this->filter;
}
