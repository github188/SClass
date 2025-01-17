#include "Stdafx.h"
#include "Data/ByteTool.h"
#include "Net/ASN1OIDDB.h"
#include "Net/ASN1Util.h"
#include "Net/ConnectionInfo.h"
#include "Net/MACInfo.h"
#include "SSWR/AVIRead/AVIRSNMPManagerForm.h"
#include "SSWR/AVIRead/AVIRSNMPWalkForm.h"
#include "Sync/Thread.h"
#include "Text/MyStringFloat.h"
#include "UI/MessageDialog.h"

void __stdcall SSWR::AVIRead::AVIRSNMPManagerForm::OnAgentAddClicked(void *userObj)
{
	SSWR::AVIRead::AVIRSNMPManagerForm *me = (SSWR::AVIRead::AVIRSNMPManagerForm*)userObj;
	Text::StringBuilderUTF8 sb;
	Net::SocketUtil::AddressInfo addr;
	UTF8Char sbuff[128];
	me->txtAgentAddr->GetText(&sb);
	if (!me->core->GetSocketFactory()->DNSResolveIP(sb.ToString(), &addr))
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Error in parsing Agent Address", (const UTF8Char*)"SNMP Manager", me);
		return;
	}

	sb.ClearStr();
	me->txtCommunity->GetText(&sb);
	if (sb.GetLength() <= 0)
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Please enter community", (const UTF8Char*)"SNMP Manager", me);
		return;
	}
	UOSInt i;
	UOSInt j;
	UOSInt k;
	Data::ArrayList<Net::SNMPManager::AgentInfo *> agentList;
	j = me->mgr->AddAgents(&addr, sb.ToString(), &agentList, me->chkAgentScan->IsChecked());
	if (j <= 0)
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Error in Adding Agent", (const UTF8Char*)"SNMP Manager", me);
	}
	else
	{
		Net::SNMPManager::AgentInfo *agent;
		if (me->chkSendToSvr->IsChecked())
		{
			Int64 cliId;
			UOSInt l;
			Net::SNMPManager::ReadingInfo *reading;
			Data::UInt32Map<UInt16> readingMap;
			UInt16 currId;
			me->SendAgentValues(&agentList);
			Sync::Thread::Sleep(100);
			i = 0;
			while (i < j)
			{
				agent = agentList.GetItem(i);
				cliId = me->mgr->Agent2CliId(agent);
				if (agent->name)
				{
					me->redir->SendDevName(cliId, agent->name);
				}
				if (agent->model)
				{
					if (agent->vendor)
					{
						Text::StringBuilderUTF8 sbPlatform;
						sbPlatform.Append(agent->vendor);
						sbPlatform.AppendChar(' ', 1);
						sbPlatform.Append(agent->model);
						me->redir->SendDevPlatform(cliId, sbPlatform.ToString());
					}
					else
					{
						me->redir->SendDevPlatform(cliId, agent->model);
					}
				}
				if (agent->cpuName)
				{
					me->redir->SendDevPlatform(cliId, agent->cpuName);
				}
				k = 0;
				l = agent->readingList->GetCount();
				while (k < l)
				{
					reading = agent->readingList->GetItem(k);
					currId = readingMap.Get((UInt32)reading->index);
					readingMap.Put((UInt32)reading->index, (UInt16)(currId + 1));
					if (reading->name)
					{
						me->redir->SendDevReadingName(cliId, k, (UInt16)reading->index, currId, reading->name);
					}
					k++;
				}
				i++;
			}
		}
		i = 0;
		while (i < j)
		{
			agent = agentList.GetItem(i);
			Net::SocketUtil::GetAddrName(sbuff, &agent->addr);
			k = me->lbAgent->AddItem(sbuff, agent);
			if (i == 0)
			{
				me->lbAgent->SetSelectedIndex(k);
			}
			i++;
		}
	}
}

