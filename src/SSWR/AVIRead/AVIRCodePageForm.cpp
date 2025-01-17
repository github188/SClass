#include "Stdafx.h"
#include "Text/MyString.h"
#include "IO/EXEFile.h"
#include "IO/Path.h"
#include "UI/GUILabel.h"
#include "UI/GUIButton.h"
#include "SSWR/AVIRead/AVIRCodePageForm.h"

void __stdcall SSWR::AVIRead::AVIRCodePageForm::OKClicked(void *userObj)
{
	SSWR::AVIRead::AVIRCodePageForm *me = (SSWR::AVIRead::AVIRCodePageForm*)userObj;
	UOSInt selInd = me->lbCodePages->GetSelectedIndex();
	if (selInd != INVALID_INDEX)
	{
		me->core->SetCodePage((UInt32)(UOSInt)me->lbCodePages->GetItem(selInd));
		me->SetDialogResult(UI::GUIForm::DR_OK);
	}
}

void __stdcall SSWR::AVIRead::AVIRCodePageForm::CancelClicked(void *userObj)
{
	SSWR::AVIRead::AVIRCodePageForm *me = (SSWR::AVIRead::AVIRCodePageForm*)userObj;
	me->SetDialogResult(UI::GUIForm::DR_CANCEL);
}

SSWR::AVIRead::AVIRCodePageForm::AVIRCodePageForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core) : UI::GUIForm(parent, 260, 664, ui)
{
	this->SetText((const UTF8Char*)"Select Code Page");
	this->SetFont(0, 8.25, false);
	this->SetNoResize(true);
	this->core = core;
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
	
	UTF8Char sbuff[512];
	UI::GUILabel *lbl;
	
	NEW_CLASS(lbl, UI::GUILabel(ui, this, (const UTF8Char*)"Code Pages"));
	lbl->SetRect(8, 8, 120, 23, false);

	NEW_CLASS(this->lbCodePages, UI::GUIListBox(ui, this, false));
	this->lbCodePages->SetRect(8, 32, 240, 560, false);

	NEW_CLASS(btnOK, UI::GUIButton(ui, this, (const UTF8Char*)"OK"));
	btnOK->SetRect(40, 600, 75, 23, false);
	btnOK->HandleButtonClick(OKClicked, this);
	NEW_CLASS(btnCancel, UI::GUIButton(ui, this, (const UTF8Char*)"Cancel"));
	btnCancel->SetRect(128, 600, 75, 23, false);
	btnCancel->HandleButtonClick(CancelClicked, this);
	this->SetDefaultButton(btnOK);
	this->SetCancelButton(btnCancel);

	UOSInt i;
	UOSInt j;
	UOSInt ind;
	UInt32 codePage;
	UInt32 currCodePage = this->core->GetCurrCodePage();
	Data::ArrayListUInt32 codePages;
	Text::EncodingFactory::GetCodePages(&codePages);
	i = 0;
	j = codePages.GetCount();
	while (i < j)
	{
		codePage = codePages.GetItem(i);
		Text::EncodingFactory::GetName(Text::StrConcat(Text::StrUInt32(sbuff, codePage), (const UTF8Char*)" "), codePage);
		ind = this->lbCodePages->AddItem(sbuff, (void*)(OSInt)codePage);
		if (codePage == currCodePage)
		{
			this->lbCodePages->SetSelectedIndex(ind);
		}
		i++;
	}
	this->lbCodePages->Focus();
}

SSWR::AVIRead::AVIRCodePageForm::~AVIRCodePageForm()
{
}

void SSWR::AVIRead::AVIRCodePageForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}
