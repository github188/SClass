#include "Stdafx.h"
#include "SSWR/AVIRead/AVIRSectorForm.h"

void __stdcall SSWR::AVIRead::AVIRSectorForm::OnParseClicked(void *userObj)
{
	SSWR::AVIRead::AVIRSectorForm *me = (SSWR::AVIRead::AVIRSectorForm *)userObj;
	Parser::ParserList *parsers = me->core->GetParserList();
	IO::ParsedObject *pobj;
	IO::ParsedObject::ParserType pt;
	pobj = parsers->ParseObject(me->data, &pt);
	if (pobj)
	{
		me->core->OpenObject(pobj);
	}
}

SSWR::AVIRead::AVIRSectorForm::AVIRSectorForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, IO::ISectorData *data) : UI::GUIForm(parent, 1024, 768, ui)
{
	UTF8Char sbuff[512];
	this->data = data;
	this->core = core;
	Text::StrConcat(Text::StrConcat(sbuff, (const UTF8Char*)"Sector Viewer - "), data->GetSourceNameObj());
	this->SetText(sbuff);
	this->SetFont(0, 8.25, false);
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));

	NEW_CLASS(this->pnlCtrl, UI::GUIPanel(ui, this));
	this->pnlCtrl->SetRect(0, 0, 100, 72, false);
	this->pnlCtrl->SetDockType(UI::GUIControl::DOCK_BOTTOM);
	NEW_CLASS(this->btnParse, UI::GUIButton(ui, this->pnlCtrl, (const UTF8Char*)"Parse File"));
	this->btnParse->SetRect(4, 4, 75, 23, false);
	this->btnParse->HandleButtonClick(OnParseClicked, this);

	this->sectorSize = this->data->GetBytesPerSector();
	this->sectorData = MemAlloc(UInt8, this->sectorSize);
}

SSWR::AVIRead::AVIRSectorForm::~AVIRSectorForm()
{
	MemFree(this->sectorData);
	SDEL_CLASS(this->data);
}

void SSWR::AVIRead::AVIRSectorForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}
