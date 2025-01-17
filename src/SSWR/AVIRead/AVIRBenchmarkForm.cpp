#include "Stdafx.h"
#include "IO/FileStream.h"
#include "IO/SystemInfo.h"
#include "Manage/Benchmark.h"
#include "Manage/CPUInfo.h"
#include "Manage/HiResClock.h"
#include "SSWR/AVIRead/AVIRBenchmarkForm.h"
#include "Sync/Thread.h"
#include "Text/MyStringFloat.h"
#include "Text/StringBuilderUTF8.h"
#include "Text/UTF8Writer.h"
#include "UI/FileDialog.h"
#include "UI/MessageDialog.h"

UTF8Char *SSWR::AVIRead::AVIRBenchmarkForm::ByteDisp(UTF8Char *sbuff, UOSInt byteSize)
{
	if (byteSize >= 1048576)
	{
		return Text::StrConcat(Text::StrUOSInt(sbuff, byteSize >> 20), (const UTF8Char*)"MB");
	}
	else if (byteSize >= 1024)
	{
		return Text::StrConcat(Text::StrUOSInt(sbuff, byteSize >> 10), (const UTF8Char*)"KB");
	}
	else
	{
		return Text::StrConcat(Text::StrUOSInt(sbuff, byteSize), (const UTF8Char*)"B");
	}
}

void SSWR::AVIRead::AVIRBenchmarkForm::StartTest(UOSInt startSize, UOSInt buffSize)
{
	UOSInt i;
	UTF8Char sbuff[32];
	Double oriT;
	Double t;
	Manage::HiResClock clk;
	UInt8 *buff1;
	UInt8 *buff2;
	UOSInt loopCnt;
	UOSInt currSize;
	SSWR::AVIRead::AVIRBenchmarkForm::TestResult *result;

	UOSInt testCnt = 0;
	currSize = startSize;
	while (currSize <= buffSize)
	{
		testCnt += 3;
		currSize = currSize << 1;
	}
	this->processing = true;
	this->pbMain->ProgressStart((const UTF8Char*)"Testing", testCnt);
	testCnt = 0;

	this->ClearResult();
	this->lvCopy->ClearItems();
	this->lvRead->ClearItems();
	this->lvWrite->ClearItems();
	buff1 = MemAllocA64(UInt8, buffSize);
	buff2 = MemAllocA64(UInt8, buffSize);
	clk.Start();
	oriT = clk.GetTimeDiff();

	currSize = startSize;
	while (currSize <= buffSize)
	{
		loopCnt = 1;
		while (true)
		{
			clk.Start();
			Benchmark_MemCopyTest(buff1, buff2, currSize, loopCnt);
			t = clk.GetTimeDiff() - oriT;
			if (t < 0.05)
			{
				loopCnt = loopCnt << 4;
			}
			else if (t >= 1)
			{
				Double rate = (Double)(currSize * (UInt64)loopCnt) / t * 2.0;

				ByteDisp(sbuff, currSize);
				i = this->lvCopy->AddItem(sbuff, 0);
				Text::StrDoubleFmt(sbuff, rate, "0.0");
				this->lvCopy->SetSubItem(i, 1, sbuff);

				result = MemAlloc(SSWR::AVIRead::AVIRBenchmarkForm::TestResult, 1);
				result->tt = TT_COPY;
				result->testSize = currSize;
				result->resultRate = rate;
				this->resultList->Add(result);
				break;
			}
			else
			{
				loopCnt = loopCnt << 1;
			}
		}
		testCnt++;
		this->pbMain->ProgressUpdate(testCnt, testCnt);
		this->ui->ProcessMessages();
		currSize = currSize << 1;
	}

	currSize = startSize;
	while (currSize <= buffSize)
	{
		loopCnt = 1;
		while (true)
		{
			clk.Start();
			Benchmark_MemWriteTest(buff1, buff2, currSize, loopCnt);
			t = clk.GetTimeDiff() - oriT;
			if (t < 0.05)
			{
				loopCnt = loopCnt << 4;
			}
			else if (t >= 1)
			{
				Double rate = (Double)(currSize * (UInt64)loopCnt) / t;

				ByteDisp(sbuff, currSize);
				i = this->lvWrite->AddItem(sbuff, 0);
				Text::StrDoubleFmt(sbuff, rate, "0.0");
				this->lvWrite->SetSubItem(i, 1, sbuff);

				result = MemAlloc(SSWR::AVIRead::AVIRBenchmarkForm::TestResult, 1);
				result->tt = TT_WRITE;
				result->testSize = currSize;
				result->resultRate = rate;
				this->resultList->Add(result);
				break;
			}
			else
			{
				loopCnt = loopCnt << 1;
			}
		}
		testCnt++;
		this->pbMain->ProgressUpdate(testCnt, testCnt);
		this->ui->ProcessMessages();
		currSize = currSize << 1;
	}

	currSize = startSize;
	while (currSize <= buffSize)
	{
		loopCnt = 1;
		while (true)
		{
			clk.Start();
			Benchmark_MemReadTest(buff1, buff2, currSize, loopCnt);
			t = clk.GetTimeDiff() - oriT;
			if (t < 0.05)
			{
				loopCnt = loopCnt << 4;
			}
			else if (t >= 1)
			{
				Double rate = (Double)(currSize * (UInt64)loopCnt) / t;

				ByteDisp(sbuff, currSize);
				i = this->lvRead->AddItem(sbuff, 0);
				Text::StrDoubleFmt(sbuff, rate, "0.0");
				this->lvRead->SetSubItem(i, 1, sbuff);

				result = MemAlloc(SSWR::AVIRead::AVIRBenchmarkForm::TestResult, 1);
				result->tt = TT_READ;
				result->testSize = currSize;
				result->resultRate = rate;
				this->resultList->Add(result);
				break;
			}
			else
			{
				loopCnt = loopCnt << 1;
			}
		}
		testCnt++;
		this->pbMain->ProgressUpdate(testCnt, testCnt);
		this->ui->ProcessMessages();
		currSize = currSize << 1;
	}

	this->pbMain->ProgressEnd();
	this->processing = false;

	MemFreeA64(buff1);
	MemFreeA64(buff2);
}

