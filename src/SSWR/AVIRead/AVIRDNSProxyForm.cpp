#include "Stdafx.h"
#include "Manage/HiResClock.h"
#include "SSWR/AVIRead/AVIRDNSProxyForm.h"
#include "Sync/MutexUsage.h"
#include "Text/MyStringFloat.h"
#include "Text/StringBuilderUTF8.h"
#include "UI/MessageDialog.h"

void __stdcall SSWR::AVIRead::AVIRDNSProxyForm::OnTimerTick(void *userObj)
{
	SSWR::AVIRead::AVIRDNSProxyForm *me = (SSWR::AVIRead::AVIRDNSProxyForm*)userObj;
	UTF8Char sbuff[32];
	UInt32 ip = me->proxy->GetServerIP();
	if (ip != me->currServer)
	{
		me->currServer = ip;
		Net::SocketUtil::GetIPv4Name(sbuff, ip);
		me->txtDNSServer->SetText(sbuff);
	}

	if (me->proxy->IsReqListv4Chg())
	{
		Data::ArrayList<const UTF8Char *> nameList;
		Text::StringBuilderUTF8 sb;
		UOSInt i;
		UOSInt j;
		me->proxy->GetReqv4List(&nameList);

		me->lbV4Request->ClearItems();
		i = 0;
		j = nameList.GetCount();
		while (i < j)
		{
			sb.ClearStr();
			sb.Append(nameList.GetItem(i));
			me->lbV4Request->AddItem(sb.ToString(), 0);
			i++;
		}
	}
	if (me->proxy->IsReqListv6Chg())
	{
		Data::ArrayList<const UTF8Char *> nameList;
		Text::StringBuilderUTF8 sb;
		UOSInt i;
		UOSInt j;
		me->proxy->GetReqv6List(&nameList);

		me->lbV6Request->ClearItems();
		i = 0;
		j = nameList.GetCount();
		while (i < j)
		{
			sb.ClearStr();
			sb.Append(nameList.GetItem(i));
			me->lbV6Request->AddItem(sb.ToString(), 0);
			i++;
		}
	}
	if (me->proxy->IsReqListOthChg())
	{
		Data::ArrayList<const UTF8Char *> nameList;
		Text::StringBuilderUTF8 sb;
		UOSInt i;
		UOSInt j;
		me->proxy->GetReqOthList(&nameList);

		me->lbOthRequest->ClearItems();
		i = 0;
		j = nameList.GetCount();
		while (i < j)
		{
			sb.ClearStr();
			sb.Append(nameList.GetItem(i));
			me->lbOthRequest->AddItem(sb.ToString(), 0);
			i++;
		}
	}
	if (me->proxy->IsTargetChg())
	{
		Data::ArrayList<Net::DNSProxy::TargetInfo*> targetList;
		UOSInt i;
		UOSInt j;
		Net::DNSProxy::TargetInfo *target;
		me->proxy->GetTargetList(&targetList);

		me->lbTarget->ClearItems();
		i = 0;
		j = targetList.GetCount();
		while (i < j)
		{
			target = targetList.GetItem(i);
			Net::SocketUtil::GetIPv4Name(sbuff, target->ip);
			me->lbTarget->AddItem(sbuff, target);
			if (me->currTarget == target)
			{
				me->lbTarget->SetSelectedIndex(i);
			}
			i++;
		}
	}

	if (me->cliChg)
	{
		WChar sbuff[32];
		ClientInfo *cli;
		UOSInt i;
		UOSInt j;
		Data::ArrayList<ClientInfo*> *cliList;
		me->cliChg = false;
		me->lbClientIP->ClearItems();
		me->lvClient->ClearItems();

		Sync::MutexUsage mutUsage(me->cliInfoMut);
		cliList = me->cliInfos->GetValues();
		i = 0;
		j = cliList->GetCount();
		while (i < j)
		{
			cli = cliList->GetItem(i);
			Net::SocketUtil::GetAddrName(sbuff, &cli->addr);
			me->lbClientIP->AddItem(sbuff, cli);
			i++;
		}
		mutUsage.EndUse();
	}
}

void __stdcall SSWR::AVIRead::AVIRDNSProxyForm::OnV4ReqSelChg(void *userObj)
{
	SSWR::AVIRead::AVIRDNSProxyForm *me = (SSWR::AVIRead::AVIRDNSProxyForm*)userObj;
	Net::DNSClient::FreeAnswers(me->v4ansList);
	UOSInt i;
	UOSInt j;

	me->lbV4Answer->ClearItems();
	const UTF8Char *req = me->lbV4Request->GetSelectedItemTextNew();
	if (req)
	{
		UTF8Char sbuff[32];
		Net::DNSClient::RequestAnswer *ans;
		Data::DateTime reqTime;
		UInt32 ttl;
		Text::StringBuilderUTF8 sb;
		sb.Append(req);
		if (me->proxy->GetRequestInfov4(sb.ToString(), me->v4ansList, &reqTime, &ttl))
		{
			UOSInt bestInd = (UOSInt)-1;
			i = 0;
			j = me->v4ansList->GetCount();
			while (i < j)
			{
				ans = me->v4ansList->GetItem(i);
				if (bestInd == (UOSInt)-1 && ans->recType == 1)
					bestInd = i;
				sb.ClearStr();
				sb.Append(ans->name);
				me->lbV4Answer->AddItem(sb.ToString(), ans);
				i++;
			}
			if (bestInd == (UOSInt)-1)
				bestInd = 0;
			if (j > 0)
			{
				me->lbV4Answer->SetSelectedIndex(bestInd);
			}
			reqTime.ToLocalTime();
			reqTime.ToString(sbuff);
			me->txtV4RequestTime->SetText(sbuff);
			Text::StrUInt32(sbuff, ttl);
			me->txtV4RequestTTL->SetText(sbuff);
		}
		else
		{
		}
		Text::StrDelNew(req);
	}
}

void __stdcall SSWR::AVIRead::AVIRDNSProxyForm::OnV4AnsSelChg(void *userObj)
{
	SSWR::AVIRead::AVIRDNSProxyForm *me = (SSWR::AVIRead::AVIRDNSProxyForm*)userObj;
	Net::DNSClient::RequestAnswer *ans = (Net::DNSClient::RequestAnswer*)me->lbV4Answer->GetSelectedItem();
	if (ans)
	{
		UTF8Char sbuff[16];
		Data::DateTime t;
		me->txtV4AnsName->SetText(ans->name);
		Text::StrInt32(sbuff, ans->recType);
		me->txtV4AnsType->SetText(sbuff);
		Text::StrInt32(sbuff, ans->recClass);
		me->txtV4AnsClass->SetText(sbuff);
		Text::StrUInt32(sbuff, ans->ttl);
		me->txtV4AnsTTL->SetText(sbuff);
		if (ans->rd)
		{
			me->txtV4AnsRD->SetText(ans->rd);
		}
		else
		{
			me->txtV4AnsRD->SetText((const UTF8Char*)"");
		}
	}
	else
	{
		me->txtV4AnsName->SetText((const UTF8Char*)"");
		me->txtV4AnsType->SetText((const UTF8Char*)"");
		me->txtV4AnsClass->SetText((const UTF8Char*)"");
		me->txtV4AnsTTL->SetText((const UTF8Char*)"");
		me->txtV4AnsRD->SetText((const UTF8Char*)"");
	}
}

