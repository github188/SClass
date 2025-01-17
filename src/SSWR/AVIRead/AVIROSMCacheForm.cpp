#include "Stdafx.h"
#include "SSWR/AVIRead/AVIROSMCacheForm.h"

void __stdcall SSWR::AVIRead::AVIROSMCacheForm::OnTimerTick(void *userObj)
{
	SSWR::AVIRead::AVIROSMCacheForm *me = (SSWR::AVIRead::AVIROSMCacheForm*)userObj;
	UTF8Char sbuff[32];
	Map::OSM::OSMCacheHandler::CacheStatus status;
	me->hdlr->GetStatus(&status);
	Text::StrInt32(sbuff, status.reqCnt);
	me->txtReqCnt->SetText(sbuff);
	Text::StrInt32(sbuff, status.remoteSuccCnt);
	me->txtRemoteSuccCnt->SetText(sbuff);
	Text::StrInt32(sbuff, status.remoteErrCnt);
	me->txtRemoteErrCnt->SetText(sbuff);
	Text::StrInt32(sbuff, status.localCnt);
	me->txtLocalCnt->SetText(sbuff);
	Text::StrInt32(sbuff, status.cacheCnt);
	me->txtCacheCnt->SetText(sbuff);
}

SSWR::AVIRead::AVIROSMCacheForm::AVIROSMCacheForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, Net::WebServer::WebListener *listener, Map::OSM::OSMCacheHandler *hdlr) : UI::GUIForm(parent, 1024, 768, ui)
{
	this->SetText((const UTF8Char*)"OSM Cache");
	this->SetFont(0, 8.25, false);

	this->core = core;
	this->listener = listener;
	this->hdlr = hdlr;

	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));

	NEW_CLASS(this->lblReqCnt, UI::GUILabel(ui, this, (const UTF8Char*)"Request Cnt"));
	this->lblReqCnt->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtReqCnt, UI::GUITextBox(ui, this, (const UTF8Char*)""));
	this->txtReqCnt->SetRect(104, 4, 150, 23, false);
	this->txtReqCnt->SetReadOnly(true);
	NEW_CLASS(this->lblRemoteSuccCnt, UI::GUILabel(ui, this, (const UTF8Char*)"Remote Succ"));
	this->lblRemoteSuccCnt->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->txtRemoteSuccCnt, UI::GUITextBox(ui, this, (const UTF8Char*)""));
	this->txtRemoteSuccCnt->SetRect(104, 28, 150, 23, false);
	this->txtRemoteSuccCnt->SetReadOnly(true);
	NEW_CLASS(this->lblRemoteErrCnt, UI::GUILabel(ui, this, (const UTF8Char*)"Remote Err"));
	this->lblRemoteErrCnt->SetRect(4, 52, 100, 23, false);
	NEW_CLASS(this->txtRemoteErrCnt, UI::GUITextBox(ui, this, (const UTF8Char*)""));
	this->txtRemoteErrCnt->SetRect(104, 52, 150, 23, false);
	this->txtRemoteErrCnt->SetReadOnly(true);
	NEW_CLASS(this->lblLocalCnt, UI::GUILabel(ui, this, (const UTF8Char*)"Local Cnt"));
	this->lblLocalCnt->SetRect(4, 76, 100, 23, false);
	NEW_CLASS(this->txtLocalCnt, UI::GUITextBox(ui, this, (const UTF8Char*)""));
	this->txtLocalCnt->SetRect(104, 76, 150, 23, false);
	this->txtLocalCnt->SetReadOnly(true);
	NEW_CLASS(this->lblCacheCnt, UI::GUILabel(ui, this, (const UTF8Char*)"Cache Cnt"));
	this->lblCacheCnt->SetRect(4, 100, 100, 23, false);
	NEW_CLASS(this->txtCacheCnt, UI::GUITextBox(ui, this, (const UTF8Char*)""));
	this->txtCacheCnt->SetRect(104, 100, 150, 23, false);
	this->txtCacheCnt->SetReadOnly(true);

	this->AddTimer(1000, OnTimerTick, this);
}

SSWR::AVIRead::AVIROSMCacheForm::~AVIROSMCacheForm()
{
	DEL_CLASS(this->listener);
}

void SSWR::AVIRead::AVIROSMCacheForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}
