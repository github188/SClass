#include "Stdafx.h"
#include "Net/UserAgentDB.h"
#include "SSWR/AVIRead/AVIRWebSiteTwitterForm.h"

void __stdcall SSWR::AVIRead::AVIRWebSiteTwitterForm::OnRequestPageClicked(void *userObj)
{
	SSWR::AVIRead::AVIRWebSiteTwitterForm *me = (SSWR::AVIRead::AVIRWebSiteTwitterForm*)userObj;
	Text::StringBuilderUTF8 sb;
	UTF8Char sbuff[64];
	me->txtChannelId->GetText(&sb);
	me->lvItems->ClearItems();
	if (sb.GetLength() > 0)
	{
		Data::DateTime dt;
		UOSInt i;
		UOSInt j;
		Data::ArrayList<Net::WebSite::WebSiteTwitterControl::ItemData*> itemList;
		Net::WebSite::WebSiteTwitterControl::ItemData *item;
		me->ctrl->GetChannelItems(sb.ToString(), 0, &itemList, 0);
		i = 0;
		j = itemList.GetCount();
		while (i < j)
		{
			item = itemList.GetItem(i);
			Text::StrInt64(sbuff, item->id);
			me->lvItems->AddItem(sbuff, 0);
			dt.SetTicks(item->recTime);
			dt.ToLocalTime();
			dt.ToString(sbuff, "yyyy-MM-dd HH:mm:ss");
			me->lvItems->SetSubItem(i, 1, sbuff);
			if (item->imgURL)
			{
				sb.ClearStr();
				sb.Append(item->imgURL);
				sb.Replace(' ', '\n');
				me->lvItems->SetSubItem(i, 2, sb.ToString());
			}
			sb.ClearStr();
			sb.Append(item->message);
			sb.Replace('\r', ' ');
			sb.Replace('\n', ' ');
			me->lvItems->SetSubItem(i, 3, sb.ToString());
			i++;
		}
		me->ctrl->FreeItems(&itemList);
	}
}

SSWR::AVIRead::AVIRWebSiteTwitterForm::AVIRWebSiteTwitterForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core) : UI::GUIForm(parent, 1024, 768, ui)
{
	this->SetText((const UTF8Char*)"Twitter");
	this->SetFont((const UTF8Char*)"MingLiu", 8.25, false);

	this->core = core;
	const UTF8Char *userAgent = 0;//Net::UserAgentDB::FindUserAgent(Manage::OSInfo::OT_WINDOWS_NT64, Net::BrowserInfo::BT_FIREFOX);
	NEW_CLASS(this->ctrl, Net::WebSite::WebSiteTwitterControl(core->GetSocketFactory(), core->GetSSLEngine(), core->GetEncFactory(), userAgent));
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));

	NEW_CLASS(this->pnlRequest, UI::GUIPanel(ui, this));
	this->pnlRequest->SetRect(0, 0, 100, 31, false);
	this->pnlRequest->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblChannelId, UI::GUILabel(ui, this->pnlRequest, (const UTF8Char*)"ChannelId"));
	this->lblChannelId->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtChannelId, UI::GUITextBox(ui, this->pnlRequest, (const UTF8Char*)""));
	this->txtChannelId->SetRect(104, 4, 150, 23, false);
	NEW_CLASS(this->btnRequestPage, UI::GUIButton(ui, this->pnlRequest, (const UTF8Char*)"Request"));
	this->btnRequestPage->SetRect(254, 4, 75, 23, false);
	this->btnRequestPage->HandleButtonClick(OnRequestPageClicked, this);
	NEW_CLASS(this->lvItems, UI::GUIListView(ui, this, UI::GUIListView::LVSTYLE_TABLE, 4));
	this->lvItems->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvItems->SetFullRowSelect(true);
	this->lvItems->SetShowGrid(true);
	this->lvItems->AddColumn((const UTF8Char*)"Id", 100);
	this->lvItems->AddColumn((const UTF8Char*)"Date", 120);
	this->lvItems->AddColumn((const UTF8Char*)"ImageURL", 200);
	this->lvItems->AddColumn((const UTF8Char*)"Message", 400);
}

SSWR::AVIRead::AVIRWebSiteTwitterForm::~AVIRWebSiteTwitterForm()
{
	DEL_CLASS(this->ctrl);
}

void SSWR::AVIRead::AVIRWebSiteTwitterForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}
