#include "Stdafx.h"
#include "Exporter/MD4Exporter.h"
#include "Exporter/MD5Exporter.h"
#include "Exporter/SFVExporter.h"
#include "Exporter/SHA1Exporter.h"
#include "IO/FileStream.h"
#include "IO/Path.h"
#include "SSWR/AVIRead/AVIRFileHashForm.h"
#include "Sync/Thread.h"

void __stdcall SSWR::AVIRead::AVIRFileHashForm::OnFileDrop(void *userObj, const UTF8Char **files, OSInt nFiles)
{
	SSWR::AVIRead::AVIRFileHashForm *me = (SSWR::AVIRead::AVIRFileHashForm *)userObj;
	OSInt i;
	i = 0;
	while (i < nFiles)
	{
		me->AddFile(files[i]);
		i++;
	}
	me->fileListChg = true;
}

void __stdcall SSWR::AVIRead::AVIRFileHashForm::OnTimerTick(void *userObj)
{
	SSWR::AVIRead::AVIRFileHashForm *me = (SSWR::AVIRead::AVIRFileHashForm *)userObj;
	UTF8Char sbuff[32];
	me->UpdateUI();
	Data::DateTime currTime;
	currTime.SetCurrTimeUTC();
	me->readMut->Lock();
	if (me->progNameChg)
	{
		me->txtFileName->SetText(me->progName);
		me->progNameChg = false;
	}
	if (me->progLastCount != me->progCount)
	{
		me->prgFile->ProgressStart(me->progName, me->progCount);
		me->progLastCount = me->progCount;
	}

	Int64 currRead = me->readSize;
	Int64 currTotal = me->totalRead;
	me->readSize = 0;
	me->prgFile->ProgressUpdate(me->progCurr, me->progCount);
	me->readMut->Unlock();

	Int64 timeDiff = currTime.DiffMS(me->lastTimerTime);
	Double spd;
	if (timeDiff > 0)
	{
		Text::StrInt64(sbuff, currRead * 1000 / timeDiff);
		spd = currRead * 1000.0 / timeDiff;
	}
	else
	{
		Text::StrInt64(sbuff, currRead);
		spd = (Double)currRead;
	}
	me->txtSpeed->SetText(sbuff);
	me->rlcSpeed->AddSample(&spd);
	Text::StrInt64(sbuff, currTotal);
	me->txtTotalSize->SetText(sbuff);
	me->lastTimerTime->SetValue(&currTime);
}

void __stdcall SSWR::AVIRead::AVIRFileHashForm::OnCheckTypeChg(void *userObj)
{
	SSWR::AVIRead::AVIRFileHashForm *me = (SSWR::AVIRead::AVIRFileHashForm*)userObj;
	me->currHashType = (IO::FileCheck::CheckType)(OSInt)me->cboCheckType->GetSelectedItem();
}

