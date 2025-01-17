#include "Stdafx.h"
#include "Net/UserAgentDB.h"
#include "SSWR/AVIRead/AVIRUserAgentParseForm.h"
#include "Text/StringBuilderUTF8.h"

void __stdcall SSWR::AVIRead::AVIRUserAgentParseForm::OnParseClicked(void *userObj)
{
	SSWR::AVIRead::AVIRUserAgentParseForm *me = (SSWR::AVIRead::AVIRUserAgentParseForm*)userObj;
	Text::StringBuilderUTF8 sb;
	me->txtUserAgent->GetText(&sb);
	if (sb.GetLength() > 0)
	{
		Net::UserAgentDB::UAEntry ent;
		Net::UserAgentDB::ParseUserAgent(&ent, sb.ToString());
		me->txtBrowser->SetText(Net::BrowserInfo::GetName(ent.browser));
		me->txtBrowserVer->SetText(ent.browserVer?((const UTF8Char*)ent.browserVer):((const UTF8Char*)"-"));
		me->txtOS->SetText(Manage::OSInfo::GetName(ent.os));
		me->txtOSVer->SetText(ent.osVer?((const UTF8Char*)ent.osVer):((const UTF8Char*)"-"));
		me->txtDeviceName->SetText(ent.devName?((const UTF8Char*)ent.devName):((const UTF8Char*)"-"));
		SDEL_TEXT(ent.browserVer);
		SDEL_TEXT(ent.osVer);
		SDEL_TEXT(ent.devName);
	}
}

SSWR::AVIRead::AVIRUserAgentParseForm::AVIRUserAgentParseForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core) : UI::GUIForm(parent, 800, 200, ui)
{
	this->SetFont(0, 8.25, false);
	this->SetText((const UTF8Char*)"User Agent Parse");
	this->SetNoResize(true);
	
	this->core = core;

	NEW_CLASS(this->lblUserAgent, UI::GUILabel(ui, this, (const UTF8Char*)"User Agent"));
	this->lblUserAgent->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtUserAgent, UI::GUITextBox(ui, this, (const UTF8Char*)""));
	this->txtUserAgent->SetRect(104, 4, 600, 23, false);
	NEW_CLASS(this->btnParse, UI::GUIButton(ui, this, (const UTF8Char*)"Parse"));
	this->btnParse->SetRect(104, 28, 75, 23, false);
	this->btnParse->HandleButtonClick(OnParseClicked, this);
	NEW_CLASS(this->lblBrowser, UI::GUILabel(ui, this, (const UTF8Char*)"Browser"));
	this->lblBrowser->SetRect(4, 52, 100, 23, false);
	NEW_CLASS(this->txtBrowser, UI::GUITextBox(ui, this, (const UTF8Char*)""));
	this->txtBrowser->SetRect(104, 52, 200, 23, false);
	this->txtBrowser->SetReadOnly(true);
	NEW_CLASS(this->lblBrowserVer, UI::GUILabel(ui, this, (const UTF8Char*)"Browser Ver"));
	this->lblBrowserVer->SetRect(4, 76, 100, 23, false);
	NEW_CLASS(this->txtBrowserVer, UI::GUITextBox(ui, this, (const UTF8Char*)""));
	this->txtBrowserVer->SetRect(104, 76, 200, 23, false);
	this->txtBrowserVer->SetReadOnly(true);
	NEW_CLASS(this->lblOS, UI::GUILabel(ui, this, (const UTF8Char*)"OS"));
	this->lblOS->SetRect(4, 100, 100, 23, false);
	NEW_CLASS(this->txtOS, UI::GUITextBox(ui, this, (const UTF8Char*)""));
	this->txtOS->SetRect(104, 100, 200, 23, false);
	this->txtOS->SetReadOnly(true);
	NEW_CLASS(this->lblOSVer, UI::GUILabel(ui, this, (const UTF8Char*)"OS Version"));
	this->lblOSVer->SetRect(4, 124, 100, 23, false);
	NEW_CLASS(this->txtOSVer, UI::GUITextBox(ui, this, (const UTF8Char*)""));
	this->txtOSVer->SetRect(104, 124, 200, 23, false);
	this->txtOSVer->SetReadOnly(true);
	NEW_CLASS(this->lblDeviceName, UI::GUILabel(ui, this, (const UTF8Char*)"Device Name"));
	this->lblDeviceName->SetRect(4, 148, 100, 23, false);
	NEW_CLASS(this->txtDeviceName, UI::GUITextBox(ui, this, (const UTF8Char*)""));
	this->txtDeviceName->SetRect(104, 148, 200, 23, false);
	this->txtDeviceName->SetReadOnly(true);

	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}

SSWR::AVIRead::AVIRUserAgentParseForm::~AVIRUserAgentParseForm()
{
}

void SSWR::AVIRead::AVIRUserAgentParseForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}