void SSWR::AVIRead::AVIRBenchmarkForm::ClearResult()
{
	UOSInt i = this->resultList->GetCount();
	while (i-- > 0)
	{
		MemFree(this->resultList->GetItem(i));
	}
	this->resultList->Clear();
}

void __stdcall SSWR::AVIRead::AVIRBenchmarkForm::OnStartClicked(void *userObj)
{
	SSWR::AVIRead::AVIRBenchmarkForm *me = (SSWR::AVIRead::AVIRBenchmarkForm*)userObj;
	if (me->processing)
		return;
	me->StartTest(128, 64 << 20); //64 MB
}

void __stdcall SSWR::AVIRead::AVIRBenchmarkForm::OnQuickClicked(void *userObj)
{
	SSWR::AVIRead::AVIRBenchmarkForm *me = (SSWR::AVIRead::AVIRBenchmarkForm*)userObj;
	if (me->processing)
		return;
	me->StartTest(64 << 20, 64 << 20); //64 MB
}

void __stdcall SSWR::AVIRead::AVIRBenchmarkForm::OnSaveClicked(void *userObj)
{
	SSWR::AVIRead::AVIRBenchmarkForm *me = (SSWR::AVIRead::AVIRBenchmarkForm*)userObj;
	UTF8Char u8buff[256];
	UI::FileDialog *dlg;
	UOSInt i;
	UOSInt j;
	SSWR::AVIRead::AVIRBenchmarkForm::TestResult *result;

	if (me->processing)
		return;

	if (me->resultList->GetCount() <= 0)
	{
		UI::MessageDialog::ShowDialog((const UTF8Char *)"No result", (const UTF8Char *)"Error", me);
		return;
	}
	NEW_CLASS(dlg, UI::FileDialog(L"SSWR", L"AVIRead", L"BenchmarkSave", true));
	dlg->AddFilter((const UTF8Char*)"*.txt", (const UTF8Char*)"Result File");
	{
		Text::StringBuilderUTF8 sb;
		sb.Append((const UTF8Char*)"Benchmark_");
		me->txtPlatform->GetText(&sb);
		sb.Append((const UTF8Char*)".txt");
		dlg->SetFileName(sb.ToString());
	}
	if (dlg->ShowDialog(me->GetHandle()))
	{
		Text::StringBuilderUTF8 sb;
		IO::SystemInfo sysInfo;
		IO::FileStream *fs;
		Text::UTF8Writer *writer;
		NEW_CLASS(fs, IO::FileStream(dlg->GetFileName(), IO::FileStream::FILE_MODE_CREATE, IO::FileStream::FILE_SHARE_DENY_NONE, IO::FileStream::BT_NORMAL));
		NEW_CLASS(writer, Text::UTF8Writer(fs));
		sb.ClearStr();
		sb.Append((const UTF8Char*)"Platform: ");
		if (sysInfo.GetPlatformName(u8buff))
		{
			sb.Append(u8buff);
		}
		else
		{
			sb.Append((const UTF8Char*)"-");
		}
		writer->WriteLine(sb.ToString());

		Manage::CPUInfo cpu;
		sb.ClearStr();
		sb.Append((const UTF8Char*)"CPU: ");
		if (cpu.GetCPUName(u8buff))
		{
			sb.Append(u8buff);
		}
		else
		{
			sb.Append((const UTF8Char*)"-");
		}
		writer->WriteLine(sb.ToString());

		Data::ArrayList<IO::SystemInfo::RAMInfo*> ramList;
		IO::SystemInfo::RAMInfo *ram;
		sysInfo.GetRAMInfo(&ramList);
		i = 0;
		j = ramList.GetCount();
		while (i < j)
		{
			ram = ramList.GetItem(i);
			sb.ClearStr();
			sb.Append((const UTF8Char*)"RAM: ");
			if (ram->deviceLocator)
			{
				sb.Append(ram->deviceLocator);
			}
			sb.Append((const UTF8Char*)"\t");
			if (ram->manufacturer)
			{
				sb.Append(ram->manufacturer);
			}
			sb.Append((const UTF8Char*)"\t");
			if (ram->partNo)
			{
				sb.Append(ram->partNo);
			}
			sb.Append((const UTF8Char*)"\t");
			if (ram->sn)
			{
				sb.Append(ram->sn);
			}
			sb.Append((const UTF8Char*)"\t");
			sb.AppendUOSInt(ram->defSpdMHz);
			sb.Append((const UTF8Char*)"\t");
			sb.AppendUOSInt(ram->confSpdMHz);
			sb.Append((const UTF8Char*)"\t");
			sb.AppendU32(ram->dataWidth);
			sb.Append((const UTF8Char*)"\t");
			sb.AppendU32(ram->totalWidth);
			sb.Append((const UTF8Char*)"\t");
			sb.AppendU64(ram->memorySize);
			writer->WriteLine(sb.ToString());
			i++;
		}
		sysInfo.FreeRAMInfo(&ramList);

		writer->WriteLine((const UTF8Char*)"Result:");
		i = 0;
		j = me->resultList->GetCount();
		while (i < j)
		{
			result = me->resultList->GetItem(i);
			sb.ClearStr();
			switch (result->tt)
			{
			case TT_COPY:
				sb.Append((const UTF8Char*)"Copy");
				break;
			case TT_WRITE:
				sb.Append((const UTF8Char*)"Write");
				break;
			case TT_READ:
				sb.Append((const UTF8Char*)"Read");
				break;
			default:
				sb.Append((const UTF8Char*)"Unknown");
				break;
			}
			sb.Append((const UTF8Char*)"\t");
			sb.AppendUOSInt(result->testSize);
			sb.Append((const UTF8Char*)"\t");
			Text::SBAppendF64(&sb, result->resultRate);
			writer->WriteLine(sb.ToString());
			i++;
		}
		DEL_CLASS(writer);
		DEL_CLASS(fs);
	}
	DEL_CLASS(dlg);
}