void __stdcall SSWR::AVIRead::AVIRDNSProxyForm::OnV6ReqSelChg(void *userObj)
{
	SSWR::AVIRead::AVIRDNSProxyForm *me = (SSWR::AVIRead::AVIRDNSProxyForm*)userObj;
	Net::DNSClient::FreeAnswers(me->v6ansList);
	UOSInt i;
	UOSInt j;

	me->lbV6Answer->ClearItems();
	const UTF8Char *req = me->lbV6Request->GetSelectedItemTextNew();
	if (req)
	{
		UTF8Char sbuff[32];
		Net::DNSClient::RequestAnswer *ans;
		Data::DateTime reqTime;
		UInt32 ttl;
		Text::StringBuilderUTF8 sb;
		sb.Append(req);
		if (me->proxy->GetRequestInfov6(sb.ToString(), me->v6ansList, &reqTime, &ttl))
		{
			UOSInt bestInd = (UOSInt)-1;
			i = 0;
			j = me->v6ansList->GetCount();
			while (i < j)
			{
				ans = me->v6ansList->GetItem(i);
				if (bestInd == (UOSInt)-1 && ans->recType == 1)
					bestInd = i;
				me->lbV6Answer->AddItem(ans->name, ans);
				i++;
			}
			if (bestInd == (UOSInt)-1)
				bestInd = 0;
			if (j > 0)
			{
				me->lbV6Answer->SetSelectedIndex(bestInd);
			}
			reqTime.ToLocalTime();
			reqTime.ToString(sbuff);
			me->txtV6RequestTime->SetText(sbuff);
			Text::StrUInt32(sbuff, ttl);
			me->txtV6RequestTTL->SetText(sbuff);
		}
		else
		{
		}
		Text::StrDelNew(req);
	}
}

void __stdcall SSWR::AVIRead::AVIRDNSProxyForm::OnV6AnsSelChg(void *userObj)
{
	SSWR::AVIRead::AVIRDNSProxyForm *me = (SSWR::AVIRead::AVIRDNSProxyForm*)userObj;
	Net::DNSClient::RequestAnswer *ans = (Net::DNSClient::RequestAnswer*)me->lbV6Answer->GetSelectedItem();
	if (ans)
	{
		UTF8Char sbuff[16];
		Data::DateTime t;
		me->txtV6AnsName->SetText(ans->name);
		Text::StrInt32(sbuff, ans->recType);
		me->txtV6AnsType->SetText(sbuff);
		Text::StrInt32(sbuff, ans->recClass);
		me->txtV6AnsClass->SetText(sbuff);
		Text::StrUInt32(sbuff, ans->ttl);
		me->txtV6AnsTTL->SetText(sbuff);
		if (ans->rd)
		{
			me->txtV6AnsRD->SetText(ans->rd);
		}
		else
		{
			me->txtV6AnsRD->SetText((const UTF8Char*)"");
		}
	}
	else
	{
		me->txtV6AnsName->SetText((const UTF8Char*)"");
		me->txtV6AnsType->SetText((const UTF8Char*)"");
		me->txtV6AnsClass->SetText((const UTF8Char*)"");
		me->txtV6AnsTTL->SetText((const UTF8Char*)"");
		me->txtV6AnsRD->SetText((const UTF8Char*)"");
	}
}

void __stdcall SSWR::AVIRead::AVIRDNSProxyForm::OnOthReqSelChg(void *userObj)
{
	SSWR::AVIRead::AVIRDNSProxyForm *me = (SSWR::AVIRead::AVIRDNSProxyForm*)userObj;
	Net::DNSClient::FreeAnswers(me->othansList);
	UOSInt i;
	UOSInt j;

	me->lbOthAnswer->ClearItems();
	const UTF8Char *req = me->lbOthRequest->GetSelectedItemTextNew();
	if (req)
	{
		UTF8Char sbuff[32];
		Net::DNSClient::RequestAnswer *ans;
		Data::DateTime reqTime;
		UInt32 ttl;
		if (me->proxy->GetRequestInfoOth(req, me->othansList, &reqTime, &ttl))
		{
			UOSInt bestInd = (UOSInt)-1;
			i = 0;
			j = me->othansList->GetCount();
			while (i < j)
			{
				ans = me->othansList->GetItem(i);
				if (bestInd == (UOSInt)-1 && ans->recType == 1)
					bestInd = i;
				me->lbOthAnswer->AddItem(ans->name, ans);
				i++;
			}
			if (bestInd == (UOSInt)-1)
				bestInd = 0;
			if (j > 0)
			{
				me->lbOthAnswer->SetSelectedIndex(bestInd);
			}
			reqTime.ToLocalTime();
			reqTime.ToString(sbuff);
			me->txtOthRequestTime->SetText(sbuff);
			Text::StrUInt32(sbuff, ttl);
			me->txtOthRequestTTL->SetText(sbuff);
		}
		else
		{
		}
		Text::StrDelNew(req);
	}
}

void __stdcall SSWR::AVIRead::AVIRDNSProxyForm::OnOthAnsSelChg(void *userObj)
{
	SSWR::AVIRead::AVIRDNSProxyForm *me = (SSWR::AVIRead::AVIRDNSProxyForm*)userObj;
	Net::DNSClient::RequestAnswer *ans = (Net::DNSClient::RequestAnswer*)me->lbOthAnswer->GetSelectedItem();
	if (ans)
	{
		UTF8Char sbuff[16];
		Data::DateTime t;
		me->txtOthAnsName->SetText(ans->name);
		Text::StrInt32(sbuff, ans->recType);
		me->txtOthAnsType->SetText(sbuff);
		Text::StrInt32(sbuff, ans->recClass);
		me->txtOthAnsClass->SetText(sbuff);
		Text::StrUInt32(sbuff, ans->ttl);
		me->txtOthAnsTTL->SetText(sbuff);
		if (ans->rd)
		{
			me->txtOthAnsRD->SetText(ans->rd);
		}
		else
		{
			me->txtOthAnsRD->SetText((const UTF8Char*)"");
		}
	}
	else
	{
		me->txtOthAnsName->SetText((const UTF8Char*)"");
		me->txtOthAnsType->SetText((const UTF8Char*)"");
		me->txtOthAnsClass->SetText((const UTF8Char*)"");
		me->txtOthAnsTTL->SetText((const UTF8Char*)"");
		me->txtOthAnsRD->SetText((const UTF8Char*)"");
	}
}

void __stdcall SSWR::AVIRead::AVIRDNSProxyForm::OnTargetSelChg(void *userObj)
{
	SSWR::AVIRead::AVIRDNSProxyForm *me = (SSWR::AVIRead::AVIRDNSProxyForm*)userObj;
	Net::DNSProxy::TargetInfo *target = (Net::DNSProxy::TargetInfo*)me->lbTarget->GetSelectedItem();
	me->currTarget = target;
	if (target)
	{
		Net::WhoisRecord *rec = me->whois->RequestIP(target->ip);
		UTF8Char sbuff[256];
		UOSInt i;
		UOSInt j;
		if (rec)
		{
			if (rec->GetNetworkName(sbuff))
			{
				me->txtTargetName->SetText(sbuff);
			}
			else
			{
				me->txtTargetName->SetText((const UTF8Char*)"Unknown");
			}
			if (rec->GetCountryCode(sbuff))
			{
				me->txtTargetCountry->SetText(sbuff);
			}
			else
			{
				me->txtTargetCountry->SetText((const UTF8Char*)"Unk");
			}
			Text::StringBuilderUTF8 sb;
			i = 0;
			j = rec->GetCount();
			while (i < j)
			{
				sb.Append(rec->GetItem(i));
				sb.Append((const UTF8Char*)"\r\n");
				i++;
			}
			me->txtTargetWhois->SetText(sb.ToString());
		}
		else
		{
			me->txtTargetWhois->SetText((const UTF8Char*)"");
			me->txtTargetName->SetText((const UTF8Char*)"");
			me->txtTargetCountry->SetText((const UTF8Char*)"");
		}
		Sync::MutexUsage mutUsage(target->mut);
		me->lbTargetDomains->ClearItems();
		i = 0;
		j = target->addrList->GetCount();
		while (i < j)
		{
			me->lbTargetDomains->AddItem(target->addrList->GetItem(i), 0);
			i++;
		}
		mutUsage.EndUse();
	}
	else
	{
		me->txtTargetWhois->SetText((const UTF8Char*)"");
		me->txtTargetName->SetText((const UTF8Char*)"");
		me->txtTargetCountry->SetText((const UTF8Char*)"");
		me->lbTargetDomains->ClearItems();
	}
}