UInt32 __stdcall SSWR::AVIRead::AVIRFileHashForm::HashThread(void *userObj)
{
	SSWR::AVIRead::AVIRFileHashForm *me = (SSWR::AVIRead::AVIRFileHashForm*)userObj;
	Bool found;
	FileStatus *status = 0;
	UTF8Char sbuff[512];
	OSInt i;
	OSInt j;
	IO::FileExporter *exporter;
	IO::FileCheck::CheckType chkType;
	me->threadStatus = 1;
	while (!me->threadToStop)
	{
		me->fileMut->Lock();
		found = false;
		i = 0;
		j = me->fileList->GetCount();
		while (i < j)
		{
			status = me->fileList->GetItem(i);
			if (status->status == 0)
			{
				found = true;
				status->status = 1;
				me->fileListChg = true;
				break;
			}
			i++;
		}
		me->fileMut->Unlock();
		if (found)
		{
			chkType = me->currHashType;
			if (chkType == IO::FileCheck::CT_MD5)
			{
				Text::StrConcat(Text::StrConcat(sbuff, status->fileName), (const UTF8Char*)".md5");
			}
			else if (chkType == IO::FileCheck::CT_CRC32)
			{
				Text::StrConcat(Text::StrConcat(sbuff, status->fileName), (const UTF8Char*)".sfv");
			}
			else if (chkType == IO::FileCheck::CT_SHA1)
			{
				Text::StrConcat(Text::StrConcat(sbuff, status->fileName), (const UTF8Char*)".sha1");
			}
			else if (chkType == IO::FileCheck::CT_MD4)
			{
				Text::StrConcat(Text::StrConcat(sbuff, status->fileName), (const UTF8Char*)".md4");
			}
			if (IO::Path::GetPathType(sbuff) == IO::Path::PT_UNKNOWN)
			{
				IO::FileCheck *fchk = IO::FileCheck::CreateCheck(status->fileName, chkType, me, false);
				IO::FileStream *fs;
				if (fchk)
				{
					if (chkType == IO::FileCheck::CT_CRC32)
					{
						NEW_CLASS(exporter, Exporter::SFVExporter());
					}
					else if (chkType == IO::FileCheck::CT_MD4)
					{
						NEW_CLASS(exporter, Exporter::MD4Exporter());
					}
					else if (chkType == IO::FileCheck::CT_MD5)
					{
						NEW_CLASS(exporter, Exporter::MD5Exporter());
					}
					else if (chkType == IO::FileCheck::CT_SHA1)
					{
						NEW_CLASS(exporter, Exporter::SHA1Exporter());
					}
					else
					{
						exporter = 0;
					}
					if (exporter)
					{
						exporter->SetCodePage(me->core->GetCurrCodePage());
						NEW_CLASS(fs, IO::FileStream(sbuff, IO::FileStream::FILE_MODE_CREATE, IO::FileStream::FILE_SHARE_DENY_NONE, IO::FileStream::BT_NORMAL));
						exporter->ExportFile(fs, sbuff, fchk, 0);
						DEL_CLASS(fs);
						DEL_CLASS(exporter);
					}
					DEL_CLASS(fchk);
					status->status = 2;
				}
				else
				{
					status->status = 3;
				}
			}
			else
			{
				status->status = 3;
			}
			me->fileListChg = true;
		}
		else
		{
			me->fileEvt->Wait(1000);
		}
	}
	me->threadStatus = 2;
	return 0;
}

void SSWR::AVIRead::AVIRFileHashForm::AddFile(const UTF8Char *fileName)
{
	FileStatus *status;
	this->fileMut->Lock();
	status = MemAlloc(FileStatus, 1);
	status->status = 0;
	status->fileName = Text::StrCopyNew(fileName);
	this->fileList->Add(status);
	this->fileMut->Unlock();
	this->fileEvt->Set();
}

void SSWR::AVIRead::AVIRFileHashForm::UpdateUI()
{
	if (this->fileListChg)
	{
		OSInt i;
		OSInt j;
		OSInt k;
		FileStatus *status;
		this->fileListChg = false;
		this->lvTasks->ClearItems();
		this->fileMut->Lock();
		i = 0;
		j = this->fileList->GetCount();
		while (i < j)
		{
			status = this->fileList->GetItem(i);
			k = this->lvTasks->AddItem(status->fileName, status);
			if (status->status == 0)
			{
				this->lvTasks->SetSubItem(k, 1, (const UTF8Char*)"Queued");
			}
			else if (status->status == 1)
			{
				this->lvTasks->SetSubItem(k, 1, (const UTF8Char*)"Processing");
			}
			else if (status->status == 2)
			{
				this->lvTasks->SetSubItem(k, 1, (const UTF8Char*)"Finished");
			}
			else if (status->status == 3)
			{
				this->lvTasks->SetSubItem(k, 1, (const UTF8Char*)"Error");
			}
			else
			{
				this->lvTasks->SetSubItem(k, 1, (const UTF8Char*)"Unknown");
			}
			i++;
		}
		this->fileMut->Unlock();
	}
}