SSWR::AVIRead::AVIRBenchmarkForm::AVIRBenchmarkForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core) : UI::GUIForm(parent, 640, 480, ui)
{
	this->SetText((const UTF8Char*)"Benchmark");
	this->SetFont(0, 8.25, false);

	this->core = core;
	this->processing = false;
	NEW_CLASS(this->resultList, Data::ArrayList<SSWR::AVIRead::AVIRBenchmarkForm::TestResult*>());
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));

	NEW_CLASS(this->pnlCtrl, UI::GUIPanel(ui, this));
	this->pnlCtrl->SetRect(0, 0, 100, 32, false);
	this->pnlCtrl->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->btnStart, UI::GUIButton(ui, this->pnlCtrl, (const UTF8Char*)"Start"));
	this->btnStart->SetRect(4, 4, 75, 23, false);
	this->btnStart->HandleButtonClick(OnStartClicked, this);
	NEW_CLASS(this->btnQuick, UI::GUIButton(ui, this->pnlCtrl, (const UTF8Char*)"Quick"));
	this->btnQuick->SetRect(84, 4, 75, 23, false);
	this->btnQuick->HandleButtonClick(OnQuickClicked, this);
	NEW_CLASS(this->btnSave, UI::GUIButton(ui, this->pnlCtrl, (const UTF8Char*)"Save"));
	this->btnSave->SetRect(164, 4, 75, 23, false);
	this->btnSave->HandleButtonClick(OnSaveClicked, this);
	NEW_CLASS(this->pbMain, UI::GUIProgressBar(ui, this, 100));
	this->pbMain->SetRect(0, 0, 100, 24, false);
	this->pbMain->SetDockType(UI::GUIControl::DOCK_BOTTOM);
	NEW_CLASS(this->tcMain, UI::GUITabControl(ui, this));
	this->tcMain->SetDockType(UI::GUIControl::DOCK_FILL);

	this->tpCopy = this->tcMain->AddTabPage((const UTF8Char*)"Copy");
	NEW_CLASS(this->lvCopy, UI::GUIListView(ui, this->tpCopy, UI::GUIListView::LVSTYLE_TABLE, 2));
	this->lvCopy->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvCopy->SetShowGrid(true);
	this->lvCopy->SetFullRowSelect(true);
	this->lvCopy->AddColumn((const UTF8Char*)"Buffer Size", 200);
	this->lvCopy->AddColumn((const UTF8Char*)"Data Rate", 150);

	this->tpRead = this->tcMain->AddTabPage((const UTF8Char*)"Read");
	NEW_CLASS(this->lvRead, UI::GUIListView(ui, this->tpRead, UI::GUIListView::LVSTYLE_TABLE, 2));
	this->lvRead->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvRead->SetShowGrid(true);
	this->lvRead->SetFullRowSelect(true);
	this->lvRead->AddColumn((const UTF8Char*)"Buffer Size", 200);
	this->lvRead->AddColumn((const UTF8Char*)"Data Rate", 150);

	this->tpWrite = this->tcMain->AddTabPage((const UTF8Char*)"Write");
	NEW_CLASS(this->lvWrite, UI::GUIListView(ui, this->tpWrite, UI::GUIListView::LVSTYLE_TABLE, 2));
	this->lvWrite->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvWrite->SetShowGrid(true);
	this->lvWrite->SetFullRowSelect(true);
	this->lvWrite->AddColumn((const UTF8Char*)"Buffer Size", 200);
	this->lvWrite->AddColumn((const UTF8Char*)"Data Rate", 150);

	this->tpInfo = this->tcMain->AddTabPage((const UTF8Char*)"Info");
	NEW_CLASS(this->pnlInfo, UI::GUIPanel(ui, this->tpInfo));
	this->pnlInfo->SetRect(0, 0, 100, 52, false);
	this->pnlInfo->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblPlatform, UI::GUILabel(ui, this->pnlInfo, (const UTF8Char*)"Platform"));
	this->lblPlatform->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtPlatform, UI::GUITextBox(ui, this->pnlInfo, (const UTF8Char*)""));
	this->txtPlatform->SetRect(104, 4, 300, 23, false);
	this->txtPlatform->SetReadOnly(true);
	NEW_CLASS(this->lblCPU, UI::GUILabel(ui, this->pnlInfo, (const UTF8Char*)"CPU"));
	this->lblCPU->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->txtCPU, UI::GUITextBox(ui, this->pnlInfo, (const UTF8Char*)""));
	this->txtCPU->SetRect(104, 28, 300, 23, false);
	this->txtCPU->SetReadOnly(true);
	NEW_CLASS(this->lvRAM, UI::GUIListView(ui, this->tpInfo, UI::GUIListView::LVSTYLE_TABLE, 9));
	this->lvRAM->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvRAM->SetFullRowSelect(true);
	this->lvRAM->SetShowGrid(true);
	this->lvRAM->AddColumn((const UTF8Char*)"Locator", 100);
	this->lvRAM->AddColumn((const UTF8Char*)"Manufacturer", 60);
	this->lvRAM->AddColumn((const UTF8Char*)"PartNo", 140);
	this->lvRAM->AddColumn((const UTF8Char*)"SN", 80);
	this->lvRAM->AddColumn((const UTF8Char*)"Def Speed", 40);
	this->lvRAM->AddColumn((const UTF8Char*)"Conf Speed", 40);
	this->lvRAM->AddColumn((const UTF8Char*)"Data Width", 40);
	this->lvRAM->AddColumn((const UTF8Char*)"Total Width", 40);
	this->lvRAM->AddColumn((const UTF8Char*)"Memory Size", 80);

	UTF8Char sbuff[128];
	IO::SystemInfo sysInfo;
	if (sysInfo.GetPlatformName(sbuff))
	{
		this->txtPlatform->SetText(sbuff);
	}
	else
	{
		this->txtPlatform->SetText((const UTF8Char*)"-");
	}
	Manage::CPUInfo cpu;
	if (cpu.GetCPUName(sbuff))
	{
		this->txtCPU->SetText(sbuff);
	}
	else
	{
		this->txtCPU->SetText((const UTF8Char*)"-");
	}
	Data::ArrayList<IO::SystemInfo::RAMInfo*> ramList;
	IO::SystemInfo::RAMInfo *ram;
	sysInfo.GetRAMInfo(&ramList);
	UOSInt i;
	UOSInt j;
	UOSInt k;
	i = 0;
	j = ramList.GetCount();
	while (i < j)
	{
		ram = ramList.GetItem(i);
		k = this->lvRAM->AddItem(ram->deviceLocator, 0);
		this->lvRAM->SetSubItem(k, 1, ram->manufacturer);
		this->lvRAM->SetSubItem(k, 2, ram->partNo);
		this->lvRAM->SetSubItem(k, 3, ram->sn);
		Text::StrUOSInt(sbuff, ram->defSpdMHz);
		this->lvRAM->SetSubItem(k, 4, sbuff);
		Text::StrUOSInt(sbuff, ram->confSpdMHz);
		this->lvRAM->SetSubItem(k, 5, sbuff);
		Text::StrUInt32(sbuff, ram->dataWidth);
		this->lvRAM->SetSubItem(k, 6, sbuff);
		Text::StrUInt32(sbuff, ram->totalWidth);
		this->lvRAM->SetSubItem(k, 7, sbuff);
		if (ram->memorySize == 0)
		{
			Text::StrUInt64(sbuff, ram->memorySize);
		}
		else if (ram->memorySize >= 1073741824)
		{
			Text::StrConcat(Text::StrUInt64(sbuff, ram->memorySize / 1073741824), (const UTF8Char*)"GB");
		}
		else if (ram->memorySize >= 1048576)
		{
			Text::StrConcat(Text::StrUInt64(sbuff, ram->memorySize / 1073741824), (const UTF8Char*)"MB");
		}
		else if (ram->memorySize >= 1024)
		{
			Text::StrConcat(Text::StrUInt64(sbuff, ram->memorySize / 1073741824), (const UTF8Char*)"KB");
		}
		this->lvRAM->SetSubItem(k, 8, sbuff);
		i++;
	}
	sysInfo.FreeRAMInfo(&ramList);
}

SSWR::AVIRead::AVIRBenchmarkForm::~AVIRBenchmarkForm()
{
	this->ClearResult();
	DEL_CLASS(this->resultList);
}

void SSWR::AVIRead::AVIRBenchmarkForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}
