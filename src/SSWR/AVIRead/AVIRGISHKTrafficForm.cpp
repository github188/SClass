#include "Stdafx.h"
#include "IO/StmData/FileData.h"
#include "Map/HKTrafficLayer.h"
#include "SSWR/AVIRead/AVIRGISHKTrafficForm.h"
#include "Text/StringBuilderUTF8.h"
#include "UI/FileDialog.h"
#include "UI/MessageDialog.h"

void __stdcall SSWR::AVIRead::AVIRGISHKTrafficForm::OnRoadCenterlineClicked(void *userObj)
{
	SSWR::AVIRead::AVIRGISHKTrafficForm *me = (SSWR::AVIRead::AVIRGISHKTrafficForm*)userObj;
	Text::StringBuilderUTF8 sb;
	UI::FileDialog *dlg;
	me->txtRoadCenterline->GetText(&sb);
	NEW_CLASS(dlg, UI::FileDialog(L"SSWR", L"AVIRead", L"HKTraffic", false));
	if (sb.GetLength() > 0)
	{
		dlg->SetFileName(sb.ToString());
	}
	dlg->SetAllowMultiSel(false);
	me->core->GetParserList()->PrepareSelector(dlg, IO::ParsedObject::PT_MAP_LAYER_PARSER);
	if (dlg->ShowDialog(me->GetHandle()))
	{
		me->txtRoadCenterline->SetText(dlg->GetFileName());
	}
	DEL_CLASS(dlg);
}

void __stdcall SSWR::AVIRead::AVIRGISHKTrafficForm::OnOKClicked(void *userObj)
{
	SSWR::AVIRead::AVIRGISHKTrafficForm *me = (SSWR::AVIRead::AVIRGISHKTrafficForm*)userObj;
	Text::StringBuilderUTF8 sb;
	me->txtRoadCenterline->GetText(&sb);
	IO::StmData::FileData *fd;
	NEW_CLASS(fd, IO::StmData::FileData(sb.ToString(), false));
	Map::IMapDrawLayer *lyr = (Map::IMapDrawLayer*)me->core->GetParserList()->ParseFileType(fd, IO::ParsedObject::PT_MAP_LAYER_PARSER);
	DEL_CLASS(fd);
	if (lyr)
	{
		Map::DrawLayerType lyrType = lyr->GetLayerType();
		if (lyrType == Map::DRAW_LAYER_POLYLINE || lyrType == Map::DRAW_LAYER_POLYLINE3D)
		{
			Map::HKTrafficLayer *traffic;
			NEW_CLASS(traffic, Map::HKTrafficLayer(me->core->GetSocketFactory(), me->core->GetSSLEngine(), me->core->GetEncFactory()));
			traffic->AddRoadLayer(lyr);
			traffic->EndInit();
			me->lyr = traffic;
			me->SetDialogResult(UI::GUIForm::DR_OK);
		}
		else
		{
			DEL_CLASS(lyr);
			UI::MessageDialog::ShowDialog((const UTF8Char*)"The file is not a polyline layer", (const UTF8Char*)"HK Traffic", me);
		}
	}
	else
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Error in parsing the file", (const UTF8Char*)"HK Traffic", me);
	}
}

void __stdcall SSWR::AVIRead::AVIRGISHKTrafficForm::OnCancelClicked(void *userObj)
{
	SSWR::AVIRead::AVIRGISHKTrafficForm *me = (SSWR::AVIRead::AVIRGISHKTrafficForm*)userObj;
	me->SetDialogResult(UI::GUIForm::DR_CANCEL);
}

SSWR::AVIRead::AVIRGISHKTrafficForm::AVIRGISHKTrafficForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core) : UI::GUIForm(parent, 1024, 120, ui)
{
	this->SetText((const UTF8Char*)"Hong Kong Traffic");
	this->SetFont(0, 8.25, false);
	this->SetNoResize(true);

	this->core = core;
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
	this->lyr = 0;

	NEW_CLASS(this->lblRoadCenterline, UI::GUILabel(ui, this, (const UTF8Char*)"Centerline"));
	this->lblRoadCenterline->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtRoadCenterline, UI::GUITextBox(ui, this, (const UTF8Char*)"L:\\Backup\\PROGS_20101124\\PROGS\\Skypower\\0_req\\20180628 Road Traffic\\CENTRELINE.mdb"));
	this->txtRoadCenterline->SetRect(104, 4, 800, 23, false);
	NEW_CLASS(this->btnRoadCenterline, UI::GUIButton(ui, this, (const UTF8Char*)"Browse"));
	this->btnRoadCenterline->SetRect(904, 4, 75, 23, false);
	this->btnRoadCenterline->HandleButtonClick(OnRoadCenterlineClicked, this);
	NEW_CLASS(this->btnCancel, UI::GUIButton(ui, this, (const UTF8Char*)"Cancel"));
	this->btnCancel->SetRect(104, 28, 75, 23, false);
	this->btnCancel->HandleButtonClick(OnCancelClicked, this);
	NEW_CLASS(this->btnOK, UI::GUIButton(ui, this, (const UTF8Char*)"OK"));
	this->btnOK->SetRect(184, 28, 75, 23, false);
	this->btnOK->HandleButtonClick(OnOKClicked, this);
}

SSWR::AVIRead::AVIRGISHKTrafficForm::~AVIRGISHKTrafficForm()
{
}

void SSWR::AVIRead::AVIRGISHKTrafficForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}

Map::IMapDrawLayer *SSWR::AVIRead::AVIRGISHKTrafficForm::GetMapLayer()
{
	return this->lyr;
}