void __stdcall SSWR::AVIRead::AVIRSNMPManagerForm::OnAgentSelChg(void *userObj)
{
	UTF8Char sbuff[128];
	SSWR::AVIRead::AVIRSNMPManagerForm *me = (SSWR::AVIRead::AVIRSNMPManagerForm*)userObj;
	Net::SNMPManager::AgentInfo *agent = (Net::SNMPManager::AgentInfo*)me->lbAgent->GetSelectedItem();
	if (agent)
	{
		Net::SocketUtil::GetAddrName(sbuff, &agent->addr);
		me->txtAgentDAddr->SetText(sbuff);
		if (agent->descr)
		{
			me->txtAgentDescr->SetText(agent->descr);
		}
		else
		{
			me->txtAgentDescr->SetText((const UTF8Char*)"");
		}
		if (agent->objIdLen > 0)
		{
			Text::StringBuilderUTF8 sb;
			Net::ASN1Util::OIDToString(agent->objId, agent->objIdLen, &sb);
			me->txtAgentOID->SetText(sb.ToString());
			sb.ClearStr();
			Net::ASN1OIDDB::OIDToNameString(agent->objId, agent->objIdLen, &sb);
			me->txtAgentOIDName->SetText(sb.ToString());
		}
		else
		{
			me->txtAgentOID->SetText((const UTF8Char*)"");
			me->txtAgentOIDName->SetText((const UTF8Char*)"");
		}
		if (agent->name)
		{
			me->txtAgentName->SetText(agent->name);
		}
		else
		{
			me->txtAgentName->SetText((const UTF8Char*)"");
		}
		if (agent->contact)
		{
			me->txtAgentContact->SetText(agent->contact);
		}
		else
		{
			me->txtAgentContact->SetText((const UTF8Char*)"");
		}
		if (agent->location)
		{
			me->txtAgentLocation->SetText(agent->location);
		}
		else
		{
			me->txtAgentLocation->SetText((const UTF8Char*)"");
		}
		Text::StrHexBytes(sbuff, agent->mac, 6, ':');
		me->txtAgentPhyAddr->SetText(sbuff);
		const Net::MACInfo::MACEntry *ent = Net::MACInfo::GetMACInfoBuff(agent->mac);
		me->txtAgentVendor->SetText((const UTF8Char*)ent->name);
		if (agent->model)
		{
			me->txtAgentModel->SetText(agent->model);	
		}
		else
		{
			me->txtAgentModel->SetText((const UTF8Char*)"");	
		}
		me->lvAgentReading->ClearItems();
		UOSInt i = 0;
		UOSInt j = agent->readingList->GetCount();
		Net::SNMPManager::ReadingInfo *reading;
		while (i < j)
		{
			reading = agent->readingList->GetItem(i);
			me->lvAgentReading->AddItem(reading->name, reading);
			Text::StrUOSInt(sbuff, reading->index);
			me->lvAgentReading->SetSubItem(i, 1, sbuff);
			me->lvAgentReading->SetSubItem(i, 2, SSWR::SMonitor::SAnalogSensor::GetReadingTypeName(reading->readingType));
			if (reading->valValid)
			{
				Text::StrDouble(sbuff, reading->currVal);
				me->lvAgentReading->SetSubItem(i, 3, sbuff);
			}
			else
			{
				me->lvAgentReading->SetSubItem(i, 3, (const UTF8Char*)"-");
			}
			
			i++;
		}
	}
	else
	{
		me->txtAgentDAddr->SetText((const UTF8Char*)"");
		me->txtAgentDescr->SetText((const UTF8Char*)"");
		me->txtAgentOID->SetText((const UTF8Char*)"");
		me->txtAgentOIDName->SetText((const UTF8Char*)"");
		me->txtAgentName->SetText((const UTF8Char*)"");
		me->txtAgentContact->SetText((const UTF8Char*)"");
		me->txtAgentLocation->SetText((const UTF8Char*)"");
		me->txtAgentAddr->SetText((const UTF8Char*)"");
		me->txtAgentVendor->SetText((const UTF8Char*)"");
		me->txtAgentModel->SetText((const UTF8Char*)"");
		me->lvAgentReading->ClearItems();
	}	
}