void __stdcall SSWR::AVIRead::AVIRDNSProxyForm::OnDNSSwitchClicked(void *userObj)
{
	SSWR::AVIRead::AVIRDNSProxyForm *me = (SSWR::AVIRead::AVIRDNSProxyForm*)userObj;
	me->proxy->SwitchDNS();
}

void __stdcall SSWR::AVIRead::AVIRDNSProxyForm::OnDNSSetClicked(void *userObj)
{
	SSWR::AVIRead::AVIRDNSProxyForm *me = (SSWR::AVIRead::AVIRDNSProxyForm*)userObj;
	Text::StringBuilderUTF8 sb;
	me->txtDNSServer2->GetText(&sb);
	UInt32 svrIP = Net::SocketUtil::GetIPAddr(sb.ToString());
	if (svrIP == 0)
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Error in parsing server ip", (const UTF8Char*)"Error", me);
		return;
	}
	else
	{
		me->proxy->SetServerIP(svrIP);
		me->UpdateDNSList();
	}
}

void __stdcall SSWR::AVIRead::AVIRDNSProxyForm::OnDNSAddClicked(void *userObj)
{
	SSWR::AVIRead::AVIRDNSProxyForm *me = (SSWR::AVIRead::AVIRDNSProxyForm*)userObj;
	Text::StringBuilderUTF8 sb;
	me->txtDNSServer2->GetText(&sb);
	UInt32 svrIP = Net::SocketUtil::GetIPAddr(sb.ToString());
	if (svrIP == 0)
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Error in parsing server ip", (const UTF8Char*)"Error", me);
		return;
	}
	else
	{
		me->proxy->AddDNSIP(svrIP);
		me->UpdateDNSList();
	}
}

void __stdcall SSWR::AVIRead::AVIRDNSProxyForm::OnSearchClicked(void *userObj)
{
	SSWR::AVIRead::AVIRDNSProxyForm *me = (SSWR::AVIRead::AVIRDNSProxyForm*)userObj;
	Data::ArrayList<const UTF8Char *> nameList;
	Text::StringBuilderUTF8 sb;
	UInt32 ip;
	UInt32 mask;
	UOSInt i;
	UOSInt j;
	me->txtSearchIPRange->GetText(&sb);
	ip = Net::SocketUtil::GetIPAddr(sb.ToString());
	if (ip == 0)
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Please enter IP Range", (const UTF8Char*)"Search", me);
		return;
	}
	sb.ClearStr();
	me->txtSearchMask->GetText(&sb);
	if (sb.ToUInt32(&mask))
	{
		if (mask >= 32)
		{
			mask = (UInt32)-1;
		}
		else
		{
			mask = (UInt32)~((1 << (32 - mask)) - 1);
			mask = BSWAPU32(mask);
		}
	}
	else if ((mask = Net::SocketUtil::GetIPAddr(sb.ToString())) != 0)
	{
	}
	else
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Please enter valid mask", (const UTF8Char*)"Search", me);
		return;
	}
	me->proxy->SearchIPv4(&nameList, ip, mask);

	me->lbSearch->ClearItems();
	i = 0;
	j = nameList.GetCount();
	while (i < j)
	{
		sb.ClearStr();
		sb.Append(nameList.GetItem(i));
		me->lbSearch->AddItem(sb.ToString(), 0);
		i++;
	}
}

void __stdcall SSWR::AVIRead::AVIRDNSProxyForm::OnSReqSelChg(void *userObj)
{
	SSWR::AVIRead::AVIRDNSProxyForm *me = (SSWR::AVIRead::AVIRDNSProxyForm*)userObj;
	Net::DNSClient::FreeAnswers(me->v4sansList);
	UOSInt i;
	UOSInt j;

	me->lbSAnswer->ClearItems();
	const UTF8Char *req = me->lbSearch->GetSelectedItemTextNew();
	if (req)
	{
		UTF8Char sbuff[32];
		Net::DNSClient::RequestAnswer *ans;
		Data::DateTime reqTime;
		UInt32 ttl;
		Text::StringBuilderUTF8 sb;
		sb.ClearStr();
		sb.Append(req);
		if (me->proxy->GetRequestInfov4(sb.ToString(), me->v4sansList, &reqTime, &ttl))
		{
			UOSInt bestInd = (UOSInt)-1;
			i = 0;
			j = me->v4sansList->GetCount();
			while (i < j)
			{
				ans = me->v4sansList->GetItem(i);
				if (bestInd == (UOSInt)-1 && ans->recType == 1)
					bestInd = i;
				sb.ClearStr();
				sb.Append(ans->name);
				me->lbSAnswer->AddItem(sb.ToString(), ans);
				i++;
			}
			if (bestInd == (UOSInt)-1)
				bestInd = 0;
			if (j > 0)
			{
				me->lbSAnswer->SetSelectedIndex(bestInd);
			}
			reqTime.ToLocalTime();
			reqTime.ToString(sbuff);
			me->txtSRequestTime->SetText(sbuff);
			Text::StrUInt32(sbuff, ttl);
			me->txtSRequestTTL->SetText(sbuff);
		}
		else
		{
		}
		Text::StrDelNew(req);
	}
}

void __stdcall SSWR::AVIRead::AVIRDNSProxyForm::OnSAnsSelChg(void *userObj)
{
	SSWR::AVIRead::AVIRDNSProxyForm *me = (SSWR::AVIRead::AVIRDNSProxyForm*)userObj;
	Net::DNSClient::RequestAnswer *ans = (Net::DNSClient::RequestAnswer*)me->lbSAnswer->GetSelectedItem();
	if (ans)
	{
		UTF8Char sbuff[16];
		Data::DateTime t;
		me->txtSAnsName->SetText(ans->name);
		Text::StrInt32(sbuff, ans->recType);
		me->txtSAnsType->SetText(sbuff);
		Text::StrInt32(sbuff, ans->recClass);
		me->txtSAnsClass->SetText(sbuff);
		Text::StrUInt32(sbuff, ans->ttl);
		me->txtSAnsTTL->SetText(sbuff);
		if (ans->rd)
		{
			me->txtSAnsRD->SetText(ans->rd);
		}
		else
		{
			me->txtSAnsRD->SetText((const UTF8Char*)"");
		}
	}
	else
	{
		me->txtSAnsName->SetText((const UTF8Char*)"");
		me->txtSAnsType->SetText((const UTF8Char*)"");
		me->txtSAnsClass->SetText((const UTF8Char*)"");
		me->txtSAnsTTL->SetText((const UTF8Char*)"");
		me->txtSAnsRD->SetText((const UTF8Char*)"");
	}
}

