#include "Stdafx.h"
#include "IO/EXEFile.h"
#include "IO/Path.h"
#include "SSWR/AVIRead/AVIRAboutForm.h"
#include "Text/MyString.h"
#include "UI/GUIButton.h"
#include "UI/GUILabel.h"

#if defined(CPU_X86_32) || defined(CPU_X86_64)
extern "C" Int32 UseAVX;
extern "C" Int32 CPUBrand;
#endif

void __stdcall SSWR::AVIRead::AVIRAboutForm::OKClicked(void *userObj)
{
	SSWR::AVIRead::AVIRAboutForm *me = (SSWR::AVIRead::AVIRAboutForm*)userObj;
	me->Close();
}

SSWR::AVIRead::AVIRAboutForm::AVIRAboutForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core) : UI::GUIForm(parent, 480, 144, ui)
{
	this->SetText((const UTF8Char*)"About");
	this->SetFont(0, 8.25, false);
	this->SetNoResize(true);
	
	this->core = core;
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));

	UTF8Char sbuff[512];
	UI::GUILabel *lbl;
	Data::DateTime t;
#if defined(CPU_X86_32) || defined(CPU_X86_64)
	UTF8Char *sptr;
	sptr = Text::StrConcat(sbuff, (const UTF8Char*)"AVIRead v4 by sswroom");
	if (CPUBrand == 1)
	{
		sptr = Text::StrConcat(sptr, (const UTF8Char*)" (Optimized for Intel CPU)");
	}
	else if (CPUBrand == 2)
	{
		sptr = Text::StrConcat(sptr, (const UTF8Char*)" (AMD CPU)");
	}
	if (UseAVX)
	{
		sptr = Text::StrConcat(sptr, (const UTF8Char*)" (AVX Optimized)");
	}
#else
	Text::StrConcat(sbuff, (const UTF8Char*)"AVIRead v4 by sswroom");
#endif
	NEW_CLASS(lbl, UI::GUILabel(ui, this, sbuff));
	lbl->SetRect(8, 8, 400, 23, false);

	IO::Path::GetProcessFileName(Text::StrConcat(sbuff, (const UTF8Char*)"Path: "));
	NEW_CLASS(lbl, UI::GUILabel(ui, this, sbuff));
	lbl->SetRect(8, 32, 400, 23, false);

	IO::Path::GetProcessFileName(sbuff);
	IO::EXEFile::GetFileTime(sbuff, &t);
	t.ToString(Text::StrConcat(sbuff, (const UTF8Char*)"Build date: "), "yyyy-MM-dd HH:mm:ss");
	NEW_CLASS(lbl, UI::GUILabel(ui, this, sbuff));
	lbl->SetRect(8, 56, 400, 23, false);

	NEW_CLASS(btn, UI::GUIButton(ui, this, (const UTF8Char*)"OK"));
	btn->SetRect(385, 74, 75, 23, false);
	btn->HandleButtonClick(OKClicked, this);
}

SSWR::AVIRead::AVIRAboutForm::~AVIRAboutForm()
{
}

void SSWR::AVIRead::AVIRAboutForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}
