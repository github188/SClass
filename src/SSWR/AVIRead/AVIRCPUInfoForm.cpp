#include "Stdafx.h"
#include "Manage/CPUDB.h"
#include "Manage/CPUInfoDetail.h"
#include "Net/HTTPClient.h"
#include "SSWR/AVIRead/AVIRCPUInfoForm.h"
#include "Text/MyStringFloat.h"
#include "Text/StringBuilderUTF8.h"
#include "UI/MessageDialog.h"
#include "Win32/Clipboard.h"

void __stdcall SSWR::AVIRead::AVIRCPUInfoForm::OnUploadClick(void *userObj)
{
	SSWR::AVIRead::AVIRCPUInfoForm *me = (SSWR::AVIRead::AVIRCPUInfoForm*)userObj;
#if defined(CPU_X86_32) || defined(CPU_X86_64)
	UTF8Char u8buff[512];
	Manage::CPUInfo cpu;
	if (cpu.GetCPUName(u8buff) == 0)
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Error in getting CPU Name", (const UTF8Char*)"Error", me);
	}
	else
	{
		const UTF8Char *cpuModel = Manage::CPUDB::X86CPUNameToModel(u8buff);
		if (cpuModel == 0)
		{
			Int32 respStatus;
			Text::StringBuilderUTF8 sbData;
			Net::SocketFactory *sockf = me->core->GetSocketFactory();
			Text::StringBuilderUTF8 sbURL;
			sbURL.Append((const UTF8Char*)"http://sswroom.no-ip.org:5080/benchmark/cpuinfo?family=");
			sbURL.AppendI32(cpu.GetFamilyId());
			sbURL.Append((const UTF8Char*)"&modelId=");
			sbURL.AppendI32(cpu.GetModelId());
			sbURL.Append((const UTF8Char*)"&stepping=");
			sbURL.AppendI32(cpu.GetStepping());

			sbData.Append(u8buff);
			Net::HTTPClient *cli;
			cli = Net::HTTPClient::CreateConnect(sockf, me->core->GetSSLEngine(), sbURL.ToString(), "POST", false);
			Text::StrUOSInt(u8buff, sbData.GetLength());
			cli->AddHeader((const UTF8Char*)"Content-Length", u8buff);
			cli->Write(sbData.ToString(), sbData.GetLength());
			respStatus = cli->GetRespStatus();
			DEL_CLASS(cli);
			if (respStatus == 200)
			{
				UI::MessageDialog::ShowDialog((const UTF8Char*)"Upload success", (const UTF8Char*)"CPUInfo", me);
			}
			else
			{
				UI::MessageDialog::ShowDialog((const UTF8Char*)"Error in uploading to server", (const UTF8Char*)"CPUInfo", me);
			}
		}
		else
		{
			UI::MessageDialog::ShowDialog((const UTF8Char*)"CPU Info already exist", (const UTF8Char*)"CPUInfo", me);
		}
	}

#endif
}

void __stdcall SSWR::AVIRead::AVIRCPUInfoForm::OnCopyInfoClick(void *userObj)
{
	SSWR::AVIRead::AVIRCPUInfoForm *me = (SSWR::AVIRead::AVIRCPUInfoForm*)userObj;
#if defined(CPU_X86_32) || defined(CPU_X86_64)
	UTF8Char u8buff[512];
	Manage::CPUInfo cpu;
	if (cpu.GetCPUName(u8buff) == 0)
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Error in getting CPU Name", (const UTF8Char*)"Error", me);
	}
	else
	{
		Text::StringBuilderUTF8 sb;
		sb.AppendI32(cpu.GetFamilyId());
		sb.AppendChar('\t', 1);
		sb.AppendI32(cpu.GetModelId());
		sb.AppendChar('\t', 1);
		sb.AppendI32(cpu.GetStepping());
		sb.AppendChar('\t', 1);
		sb.Append(u8buff);
		Win32::Clipboard::SetString(me->GetHandle(), sb.ToString());
	}
#endif
}