void __stdcall SSWR::AVIRead::AVIRDNSProxyForm::OnClientSelChg(void *userObj)
{
	SSWR::AVIRead::AVIRDNSProxyForm *me = (SSWR::AVIRead::AVIRDNSProxyForm*)userObj;
	ClientInfo *cli = (ClientInfo*)me->lbClientIP->GetSelectedItem();
	me->lvClient->ClearItems();
	if (cli)
	{
		UTF8Char sbuff[32];
		UTF8Char *sptr;
		UOSInt i;
		UOSInt j;
		HourInfo *hInfo;
		Sync::MutexUsage mutUsage(cli->mut);
		i = cli->hourInfos->GetCount();
		while (i-- > 0)
		{
			hInfo = cli->hourInfos->GetItem(i);
			sptr = Text::StrInt32(sbuff, hInfo->year);
			sptr = Text::StrConcat(sptr, (const UTF8Char*)"-");
			sptr = Text::StrInt32(sptr, hInfo->month);
			sptr = Text::StrConcat(sptr, (const UTF8Char*)"-");
			sptr = Text::StrInt32(sptr, hInfo->day);
			sptr = Text::StrConcat(sptr, (const UTF8Char*)"-");
			sptr = Text::StrInt32(sptr, hInfo->hour);
			sptr = Text::StrConcat(sptr, (const UTF8Char*)":00");
			j = me->lvClient->AddItem(sbuff, 0);
			Text::StrInt64(sbuff, hInfo->reqCount);
			me->lvClient->SetSubItem(j, 1, sbuff);
		}
		mutUsage.EndUse();
	}
}

void __stdcall SSWR::AVIRead::AVIRDNSProxyForm::OnDisableV6Chg(void *userObj, Bool isChecked)
{
	SSWR::AVIRead::AVIRDNSProxyForm *me = (SSWR::AVIRead::AVIRDNSProxyForm*)userObj;
	me->proxy->SetDisableV6(isChecked);
}

void __stdcall SSWR::AVIRead::AVIRDNSProxyForm::OnBlackListClicked(void *userObj)
{
	SSWR::AVIRead::AVIRDNSProxyForm *me = (SSWR::AVIRead::AVIRDNSProxyForm*)userObj;
	Text::StringBuilderUTF8 sb;
	me->txtBlackList->GetText(&sb);
	if (sb.GetLength() > 0)
	{
		if (me->proxy->AddBlackList(sb.ToString()))
		{
			me->txtBlackList->SetText((const UTF8Char*)"");
			me->UpdateBlackList();
		}
	}
}

void __stdcall SSWR::AVIRead::AVIRDNSProxyForm::OnDNSRequest(void *userObj, const UTF8Char *reqName, Int32 reqType, Int32 reqClass, const Net::SocketUtil::AddressInfo *reqAddr, UInt16 reqPort, UInt32 reqId, Double timeUsed)
{
	SSWR::AVIRead::AVIRDNSProxyForm *me = (SSWR::AVIRead::AVIRDNSProxyForm*)userObj;
	UTF8Char sbuff[32];
	Text::StringBuilderUTF8 sb;
	sb.Append(reqName);
	sb.Append((const UTF8Char*)" from ");
	Net::SocketUtil::GetAddrName(sbuff, reqAddr, reqPort);
	sb.Append(sbuff);
	sb.Append((const UTF8Char*)", T=");
	sb.AppendI32(reqType);
	sb.Append((const UTF8Char*)", C=");
	sb.AppendI32(reqClass);
	sb.Append((const UTF8Char*)", t=");
	Text::SBAppendF64(&sb, timeUsed);
	me->log->LogMessage(sb.ToString(), IO::ILogHandler::LOG_LEVEL_RAW);

	ClientInfo *cli;
	UInt32 cliId = Net::SocketUtil::CalcCliId(reqAddr);
	Sync::MutexUsage cimutUsage(me->cliInfoMut);
	cli = me->cliInfos->Get(cliId);
	if (cli == 0)
	{
		cli = MemAlloc(ClientInfo, 1);
		cli->cliId = cliId;
		cli->addr = *reqAddr;
		NEW_CLASS(cli->mut, Sync::Mutex());
		NEW_CLASS(cli->hourInfos, Data::ArrayList<HourInfo*>());
		me->cliInfos->Put(cliId, cli);
		me->cliChg = true;
	}
	cimutUsage.EndUse();
	Data::DateTime dt;
	HourInfo *hInfo;
	dt.SetCurrTimeUTC();
	Sync::MutexUsage mutUsage(cli->mut);
	hInfo = cli->hourInfos->GetItem(0);
	if (hInfo != 0 && hInfo->year == dt.GetYear() && hInfo->month == dt.GetMonth() && hInfo->day == dt.GetDay() && hInfo->hour == dt.GetHour())
	{
		hInfo->reqCount++;
	}
	else
	{
		if (cli->hourInfos->GetCount() >= 72)
		{
			hInfo = cli->hourInfos->RemoveAt(71);
		}
		else
		{
			hInfo = MemAlloc(HourInfo, 1);
		}
		hInfo->year = dt.GetYear();
		hInfo->month = dt.GetMonth();
		hInfo->day = dt.GetDay();
		hInfo->hour = dt.GetHour();
		hInfo->reqCount = 1;
		cli->hourInfos->Insert(0, hInfo);
	}
	mutUsage.EndUse();
}

void SSWR::AVIRead::AVIRDNSProxyForm::UpdateDNSList()
{
	WChar sbuff[32];
	Data::ArrayList<UInt32> ipList;
	UOSInt i;
	UOSInt j;
	UInt32 ip;
	this->proxy->GetDNSList(&ipList);
	this->lbDNSList->ClearItems();
	i = 0;
	j = ipList.GetCount();
	while (i < j)
	{
		ip = ipList.GetItem(i);
		Net::SocketUtil::GetIPv4Name(sbuff, ip);
		this->lbDNSList->AddItem(sbuff, (void*)(OSInt)ip);
		i++;
	}
}

void SSWR::AVIRead::AVIRDNSProxyForm::UpdateBlackList()
{
	Data::ArrayList<const UTF8Char *> blackList;
	UOSInt i;
	UOSInt j;
	Text::StringBuilderUTF8 sb;
	this->proxy->GetBlackList(&blackList);
	this->lbBlackList->ClearItems();
	i = 0;
	j = blackList.GetCount();
	while (i < j)
	{
		sb.ClearStr();
		sb.Append(blackList.GetItem(i));
		this->lbBlackList->AddItem(sb.ToString(), 0);
		i++;
	}
	
}