SSWR::AVIRead::AVIRFileHashForm::AVIRFileHashForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core) : UI::GUIForm(parent, 1024, 768, ui)
{
	this->SetFont(0, 8.25, false);
	this->SetText((const UTF8Char*)"File Hash");

	this->core = core;
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
	NEW_CLASS(this->fileMut, Sync::Mutex());
	NEW_CLASS(this->fileList, Data::ArrayList<FileStatus*>());
	NEW_CLASS(this->readMut, Sync::Mutex());
	NEW_CLASS(this->fileEvt, Sync::Event(true, (const UTF8Char*)"SSWR.AVIRead.AVIRFileHashForm.fileEvt"));
	this->progName = 0;
	this->progNameChg = false;
	this->progLastCount = 0;
	this->progCurr = 0;
	this->progCount = 0;
	this->threadStatus = 0;
	this->threadToStop = false;
	this->currHashType = IO::FileCheck::CT_MD5;
	this->readSize = 0;
	this->totalRead = 0;
	NEW_CLASS(this->lastTimerTime, Data::DateTime());

	NEW_CLASS(this->pnlCheckType, UI::GUIPanel(ui, this));
	this->pnlCheckType->SetRect(0, 0, 100, 31, false);
	this->pnlCheckType->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblCheckType, UI::GUILabel(ui, this->pnlCheckType, (const UTF8Char*)"Hash Type"));
	this->lblCheckType->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->cboCheckType, UI::GUIComboBox(ui, this->pnlCheckType, false));
	this->cboCheckType->SetRect(104, 4, 100, 23, false);
	this->cboCheckType->AddItem((const UTF8Char*)"MD5", (void*)(OSInt)IO::FileCheck::CT_MD5);
	this->cboCheckType->AddItem((const UTF8Char*)"MD4", (void*)(OSInt)IO::FileCheck::CT_MD4);
	this->cboCheckType->AddItem((const UTF8Char*)"CRC", (void*)(OSInt)IO::FileCheck::CT_CRC32);
	this->cboCheckType->AddItem((const UTF8Char*)"SHA1", (void*)(OSInt)IO::FileCheck::CT_SHA1);
	this->cboCheckType->SetSelectedIndex(0);
	this->cboCheckType->HandleSelectionChange(OnCheckTypeChg, this);
	NEW_CLASS(this->tcMain, UI::GUITabControl(ui, this));
	this->tcMain->SetDockType(UI::GUIControl::DOCK_FILL);
	this->tpStatus = this->tcMain->AddTabPage((const UTF8Char*)"Status");
	this->tpTasks = this->tcMain->AddTabPage((const UTF8Char*)"Tasks");
	this->tpFiles = this->tcMain->AddTabPage((const UTF8Char*)"Files");

	NEW_CLASS(this->pnlStatus, UI::GUIPanel(ui, this->tpStatus));
	this->pnlStatus->SetRect(0, 0, 100, 72, false);
	this->pnlStatus->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->pnlStatusFile, UI::GUIPanel(ui, this->pnlStatus));
	this->pnlStatusFile->SetRect(0, 0, 100, 24, false);
	this->pnlStatusFile->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblFile, UI::GUILabel(ui, this->pnlStatusFile, (const UTF8Char*)"File"));
	this->lblFile->SetRect(0, 0, 56, 24, false);
	this->lblFile->SetDockType(UI::GUIControl::DOCK_LEFT);
	NEW_CLASS(this->txtFileName, UI::GUITextBox(ui, this->pnlStatusFile, (const UTF8Char*)""));
	this->txtFileName->SetReadOnly(true);
	this->txtFileName->SetDockType(UI::GUIControl::DOCK_FILL);
	NEW_CLASS(this->lblSpeed, UI::GUILabel(ui, this->pnlStatus, (const UTF8Char*)"Speed"));
	this->lblSpeed->SetRect(0, 24, 48, 23, false);
	NEW_CLASS(this->txtSpeed, UI::GUITextBox(ui, this->pnlStatus, (const UTF8Char*)""));
	this->txtSpeed->SetReadOnly(true);
	this->txtSpeed->SetRect(56, 24, 100, 23, false);
	NEW_CLASS(this->lblTotalSize, UI::GUILabel(ui, this->pnlStatus, (const UTF8Char*)"Total Size"));
	this->lblTotalSize->SetRect(184, 24, 80, 23, false);
	NEW_CLASS(this->txtTotalSize, UI::GUITextBox(ui, this->pnlStatus, (const UTF8Char*)""));
	this->txtTotalSize->SetRect(272, 24, 100, 23, false);
	this->txtTotalSize->SetReadOnly(true);
	NEW_CLASS(this->prgFile, UI::GUIProgressBar(ui, this->pnlStatus, 0));
	this->prgFile->SetRect(0, 0, 100, 24, false);
	this->prgFile->SetDockType(UI::GUIControl::DOCK_BOTTOM);
 	NEW_CLASS(this->rlcSpeed, UI::GUIRealtimeLineChart(ui, this->tpStatus, this->core->GetDrawEngine(), 1, 350, 500));
	this->rlcSpeed->SetDockType(UI::GUIControl::DOCK_FILL);

	NEW_CLASS(this->pnlTasks, UI::GUIPanel(ui, this->tpTasks));
	this->pnlTasks->SetRect(0, 0, 100, 32, false);
	this->pnlTasks->SetDockType(UI::GUIControl::DOCK_BOTTOM);
	NEW_CLASS(this->btnTasksClear, UI::GUIButton(ui, this->pnlTasks, (const UTF8Char*)"Clear Completed"));
	this->btnTasksClear->SetRect(4, 4, 120, 23, false);
	NEW_CLASS(this->lvTasks, UI::GUIListView(ui, this->tpTasks, UI::GUIListView::LVSTYLE_TABLE, 2));
	this->lvTasks->AddColumn((const UTF8Char*)"File Name", 500);
	this->lvTasks->AddColumn((const UTF8Char*)"Status", 100);
	this->lvTasks->SetShowGrid(true);
	this->lvTasks->SetFullRowSelect(true);
	this->lvTasks->SetDockType(UI::GUIControl::DOCK_FILL);

	NEW_CLASS(this->lvFiles, UI::GUIListView(ui, this->tpFiles, UI::GUIListView::LVSTYLE_TABLE, 2));
	this->lvFiles->AddColumn((const UTF8Char*)"File Name", 500);
	this->lvFiles->AddColumn((const UTF8Char*)"Hash", 200);
	this->lvFiles->SetShowGrid(true);
	this->lvFiles->SetFullRowSelect(true);
	this->lvFiles->SetDockType(UI::GUIControl::DOCK_FILL);

	this->HandleDropFiles(OnFileDrop, this);
	this->AddTimer(500, OnTimerTick, this);
	Sync::Thread::Create(HashThread, this);
}