SSWR::AVIRead::AVIRCPUInfoForm::AVIRCPUInfoForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core) : UI::GUIForm(parent, 800, 600, ui)
{
	this->SetText((const UTF8Char*)"CPU Info");
	this->SetFont(0, 8.25, false);
	
	this->core = core;
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));

	NEW_CLASS(this->tcMain, UI::GUITabControl(ui, this));
	this->tcMain->SetDockType(UI::GUIControl::DOCK_FILL);

	this->tpCPU = this->tcMain->AddTabPage((const UTF8Char*)"CPU");
	NEW_CLASS(this->pnlMain, UI::GUIPanel(ui, this->tpCPU));
	this->pnlMain->SetRect(0, 0, 100, 31, false);
	this->pnlMain->SetDockType(UI::GUIControl::DOCK_BOTTOM);
	NEW_CLASS(this->btnUpload, UI::GUIButton(ui, this->pnlMain, (const UTF8Char*)"Upload Info"));
	this->btnUpload->SetRect(4, 4, 100, 23, false);
	this->btnUpload->HandleButtonClick(OnUploadClick, this);
	NEW_CLASS(this->btnCopyInfo, UI::GUIButton(ui, this->pnlMain, (const UTF8Char*)"Copy Info"));
	this->btnCopyInfo->SetRect(108, 4, 100, 23, false);
	this->btnCopyInfo->HandleButtonClick(OnCopyInfoClick, this);
	NEW_CLASS(this->lvMain, UI::GUIListView(ui, this->tpCPU, UI::GUIListView::LVSTYLE_TABLE, 2));
	this->lvMain->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvMain->SetFullRowSelect(true);
	this->lvMain->SetShowGrid(true);
	this->lvMain->AddColumn((const UTF8Char*)"Name", 200);
	this->lvMain->AddColumn((const UTF8Char*)"Value", 550);

	this->tpCache = this->tcMain->AddTabPage((const UTF8Char*)"Cache");
	NEW_CLASS(this->lbCache, UI::GUIListBox(ui, this->tpCache, false));
	this->lbCache->SetDockType(UI::GUIControl::DOCK_FILL);

	this->tpFeature = this->tcMain->AddTabPage((const UTF8Char*)"Feature");
	NEW_CLASS(this->lvFeature, UI::GUIListView(ui, this->tpFeature, UI::GUIListView::LVSTYLE_TABLE, 4));
	this->lvFeature->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvFeature->SetFullRowSelect(true);
	this->lvFeature->SetShowGrid(true);
	this->lvFeature->AddColumn((const UTF8Char*)"Short Name", 100);
	this->lvFeature->AddColumn((const UTF8Char*)"Value", 50);
	this->lvFeature->AddColumn((const UTF8Char*)"Name", 150);
	this->lvFeature->AddColumn((const UTF8Char*)"Description", 450);

	Manage::CPUInfoDetail cpu;
	Text::StringBuilderUTF8 sb;
	UOSInt i = 0;
	UOSInt j = cpu.GetInfoCnt();
	UOSInt k;
	while (i < j)
	{
		sb.ClearStr();
		cpu.GetInfoName(i, &sb);
		k = this->lvMain->AddItem(sb.ToString(), 0);
		sb.ClearStr();
		cpu.GetInfoValue(i, &sb);
		this->lvMain->SetSubItem(k, 1, sb.ToString());
		i++;
	}
	Data::ArrayList<const UTF8Char *> infoList;
	cpu.GetCacheInfoList(&infoList);
	i = 0;
	j = infoList.GetCount();
	while (i < j)
	{
		this->lbCache->AddItem(infoList.GetItem(i), 0);
		i++;
	}

	Int32 flags1;
	Int32 flags2;
	cpu.GetFeatureFlags(&flags1, &flags2);
	i = 0;
	while (i < 32)
	{
		k = this->lvFeature->AddItem(Manage::CPUInfo::GetFeatureShortName(i), 0);
		if (flags1 & (1 << i))
		{
			this->lvFeature->SetSubItem(k, 1, (const UTF8Char*)"1");
		}
		else
		{
			this->lvFeature->SetSubItem(k, 1, (const UTF8Char*)"0");
		}
		this->lvFeature->SetSubItem(k, 2, Manage::CPUInfo::GetFeatureName(i));
		this->lvFeature->SetSubItem(k, 3, Manage::CPUInfo::GetFeatureDesc(i));
		i++;
	}
	i = 0;
	while (i < 32)
	{
		k = this->lvFeature->AddItem(Manage::CPUInfo::GetFeatureShortName(i + 32), 0);
		if (flags2 & (1 << i))
		{
			this->lvFeature->SetSubItem(k, 1, (const UTF8Char*)"1");
		}
		else
		{
			this->lvFeature->SetSubItem(k, 1, (const UTF8Char*)"0");
		}
		this->lvFeature->SetSubItem(k, 2, Manage::CPUInfo::GetFeatureName(i + 32));
		this->lvFeature->SetSubItem(k, 3, Manage::CPUInfo::GetFeatureDesc(i + 32));
		i++;
	}

	Double t;
	Int32 r;
	UTF8Char u8buff[256];
	sb.ClearStr();
	if (cpu.GetCPUName(u8buff))
	{
		sb.Append(u8buff);
		k = this->lvMain->AddItem((const UTF8Char*)"CPU Name", 0);
		this->lvMain->SetSubItem(k, 1, sb.ToString());
	}
	if (cpu.GetCPURatio(&r))
	{
		sb.ClearStr();
		sb.AppendI32(r);
		k = this->lvMain->AddItem((const UTF8Char*)"Ratio", 0);
		this->lvMain->SetSubItem(k, 1, sb.ToString());
	}
	if (cpu.GetCPUTurboRatio(&r))
	{
		sb.ClearStr();
		sb.AppendI32(r);
		k = this->lvMain->AddItem((const UTF8Char*)"Turbo Ratio", 0);
		this->lvMain->SetSubItem(k, 1, sb.ToString());
	}
	if (cpu.GetCPUTCC(&t))
	{
		sb.ClearStr();
		Text::SBAppendF64(&sb, t);
		k = this->lvMain->AddItem((const UTF8Char*)"TCC", 0);
		this->lvMain->SetSubItem(k, 1, sb.ToString());
	}
	if (cpu.GetCPUTemp(0, &t))
	{
		sb.ClearStr();
		Text::SBAppendF64(&sb, t);
		k = this->lvMain->AddItem((const UTF8Char*)"Temp", 0);
		this->lvMain->SetSubItem(k, 1, sb.ToString());
	}
}

SSWR::AVIRead::AVIRCPUInfoForm::~AVIRCPUInfoForm()
{
}

void SSWR::AVIRead::AVIRCPUInfoForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}