SSWR::AVIRead::AVIRDNSProxyForm::AVIRDNSProxyForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core) : UI::GUIForm(parent, 1024, 768, ui)
{
	UTF8Char sbuff[32];
	this->SetFont(0, 8.25, 0);
	this->SetText((const UTF8Char*)"DNS Proxy");

	this->core = core;
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
	this->currServer = 0;
	this->currTarget = 0;

	NEW_CLASS(this->tcMain, UI::GUITabControl(ui, this));
	this->tcMain->SetDockType(UI::GUIControl::DOCK_FILL);

	this->tpStatus = this->tcMain->AddTabPage((const UTF8Char*)"Status");
	NEW_CLASS(this->lblDNSPort, UI::GUILabel(ui, this->tpStatus, (const UTF8Char*)"Listening Port"));
	this->lblDNSPort->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtDNSPort, UI::GUITextBox(ui, this->tpStatus, (const UTF8Char*)"53"));
	this->txtDNSPort->SetRect(104, 4, 100, 23, false);
	this->txtDNSPort->SetReadOnly(true);
	NEW_CLASS(this->lblDisableV6, UI::GUILabel(ui, this->tpStatus, (const UTF8Char*)"Disable IPv6"));
	this->lblDisableV6->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->chkDisableV6, UI::GUICheckBox(ui, this->tpStatus, (const UTF8Char*)"", false));
	this->chkDisableV6->SetRect(104, 28, 100, 23, false);
	this->chkDisableV6->HandleCheckedChange(OnDisableV6Chg, this);
	NEW_CLASS(this->lblDNSServer, UI::GUILabel(ui, this->tpStatus, (const UTF8Char*)"DNS Server"));
	this->lblDNSServer->SetRect(4, 52, 100, 23, false);
	NEW_CLASS(this->txtDNSServer, UI::GUITextBox(ui, this->tpStatus, (const UTF8Char*)""));
	this->txtDNSServer->SetRect(104, 52, 200, 23, false);
	this->txtDNSServer->SetReadOnly(true);
	NEW_CLASS(this->btnDNSSwitch, UI::GUIButton(ui, this->tpStatus, (const UTF8Char*)"Switch"));
	this->btnDNSSwitch->SetRect(304, 52, 75, 23, false);
	this->btnDNSSwitch->HandleButtonClick(OnDNSSwitchClicked, this);
	NEW_CLASS(this->lblDNSList, UI::GUILabel(ui, this->tpStatus, (const UTF8Char*)"DNS List"));
	this->lblDNSList->SetRect(4, 76, 100, 23, false);
	NEW_CLASS(this->lbDNSList, UI::GUIListBox(ui, this->tpStatus, false));
	this->lbDNSList->SetRect(104, 76, 100, 240, false);
	NEW_CLASS(this->txtDNSServer2, UI::GUITextBox(ui, this->tpStatus, (const UTF8Char*)""));
	this->txtDNSServer2->SetRect(104, 316, 100, 23, false);
	NEW_CLASS(this->btnDNSAdd, UI::GUIButton(ui, this->tpStatus, (const UTF8Char*)"&Add"));
	this->btnDNSAdd->SetRect(204, 316, 75, 23, false);
	this->btnDNSAdd->HandleButtonClick(OnDNSAddClicked, this);
	NEW_CLASS(this->btnDNSSet, UI::GUIButton(ui, this->tpStatus, (const UTF8Char*)"&Set"));
	this->btnDNSSet->SetRect(284, 316, 75, 23, false);
	this->btnDNSSet->HandleButtonClick(OnDNSSetClicked, this);

	this->tpV4Main = this->tcMain->AddTabPage((const UTF8Char*)"Req v4");
	NEW_CLASS(this->lbV4Request, UI::GUIListBox(ui, this->tpV4Main, false));
	this->lbV4Request->SetRect(0, 0, 200, 100, false);
	this->lbV4Request->SetDockType(UI::GUIControl::DOCK_LEFT);
	this->lbV4Request->HandleSelectionChange(OnV4ReqSelChg, this);
	NEW_CLASS(this->hspV4Request, UI::GUIHSplitter(ui, this->tpV4Main, 3, false));
	NEW_CLASS(this->pnlV4Request, UI::GUIPanel(ui, this->tpV4Main));
	this->pnlV4Request->SetRect(0, 0, 100, 56, false);
	this->pnlV4Request->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblV4RequestTime, UI::GUILabel(ui, this->pnlV4Request, (const UTF8Char*)"Request Time"));
	this->lblV4RequestTime->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtV4RequestTime, UI::GUITextBox(ui, this->pnlV4Request, (const UTF8Char*)""));
	this->txtV4RequestTime->SetRect(104, 4, 200, 23, false);
	this->txtV4RequestTime->SetReadOnly(true);
	NEW_CLASS(this->lblV4RequestTTL, UI::GUILabel(ui, this->pnlV4Request, (const UTF8Char*)"TTL"));
	this->lblV4RequestTTL->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->txtV4RequestTTL, UI::GUITextBox(ui, this->pnlV4Request, (const UTF8Char*)""));
	this->txtV4RequestTTL->SetRect(104, 27, 200, 23, false);
	this->txtV4RequestTTL->SetReadOnly(true);
	NEW_CLASS(this->lbV4Answer, UI::GUIListBox(ui, this->tpV4Main, false));
	this->lbV4Answer->SetRect(0, 0, 200, 100, false);
	this->lbV4Answer->SetDockType(UI::GUIControl::DOCK_LEFT);
	this->lbV4Answer->HandleSelectionChange(OnV4AnsSelChg, this);
	NEW_CLASS(this->hspV4Answer, UI::GUIHSplitter(ui, this->tpV4Main, 3, false));
	NEW_CLASS(this->pnlV4Detail, UI::GUIPanel(ui, this->tpV4Main));
	this->pnlV4Detail->SetDockType(UI::GUIControl::DOCK_FILL);
	NEW_CLASS(this->lblV4AnsName, UI::GUILabel(ui, this->pnlV4Detail, (const UTF8Char*)"Name"));
	this->lblV4AnsName->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtV4AnsName, UI::GUITextBox(ui, this->pnlV4Detail, (const UTF8Char*)""));
	this->txtV4AnsName->SetRect(104, 4, 200, 23, false);
	this->txtV4AnsName->SetReadOnly(true);
	NEW_CLASS(this->lblV4AnsType, UI::GUILabel(ui, this->pnlV4Detail, (const UTF8Char*)"Type"));
	this->lblV4AnsType->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->txtV4AnsType, UI::GUITextBox(ui, this->pnlV4Detail, (const UTF8Char*)""));
	this->txtV4AnsType->SetRect(104, 28, 100, 23, false);
	this->txtV4AnsType->SetReadOnly(true);
	NEW_CLASS(this->lblV4AnsClass, UI::GUILabel(ui, this->pnlV4Detail, (const UTF8Char*)"Class"));
	this->lblV4AnsClass->SetRect(4, 52, 100, 23, false);
	NEW_CLASS(this->txtV4AnsClass, UI::GUITextBox(ui, this->pnlV4Detail, (const UTF8Char*)""));
	this->txtV4AnsClass->SetRect(104, 52, 100, 23, false);
	this->txtV4AnsClass->SetReadOnly(true);
	NEW_CLASS(this->lblV4AnsTTL, UI::GUILabel(ui, this->pnlV4Detail, (const UTF8Char*)"TTL"));
	this->lblV4AnsTTL->SetRect(4, 76, 100, 23, false);
	NEW_CLASS(this->txtV4AnsTTL, UI::GUITextBox(ui, this->pnlV4Detail, (const UTF8Char*)""));
	this->txtV4AnsTTL->SetRect(104, 76, 100, 23, false);
	this->txtV4AnsTTL->SetReadOnly(true);
	NEW_CLASS(this->lblV4AnsRD, UI::GUILabel(ui, this->pnlV4Detail, (const UTF8Char*)"RD"));
	this->lblV4AnsRD->SetRect(4, 100, 100, 23, false);
	NEW_CLASS(this->txtV4AnsRD, UI::GUITextBox(ui, this->pnlV4Detail, (const UTF8Char*)""));
	this->txtV4AnsRD->SetRect(104, 100, 160, 23, false);
	this->txtV4AnsRD->SetReadOnly(true);

	this->tpV6Main = this->tcMain->AddTabPage((const UTF8Char*)"Req v6");
	NEW_CLASS(this->lbV6Request, UI::GUIListBox(ui, this->tpV6Main, false));
	this->lbV6Request->SetRect(0, 0, 200, 100, false);
	this->lbV6Request->SetDockType(UI::GUIControl::DOCK_LEFT);
	this->lbV6Request->HandleSelectionChange(OnV6ReqSelChg, this);
	NEW_CLASS(this->hspV6Request, UI::GUIHSplitter(ui, this->tpV6Main, 3, false));
	NEW_CLASS(this->pnlV6Request, UI::GUIPanel(ui, this->tpV6Main));
	this->pnlV6Request->SetRect(0, 0, 100, 56, false);
	this->pnlV6Request->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblV6RequestTime, UI::GUILabel(ui, this->pnlV6Request, (const UTF8Char*)"Request Time"));
	this->lblV6RequestTime->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtV6RequestTime, UI::GUITextBox(ui, this->pnlV6Request, (const UTF8Char*)""));
	this->txtV6RequestTime->SetRect(104, 4, 200, 23, false);
	this->txtV6RequestTime->SetReadOnly(true);
	NEW_CLASS(this->lblV6RequestTTL, UI::GUILabel(ui, this->pnlV6Request, (const UTF8Char*)"TTL"));
	this->lblV6RequestTTL->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->txtV6RequestTTL, UI::GUITextBox(ui, this->pnlV6Request, (const UTF8Char*)""));
	this->txtV6RequestTTL->SetRect(104, 27, 200, 23, false);
	this->txtV6RequestTTL->SetReadOnly(true);
	NEW_CLASS(this->lbV6Answer, UI::GUIListBox(ui, this->tpV6Main, false));
	this->lbV6Answer->SetRect(0, 0, 200, 100, false);
	this->lbV6Answer->SetDockType(UI::GUIControl::DOCK_LEFT);
	this->lbV6Answer->HandleSelectionChange(OnV6AnsSelChg, this);
	NEW_CLASS(this->hspV6Answer, UI::GUIHSplitter(ui, this->tpV6Main, 3, false));
	NEW_CLASS(this->pnlV6Detail, UI::GUIPanel(ui, this->tpV6Main));
	this->pnlV6Detail->SetDockType(UI::GUIControl::DOCK_FILL);
	NEW_CLASS(this->lblV6AnsName, UI::GUILabel(ui, this->pnlV6Detail, (const UTF8Char*)"Name"));
	this->lblV6AnsName->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtV6AnsName, UI::GUITextBox(ui, this->pnlV6Detail, (const UTF8Char*)""));
	this->txtV6AnsName->SetRect(104, 4, 200, 23, false);
	this->txtV6AnsName->SetReadOnly(true);
	NEW_CLASS(this->lblV6AnsType, UI::GUILabel(ui, this->pnlV6Detail, (const UTF8Char*)"Type"));
	this->lblV6AnsType->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->txtV6AnsType, UI::GUITextBox(ui, this->pnlV6Detail, (const UTF8Char*)""));
	this->txtV6AnsType->SetRect(104, 28, 100, 23, false);
	this->txtV6AnsType->SetReadOnly(true);
	NEW_CLASS(this->lblV6AnsClass, UI::GUILabel(ui, this->pnlV6Detail, (const UTF8Char*)"Class"));
	this->lblV6AnsClass->SetRect(4, 52, 100, 23, false);
	NEW_CLASS(this->txtV6AnsClass, UI::GUITextBox(ui, this->pnlV6Detail, (const UTF8Char*)""));
	this->txtV6AnsClass->SetRect(104, 52, 100, 23, false);
	this->txtV6AnsClass->SetReadOnly(true);
	NEW_CLASS(this->lblV6AnsTTL, UI::GUILabel(ui, this->pnlV6Detail, (const UTF8Char*)"TTL"));
	this->lblV6AnsTTL->SetRect(4, 76, 100, 23, false);
	NEW_CLASS(this->txtV6AnsTTL, UI::GUITextBox(ui, this->pnlV6Detail, (const UTF8Char*)""));
	this->txtV6AnsTTL->SetRect(104, 76, 100, 23, false);
	this->txtV6AnsTTL->SetReadOnly(true);
	NEW_CLASS(this->lblV6AnsRD, UI::GUILabel(ui, this->pnlV6Detail, (const UTF8Char*)"RD"));
	this->lblV6AnsRD->SetRect(4, 100, 100, 23, false);
	NEW_CLASS(this->txtV6AnsRD, UI::GUITextBox(ui, this->pnlV6Detail, (const UTF8Char*)""));
	this->txtV6AnsRD->SetRect(104, 100, 160, 23, false);
	this->txtV6AnsRD->SetReadOnly(true);

	this->tpOthMain = this->tcMain->AddTabPage((const UTF8Char*)"Req Oth");
	NEW_CLASS(this->lbOthRequest, UI::GUIListBox(ui, this->tpOthMain, false));
	this->lbOthRequest->SetRect(0, 0, 200, 100, false);
	this->lbOthRequest->SetDockType(UI::GUIControl::DOCK_LEFT);
	this->lbOthRequest->HandleSelectionChange(OnOthReqSelChg, this);
	NEW_CLASS(this->hspOthRequest, UI::GUIHSplitter(ui, this->tpOthMain, 3, false));
	NEW_CLASS(this->pnlOthRequest, UI::GUIPanel(ui, this->tpOthMain));
	this->pnlOthRequest->SetRect(0, 0, 100, 56, false);
	this->pnlOthRequest->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblOthRequestTime, UI::GUILabel(ui, this->pnlOthRequest, (const UTF8Char*)"Request Time"));
	this->lblOthRequestTime->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtOthRequestTime, UI::GUITextBox(ui, this->pnlOthRequest, (const UTF8Char*)""));
	this->txtOthRequestTime->SetRect(104, 4, 200, 23, false);
	this->txtOthRequestTime->SetReadOnly(true);
	NEW_CLASS(this->lblOthRequestTTL, UI::GUILabel(ui, this->pnlOthRequest, (const UTF8Char*)"TTL"));
	this->lblOthRequestTTL->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->txtOthRequestTTL, UI::GUITextBox(ui, this->pnlOthRequest, (const UTF8Char*)""));
	this->txtOthRequestTTL->SetRect(104, 27, 200, 23, false);
	this->txtOthRequestTTL->SetReadOnly(true);
	NEW_CLASS(this->lbOthAnswer, UI::GUIListBox(ui, this->tpOthMain, false));
	this->lbOthAnswer->SetRect(0, 0, 200, 100, false);
	this->lbOthAnswer->SetDockType(UI::GUIControl::DOCK_LEFT);
	this->lbOthAnswer->HandleSelectionChange(OnOthAnsSelChg, this);
	NEW_CLASS(this->hspOthAnswer, UI::GUIHSplitter(ui, this->tpOthMain, 3, false));
	NEW_CLASS(this->pnlOthDetail, UI::GUIPanel(ui, this->tpOthMain));
	this->pnlOthDetail->SetDockType(UI::GUIControl::DOCK_FILL);
	NEW_CLASS(this->lblOthAnsName, UI::GUILabel(ui, this->pnlOthDetail, (const UTF8Char*)"Name"));
	this->lblOthAnsName->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtOthAnsName, UI::GUITextBox(ui, this->pnlOthDetail, (const UTF8Char*)""));
	this->txtOthAnsName->SetRect(104, 4, 200, 23, false);
	this->txtOthAnsName->SetReadOnly(true);
	NEW_CLASS(this->lblOthAnsType, UI::GUILabel(ui, this->pnlOthDetail, (const UTF8Char*)"Type"));
	this->lblOthAnsType->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->txtOthAnsType, UI::GUITextBox(ui, this->pnlOthDetail, (const UTF8Char*)""));
	this->txtOthAnsType->SetRect(104, 28, 100, 23, false);
	this->txtOthAnsType->SetReadOnly(true);
	NEW_CLASS(this->lblOthAnsClass, UI::GUILabel(ui, this->pnlOthDetail, (const UTF8Char*)"Class"));
	this->lblOthAnsClass->SetRect(4, 52, 100, 23, false);
	NEW_CLASS(this->txtOthAnsClass, UI::GUITextBox(ui, this->pnlOthDetail, (const UTF8Char*)""));
	this->txtOthAnsClass->SetRect(104, 52, 100, 23, false);
	this->txtOthAnsClass->SetReadOnly(true);
	NEW_CLASS(this->lblOthAnsTTL, UI::GUILabel(ui, this->pnlOthDetail, (const UTF8Char*)"TTL"));
	this->lblOthAnsTTL->SetRect(4, 76, 100, 23, false);
	NEW_CLASS(this->txtOthAnsTTL, UI::GUITextBox(ui, this->pnlOthDetail, (const UTF8Char*)""));
	this->txtOthAnsTTL->SetRect(104, 76, 100, 23, false);
	this->txtOthAnsTTL->SetReadOnly(true);
	NEW_CLASS(this->lblOthAnsRD, UI::GUILabel(ui, this->pnlOthDetail, (const UTF8Char*)"RD"));
	this->lblOthAnsRD->SetRect(4, 100, 100, 23, false);
	NEW_CLASS(this->txtOthAnsRD, UI::GUITextBox(ui, this->pnlOthDetail, (const UTF8Char*)""));
	this->txtOthAnsRD->SetRect(104, 100, 160, 23, false);
	this->txtOthAnsRD->SetReadOnly(true);

	this->tpTarget = this->tcMain->AddTabPage((const UTF8Char*)"Target");
	NEW_CLASS(this->lbTarget, UI::GUIListBox(ui, this->tpTarget, false));
	this->lbTarget->SetRect(0, 0, 150, 100, false);
	this->lbTarget->SetDockType(UI::GUIControl::DOCK_LEFT);
	this->lbTarget->HandleSelectionChange(OnTargetSelChg, this);
	NEW_CLASS(this->hspTarget, UI::GUIHSplitter(ui, this->tpTarget, 3, false));
	NEW_CLASS(this->tcTarget, UI::GUITabControl(ui, this->tpTarget));
	this->tcTarget->SetDockType(UI::GUIControl::DOCK_FILL);
	this->tpTargetInfo = this->tcTarget->AddTabPage((const UTF8Char*)"Info");
	NEW_CLASS(this->lblTargetName, UI::GUILabel(ui, this->tpTargetInfo, (const UTF8Char*)"Name"));
	this->lblTargetName->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtTargetName, UI::GUITextBox(ui, this->tpTargetInfo, (const UTF8Char*)""));
	this->txtTargetName->SetRect(104, 4, 200, 23, false);
	this->txtTargetName->SetReadOnly(true);
	NEW_CLASS(this->lblTargetCountry, UI::GUILabel(ui, this->tpTargetInfo, (const UTF8Char*)"Country"));
	this->lblTargetCountry->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->txtTargetCountry, UI::GUITextBox(ui, this->tpTargetInfo, (const UTF8Char*)""));
	this->txtTargetCountry->SetRect(104, 28, 100, 23, false);
	this->txtTargetCountry->SetReadOnly(true);
	NEW_CLASS(this->lblTargetDomains, UI::GUILabel(ui, this->tpTargetInfo, (const UTF8Char*)"Domains"));
	this->lblTargetDomains->SetRect(4, 52, 100, 23, false);
	NEW_CLASS(this->lbTargetDomains, UI::GUIListBox(ui, this->tpTargetInfo, false));
	this->lbTargetDomains->SetRect(104, 52, 400, 300, false);
	this->tpTargetWhois = this->tcTarget->AddTabPage((const UTF8Char*)"Whois");
	NEW_CLASS(this->txtTargetWhois, UI::GUITextBox(ui, this->tpTargetWhois, (const UTF8Char*)"", true));
	this->txtTargetWhois->SetDockType(UI::GUIControl::DOCK_FILL);
	this->txtTargetWhois->SetReadOnly(true);

	this->tpSearch = this->tcMain->AddTabPage((const UTF8Char*)"Search");
	NEW_CLASS(this->pnlSearch, UI::GUIPanel(ui, this->tpSearch));
	this->pnlSearch->SetRect(0, 0, 100, 31, false);
	this->pnlSearch->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblSearchIPRange, UI::GUILabel(ui, this->pnlSearch, (const UTF8Char*)"IP Range"));
	this->lblSearchIPRange->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtSearchIPRange, UI::GUITextBox(ui, this->pnlSearch, (const UTF8Char*)"202.128.224.0"));
	this->txtSearchIPRange->SetRect(104, 4, 100, 23, false);
	NEW_CLASS(this->lblSearchMask, UI::GUILabel(ui, this->pnlSearch, (const UTF8Char*)"Mask"));
	this->lblSearchMask->SetRect(204, 4, 100, 23, false);
	NEW_CLASS(this->txtSearchMask, UI::GUITextBox(ui, this->pnlSearch, (const UTF8Char*)"19"));
	this->txtSearchMask->SetRect(304, 4, 100, 23, false);
	NEW_CLASS(this->btnSearch, UI::GUIButton(ui, this->pnlSearch, (const UTF8Char*)"Search"));
	this->btnSearch->SetRect(404, 4, 75, 23, false);
	this->btnSearch->HandleButtonClick(OnSearchClicked, this);
	NEW_CLASS(this->lbSearch, UI::GUIListBox(ui, this->tpSearch, false));
	this->lbSearch->SetRect(0, 0, 200, 100, false);
	this->lbSearch->SetDockType(UI::GUIControl::DOCK_LEFT);
	this->lbSearch->HandleSelectionChange(OnSReqSelChg, this);
	NEW_CLASS(this->hspSearch, UI::GUIHSplitter(ui, this->tpSearch, 3, false));
	NEW_CLASS(this->pnlSRequest, UI::GUIPanel(ui, this->tpSearch));
	this->pnlSRequest->SetRect(0, 0, 100, 56, false);
	this->pnlSRequest->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblSRequestTime, UI::GUILabel(ui, this->pnlSRequest, (const UTF8Char*)"Request Time"));
	this->lblSRequestTime->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtSRequestTime, UI::GUITextBox(ui, this->pnlSRequest, (const UTF8Char*)""));
	this->txtSRequestTime->SetRect(104, 4, 200, 23, false);
	this->txtSRequestTime->SetReadOnly(true);
	NEW_CLASS(this->lblSRequestTTL, UI::GUILabel(ui, this->pnlSRequest, (const UTF8Char*)"TTL"));
	this->lblSRequestTTL->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->txtSRequestTTL, UI::GUITextBox(ui, this->pnlSRequest, (const UTF8Char*)""));
	this->txtSRequestTTL->SetRect(104, 27, 200, 23, false);
	this->txtSRequestTTL->SetReadOnly(true);
	NEW_CLASS(this->lbSAnswer, UI::GUIListBox(ui, this->tpSearch, false));
	this->lbSAnswer->SetRect(0, 0, 200, 100, false);
	this->lbSAnswer->SetDockType(UI::GUIControl::DOCK_LEFT);
	this->lbSAnswer->HandleSelectionChange(OnSAnsSelChg, this);
	NEW_CLASS(this->hspSAnswer, UI::GUIHSplitter(ui, this->tpSearch, 3, false));
	NEW_CLASS(this->pnlSDetail, UI::GUIPanel(ui, this->tpSearch));
	this->pnlSDetail->SetDockType(UI::GUIControl::DOCK_FILL);
	NEW_CLASS(this->lblSAnsName, UI::GUILabel(ui, this->pnlSDetail, (const UTF8Char*)"Name"));
	this->lblSAnsName->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtSAnsName, UI::GUITextBox(ui, this->pnlSDetail, (const UTF8Char*)""));
	this->txtSAnsName->SetRect(104, 4, 200, 23, false);
	this->txtSAnsName->SetReadOnly(true);
	NEW_CLASS(this->lblSAnsType, UI::GUILabel(ui, this->pnlSDetail, (const UTF8Char*)"Type"));
	this->lblSAnsType->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->txtSAnsType, UI::GUITextBox(ui, this->pnlSDetail, (const UTF8Char*)""));
	this->txtSAnsType->SetRect(104, 28, 100, 23, false);
	this->txtSAnsType->SetReadOnly(true);
	NEW_CLASS(this->lblSAnsClass, UI::GUILabel(ui, this->pnlSDetail, (const UTF8Char*)"Class"));
	this->lblSAnsClass->SetRect(4, 52, 100, 23, false);
	NEW_CLASS(this->txtSAnsClass, UI::GUITextBox(ui, this->pnlSDetail, (const UTF8Char*)""));
	this->txtSAnsClass->SetRect(104, 52, 100, 23, false);
	this->txtSAnsClass->SetReadOnly(true);
	NEW_CLASS(this->lblSAnsTTL, UI::GUILabel(ui, this->pnlSDetail, (const UTF8Char*)"TTL"));
	this->lblSAnsTTL->SetRect(4, 76, 100, 23, false);
	NEW_CLASS(this->txtSAnsTTL, UI::GUITextBox(ui, this->pnlSDetail, (const UTF8Char*)""));
	this->txtSAnsTTL->SetRect(104, 76, 100, 23, false);
	this->txtSAnsTTL->SetReadOnly(true);
	NEW_CLASS(this->lblSAnsRD, UI::GUILabel(ui, this->pnlSDetail, (const UTF8Char*)"RD"));
	this->lblSAnsRD->SetRect(4, 100, 100, 23, false);
	NEW_CLASS(this->txtSAnsRD, UI::GUITextBox(ui, this->pnlSDetail, (const UTF8Char*)""));
	this->txtSAnsRD->SetRect(104, 100, 160, 23, false);
	this->txtSAnsRD->SetReadOnly(true);

	this->tpBlackList = this->tcMain->AddTabPage((const UTF8Char*)"Blacklist");
	NEW_CLASS(this->pnlBlackList, UI::GUIPanel(ui, this->tpBlackList));
	this->pnlBlackList->SetRect(0, 0, 100, 32, false);
	this->pnlBlackList->SetDockType(UI::GUIControl::DOCK_BOTTOM);
	NEW_CLASS(this->lbBlackList, UI::GUIListBox(ui, this->tpBlackList, false));
	this->lbBlackList->SetDockType(UI::GUIControl::DOCK_FILL);
	NEW_CLASS(this->txtBlackList, UI::GUITextBox(ui, this->pnlBlackList, (const UTF8Char*)""));
	this->txtBlackList->SetRect(4, 4, 200, 23, false);
	NEW_CLASS(this->btnBlackList, UI::GUIButton(ui, this->pnlBlackList, (const UTF8Char*)"Add"));
	this->btnBlackList->SetRect(204, 4, 75, 23, false);
	this->btnBlackList->HandleButtonClick(OnBlackListClicked, this);

	this->tpLog = this->tcMain->AddTabPage((const UTF8Char*)"Log");
	NEW_CLASS(this->txtLog, UI::GUITextBox(ui, this->tpLog, (const UTF8Char*)"", false));
	this->txtLog->SetReadOnly(true);
	this->txtLog->SetRect(0, 0, 100, 23, false);
	this->txtLog->SetDockType(UI::GUIControl::DOCK_BOTTOM);
	NEW_CLASS(this->lbLog, UI::GUIListBox(ui, this->tpLog, false));
	this->lbLog->SetDockType(UI::GUIControl::DOCK_FILL);

	this->tpClient = this->tcMain->AddTabPage((const UTF8Char*)"Client");
	NEW_CLASS(this->lbClientIP, UI::GUIListBox(ui, this->tpClient, false));
	this->lbClientIP->SetRect(0, 0, 150, 23, false);
	this->lbClientIP->SetDockType(UI::GUIControl::DOCK_LEFT);
	this->lbClientIP->HandleSelectionChange(OnClientSelChg, this);
	NEW_CLASS(this->hspClient, UI::GUIHSplitter(ui, this->tpClient, 3, false));
	NEW_CLASS(this->lvClient, UI::GUIListView(ui, this->tpClient, UI::GUIListView::LVSTYLE_TABLE, 2));
	this->lvClient->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvClient->AddColumn((const UTF8Char*)"Time", 200);
	this->lvClient->AddColumn((const UTF8Char*)"Count", 100);

	NEW_CLASS(this->cliInfoMut, Sync::Mutex());
	NEW_CLASS(this->cliInfos, Data::UInt32Map<ClientInfo*>());
	this->cliChg = false;

	NEW_CLASS(this->log, IO::LogTool());
	NEW_CLASS(this->logger, UI::ListBoxLogger(this, this->lbLog, 500, true));
	this->log->AddLogHandler(this->logger, IO::ILogHandler::LOG_LEVEL_RAW);

	NEW_CLASS(this->v4ansList, Data::ArrayList<Net::DNSClient::RequestAnswer *>());
	NEW_CLASS(this->v6ansList, Data::ArrayList<Net::DNSClient::RequestAnswer *>());
	NEW_CLASS(this->othansList, Data::ArrayList<Net::DNSClient::RequestAnswer *>());
	NEW_CLASS(this->v4sansList, Data::ArrayList<Net::DNSClient::RequestAnswer *>());
	NEW_CLASS(this->whois, Net::WhoisHandler(this->core->GetSocketFactory()));
	NEW_CLASS(this->proxy, Net::DNSProxy(this->core->GetSocketFactory(), true));
	this->proxy->HandleDNSRequest(OnDNSRequest, this);
	this->AddTimer(1000, OnTimerTick, this);
	UInt32 svrIP = this->proxy->GetServerIP();
	Net::SocketUtil::GetIPv4Name(sbuff, svrIP);
	this->txtDNSServer2->SetText(sbuff);

	this->UpdateDNSList();
}