SSWR::AVIRead::AVIRFileHashForm::~AVIRFileHashForm()
{
	OSInt i;
	FileStatus *status;
	this->threadToStop = true;
	this->fileEvt->Set();
	while (this->threadStatus != 2)
	{
		Sync::Thread::Sleep(10);
	}

	DEL_CLASS(this->lastTimerTime);
	DEL_CLASS(this->readMut);
	i = this->fileList->GetCount();
	while (i-- > 0)
	{
		status = this->fileList->GetItem(i);
		Text::StrDelNew(status->fileName);
		MemFree(status);
	}
	DEL_CLASS(this->fileEvt);
	DEL_CLASS(this->fileList);
	DEL_CLASS(this->fileMut);
}

void SSWR::AVIRead::AVIRFileHashForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}

void SSWR::AVIRead::AVIRFileHashForm::ProgressStart(const UTF8Char *name, Int64 count)
{
	this->readMut->Lock();
	if (this->progName)
	{
		Text::StrDelNew(this->progName);
	}
	this->progName = Text::StrCopyNew(name);
	this->progNameChg = true;
	this->readSize += this->progCount - this->progCurr;
	this->totalRead += this->progCount - this->progCurr;
	this->progCount = count;
	this->progCurr = 0;
	this->readMut->Unlock();
}

void SSWR::AVIRead::AVIRFileHashForm::ProgressUpdate(Int64 currCount, Int64 newCount)
{
	this->readMut->Lock();
	this->readSize += currCount - this->progCurr;
	this->totalRead += currCount - this->progCurr;
	this->progCurr = currCount;
	this->readMut->Unlock();
}

void SSWR::AVIRead::AVIRFileHashForm::ProgressEnd()
{
}