void __stdcall SSWR::AVIRead::AVIRSNMPManagerForm::OnTimerTick(void *userObj)
{
	SSWR::AVIRead::AVIRSNMPManagerForm *me = (SSWR::AVIRead::AVIRSNMPManagerForm*)userObj;
	UTF8Char sbuff[32];
	Data::DateTime dt;
	dt.SetCurrTimeUTC();
	if (dt.ToTicks() - me->lastUpdateTime >= 30000)
	{
		Net::SNMPManager::ReadingInfo *reading;
		me->mgr->UpdateValues();
		UOSInt i = me->lvAgentReading->GetCount();
		while (i-- > 0)
		{
			reading = (Net::SNMPManager::ReadingInfo*)me->lvAgentReading->GetItem(i);
			if (reading->valValid)
			{
				Text::StrDouble(sbuff, reading->currVal);
				me->lvAgentReading->SetSubItem(i, 3, sbuff);
			}
			else
			{
				me->lvAgentReading->SetSubItem(i, 3, (const UTF8Char*)"-");
			}
		}
		if (me->chkSendToSvr->IsChecked())
		{
			Data::ArrayList<Net::SNMPManager::AgentInfo*> agentList;
			me->mgr->GetAgentList(&agentList);
			if (agentList.GetCount() > 0)
			{
				me->SendAgentValues(&agentList);
			}
		}
		dt.SetCurrTimeUTC();
		me->lastUpdateTime = dt.ToTicks();
	}
}

void __stdcall SSWR::AVIRead::AVIRSNMPManagerForm::OnAgentWalkClicked(void *userObj)
{
	SSWR::AVIRead::AVIRSNMPManagerForm *me = (SSWR::AVIRead::AVIRSNMPManagerForm*)userObj;
	Net::SNMPManager::AgentInfo *agent = (Net::SNMPManager::AgentInfo*)me->lbAgent->GetSelectedItem();
	if (agent)
	{
		SSWR::AVIRead::AVIRSNMPWalkForm *frm;
		NEW_CLASS(frm, SSWR::AVIRead::AVIRSNMPWalkForm(0, me->ui, me->core, &agent->addr, agent->community));
		frm->ShowDialog(me);
		DEL_CLASS(frm);
	}
}

void SSWR::AVIRead::AVIRSNMPManagerForm::SendAgentValues(Data::ArrayList<Net::SNMPManager::AgentInfo *> *agentList)
{
	Net::SNMPManager::AgentInfo *agent;
	SSWR::SMonitor::ISMonitorCore::DevRecord2 devRec;
	Int64 cliId;
	UOSInt i = agentList->GetCount();
	while (i-- > 0)
	{
		agent = agentList->GetItem(i);
		Net::SNMPManager::Agent2Record(agent, &devRec, &cliId);
		this->redir->SendDevReading(cliId, &devRec, 30, 30);
	}
}