SSWR::AVIRead::AVIRDNSProxyForm::~AVIRDNSProxyForm()
{
	UOSInt i;
	UOSInt j;
	Data::ArrayList<ClientInfo*> *cliInfoList;
	ClientInfo *cli;
	DEL_CLASS(this->proxy);
	Net::DNSClient::FreeAnswers(this->v4ansList);
	Net::DNSClient::FreeAnswers(this->v6ansList);
	Net::DNSClient::FreeAnswers(this->othansList);
	Net::DNSClient::FreeAnswers(this->v4sansList);
	DEL_CLASS(this->cliInfoMut);
	cliInfoList = this->cliInfos->GetValues();
	i = cliInfoList->GetCount();
	while (i-- > 0)
	{
		cli = cliInfoList->GetItem(i);
		j = cli->hourInfos->GetCount();
		while (j-- > 0)
		{
			MemFree(cli->hourInfos->GetItem(j));
		}
		DEL_CLASS(cli->hourInfos);
		DEL_CLASS(cli->mut);
		MemFree(cli);
	}
	DEL_CLASS(this->cliInfos);
	DEL_CLASS(this->whois);
	DEL_CLASS(this->v4ansList);
	DEL_CLASS(this->v6ansList);
	DEL_CLASS(this->othansList);
	DEL_CLASS(this->v4sansList);
	DEL_CLASS(this->log);
	DEL_CLASS(this->logger);
}

void SSWR::AVIRead::AVIRDNSProxyForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}

Bool SSWR::AVIRead::AVIRDNSProxyForm::IsError()
{
	return this->proxy->IsError();
}

void SSWR::AVIRead::AVIRDNSProxyForm::SetDNSList(Data::ArrayList<UInt32> *dnsList)
{
	UOSInt i;
	UOSInt j;
	j = dnsList->GetCount();
	if (j > 0)
	{
		this->proxy->SetServerIP(dnsList->GetItem(0));
		i = 1;
		while (i < j)
		{
			this->proxy->AddDNSIP(dnsList->GetItem(i));
			i++;
		}
		this->UpdateDNSList();
	}
}

void SSWR::AVIRead::AVIRDNSProxyForm::SetDisableV6(Bool disableV6)
{
	this->proxy->SetDisableV6(disableV6);
	this->chkDisableV6->SetChecked(disableV6);
}

void SSWR::AVIRead::AVIRDNSProxyForm::AddBlacklist(const UTF8Char *blackList)
{
	if (blackList == 0 || blackList[0] == 0)
		return;
	const UTF8Char *bl = Text::StrCopyNew(blackList);
	this->proxy->AddBlackList(bl);
	Text::StrDelNew(bl);
	this->UpdateBlackList();
}