SSWR::AVIRead::AVIRSNMPManagerForm::AVIRSNMPManagerForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core) : UI::GUIForm(parent, 1024, 768, ui)
{
	this->SetFont(0, 8.25, false);
	this->SetText((const UTF8Char*)"SNMP Manager");

	this->core = core;
	Data::DateTime dt;
	dt.SetCurrTimeUTC();
	this->lastUpdateTime = dt.ToTicks();
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));

	NEW_CLASS(this->pnlControl, UI::GUIPanel(ui, this));
	this->pnlControl->SetRect(0, 0, 100, 104, false);
	this->pnlControl->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblAgentAddr, UI::GUILabel(ui, this->pnlControl, (const UTF8Char*)"Agent Addr"));
	this->lblAgentAddr->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtAgentAddr, UI::GUITextBox(ui, this->pnlControl, (const UTF8Char*)""));
	this->txtAgentAddr->SetRect(104, 4, 200, 23, false);
	NEW_CLASS(this->chkAgentScan, UI::GUICheckBox(ui, this->pnlControl, (const UTF8Char*)"Scan IP", false));
	this->chkAgentScan->SetRect(304, 4, 100, 23, false);
	NEW_CLASS(this->lblCommunity, UI::GUILabel(ui, this->pnlControl, (const UTF8Char*)"Community"));
	this->lblCommunity->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->txtCommunity, UI::GUITextBox(ui, this->pnlControl, (const UTF8Char*)"public"));
	this->txtCommunity->SetRect(104, 28, 200, 23, false);
	NEW_CLASS(this->btnAgentAdd, UI::GUIButton(ui, this->pnlControl, (const UTF8Char*)"Add"));
	this->btnAgentAdd->SetRect(104, 52, 75, 23, false);
	this->btnAgentAdd->HandleButtonClick(OnAgentAddClicked, this);
	NEW_CLASS(this->chkSendToSvr, UI::GUICheckBox(ui, this->pnlControl, (const UTF8Char*)"Send to Server", false));
	this->chkSendToSvr->SetRect(104, 76, 150, 23, false);
	NEW_CLASS(this->lbAgent, UI::GUIListBox(ui, this, false));
	this->lbAgent->SetRect(0, 0, 150, 23, false);
	this->lbAgent->SetDockType(UI::GUIControl::DOCK_LEFT);
	this->lbAgent->HandleSelectionChange(OnAgentSelChg, this);
	NEW_CLASS(this->hspAgent, UI::GUIHSplitter(ui, this, 3, false));
	NEW_CLASS(this->pnlAgent, UI::GUIPanel(ui, this));
	this->pnlAgent->SetRect(0, 0, 100, 248, false);
	this->pnlAgent->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblAgentDAddr, UI::GUILabel(ui, this->pnlAgent, (const UTF8Char*)"Agent Address"));
	this->lblAgentDAddr->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtAgentDAddr, UI::GUITextBox(ui, this->pnlAgent, (const UTF8Char*)""));
	this->txtAgentDAddr->SetRect(104, 4, 150, 23, false);
	this->txtAgentDAddr->SetReadOnly(true);
	NEW_CLASS(this->btnAgentWalk, UI::GUIButton(ui, this->pnlAgent, (const UTF8Char*)"Walk"));
	this->btnAgentWalk->SetRect(254, 4, 75, 23, false);
	this->btnAgentWalk->HandleButtonClick(OnAgentWalkClicked, this);
	NEW_CLASS(this->lblAgentDescr, UI::GUILabel(ui, this->pnlAgent, (const UTF8Char*)"Description"));
	this->lblAgentDescr->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->txtAgentDescr, UI::GUITextBox(ui, this->pnlAgent, (const UTF8Char*)""));
	this->txtAgentDescr->SetRect(104, 28, 500, 23, false);
	this->txtAgentDescr->SetReadOnly(true);
	NEW_CLASS(this->lblAgentOID, UI::GUILabel(ui, this->pnlAgent, (const UTF8Char*)"OID"));
	this->lblAgentOID->SetRect(4, 52, 100, 23, false);
	NEW_CLASS(this->txtAgentOID, UI::GUITextBox(ui, this->pnlAgent, (const UTF8Char*)""));
	this->txtAgentOID->SetRect(104, 52, 150, 23, false);
	this->txtAgentOID->SetReadOnly(true);
	NEW_CLASS(this->lblAgentOIDName, UI::GUILabel(ui, this->pnlAgent, (const UTF8Char*)"OID Name"));
	this->lblAgentOIDName->SetRect(4, 76, 100, 23, false);
	NEW_CLASS(this->txtAgentOIDName, UI::GUITextBox(ui, this->pnlAgent, (const UTF8Char*)""));
	this->txtAgentOIDName->SetRect(104, 76, 150, 23, false);
	this->txtAgentOIDName->SetReadOnly(true);
	NEW_CLASS(this->lblAgentName, UI::GUILabel(ui, this->pnlAgent, (const UTF8Char*)"Name"));
	this->lblAgentName->SetRect(4, 100, 100, 23, false);
	NEW_CLASS(this->txtAgentName, UI::GUITextBox(ui, this->pnlAgent, (const UTF8Char*)""));
	this->txtAgentName->SetRect(104, 100, 150, 23, false);
	this->txtAgentName->SetReadOnly(true);
	NEW_CLASS(this->lblAgentContact, UI::GUILabel(ui, this->pnlAgent, (const UTF8Char*)"Contact"));
	this->lblAgentContact->SetRect(4, 124, 100, 23, false);
	NEW_CLASS(this->txtAgentContact, UI::GUITextBox(ui, this->pnlAgent, (const UTF8Char*)""));
	this->txtAgentContact->SetRect(104, 124, 150, 23, false);
	this->txtAgentContact->SetReadOnly(true);
	NEW_CLASS(this->lblAgentLocation, UI::GUILabel(ui, this->pnlAgent, (const UTF8Char*)"Location"));
	this->lblAgentLocation->SetRect(4, 148, 100, 23, false);
	NEW_CLASS(this->txtAgentLocation, UI::GUITextBox(ui, this->pnlAgent, (const UTF8Char*)""));
	this->txtAgentLocation->SetRect(104, 148, 150, 23, false);
	this->txtAgentLocation->SetReadOnly(true);
	NEW_CLASS(this->lblAgentPhyAddr, UI::GUILabel(ui, this->pnlAgent, (const UTF8Char*)"Phy Addr"));
	this->lblAgentPhyAddr->SetRect(4, 172, 100, 23, false);
	NEW_CLASS(this->txtAgentPhyAddr, UI::GUITextBox(ui, this->pnlAgent, (const UTF8Char*)""));
	this->txtAgentPhyAddr->SetRect(104, 172, 150, 23, false);
	this->txtAgentPhyAddr->SetReadOnly(true);
	NEW_CLASS(this->lblAgentVendor, UI::GUILabel(ui, this->pnlAgent, (const UTF8Char*)"Vendor"));
	this->lblAgentVendor->SetRect(4, 196, 100, 23, false);
	NEW_CLASS(this->txtAgentVendor, UI::GUITextBox(ui, this->pnlAgent, (const UTF8Char*)""));
	this->txtAgentVendor->SetRect(104, 196, 150, 23, false);
	this->txtAgentVendor->SetReadOnly(true);
	NEW_CLASS(this->lblAgentModel, UI::GUILabel(ui, this->pnlAgent, (const UTF8Char*)"Model"));
	this->lblAgentModel->SetRect(4, 220, 100, 23, false);
	NEW_CLASS(this->txtAgentModel, UI::GUITextBox(ui, this->pnlAgent, (const UTF8Char*)""));
	this->txtAgentModel->SetRect(104, 220, 150, 23, false);
	this->txtAgentModel->SetReadOnly(true);
	NEW_CLASS(this->lvAgentReading, UI::GUIListView(ui, this, UI::GUIListView::LVSTYLE_TABLE, 4));
	this->lvAgentReading->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvAgentReading->SetShowGrid(true);
	this->lvAgentReading->SetFullRowSelect(true);
	this->lvAgentReading->AddColumn((const UTF8Char*)"Name", 150);
	this->lvAgentReading->AddColumn((const UTF8Char*)"Index", 60);
	this->lvAgentReading->AddColumn((const UTF8Char*)"Type", 100);
	this->lvAgentReading->AddColumn((const UTF8Char*)"Value", 150);

	this->AddTimer(1000, OnTimerTick, this);

	Net::SocketFactory *sockf = this->core->GetSocketFactory();
	NEW_CLASS(this->redir, SSWR::SMonitor::SMonitorRedir(sockf));
	NEW_CLASS(this->mgr, Net::SNMPManager(sockf));
	if (this->mgr->IsError())
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Error in starting SNMP Manager", (const UTF8Char*)"Error", this);
	}

	Data::ArrayList<Net::ConnectionInfo*> connInfoList;
	Net::ConnectionInfo *connInfo;
	UTF8Char sbuff[32];
	UOSInt i;
	UOSInt j;
	UInt32 ip;
	UInt32 netmask;
	sockf->GetConnInfoList(&connInfoList);
	i = 0;
	j = connInfoList.GetCount();
	while (i < j)
	{
		connInfo = connInfoList.GetItem(i);
		ip = connInfo->GetDefaultGW();
		if (ip)
		{
			netmask = Net::SocketUtil::GetDefNetMaskv4(ip);
			ip |= ~netmask;
			Net::SocketUtil::GetIPv4Name(sbuff, ip);
			this->txtAgentAddr->SetText(sbuff);
		}
		DEL_CLASS(connInfo);
		i++;
	}
}

SSWR::AVIRead::AVIRSNMPManagerForm::~AVIRSNMPManagerForm()
{
	DEL_CLASS(this->mgr);
	DEL_CLASS(this->redir);
}

void SSWR::AVIRead::AVIRSNMPManagerForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}

Bool SSWR::AVIRead::AVIRSNMPManagerForm::IsError()
{
	return this->mgr->IsError();
}
