#include "Stdafx.h"
#include "IO/FileStream.h"
#include "Net/UserAgentDB.h"
#include "Net/WebSite/WebSite48IdolControl.h"
#include "SSWR/DownloadMonitor/DownMonMainForm.h"
#include "Sync/MutexUsage.h"
#include "Text/UTF8Reader.h"
#include "Text/UTF8Util.h"
#include "Text/UTF8Writer.h"
#include "UI/MessageDialog.h"
#include "Win32/Clipboard.h"
#include <stdio.h>

void __stdcall SSWR::DownloadMonitor::DownMonMainForm::OnTimerTick(void *userObj)
{
	SSWR::DownloadMonitor::DownMonMainForm *me = (SSWR::DownloadMonitor::DownMonMainForm*)userObj;
	SSWR::DownloadMonitor::DownMonCore::CheckStatus status = me->core->GetCurrStatus();
	if (status != me->currStatus)
	{
		me->currStatus = status;
		switch (status)
		{
		case SSWR::DownloadMonitor::DownMonCore::CS_IDLE:
			me->txtStatus->SetText((const UTF8Char*)"Idle");
			break;
		case SSWR::DownloadMonitor::DownMonCore::CS_DOWNLOADING:
			me->txtStatus->SetText((const UTF8Char*)"Downloading");
			break;
		case SSWR::DownloadMonitor::DownMonCore::CS_CHECKING:
			me->txtStatus->SetText((const UTF8Char*)"Checking");
			break;
		case SSWR::DownloadMonitor::DownMonCore::CS_EXTRACTING:
			me->txtStatus->SetText((const UTF8Char*)"Extracting");
			break;
		case SSWR::DownloadMonitor::DownMonCore::CS_VALIDATING:
			me->txtStatus->SetText((const UTF8Char*)"Validating");
			break;
		case SSWR::DownloadMonitor::DownMonCore::CS_MOVING:
			me->txtStatus->SetText((const UTF8Char*)"Moving");
			break;
		case SSWR::DownloadMonitor::DownMonCore::CS_MUXING:
			me->txtStatus->SetText((const UTF8Char*)"Muxing");
			break;
		default:
			break;
		}
	}

	Bool changed = false;
	UOSInt i;
	UOSInt j;
	Int32 id;
	Sync::MutexUsage mutUsage(me->endedMut);
	while (me->endedList->GetCount() > 0)
	{
		id = me->endedList->RemoveAt(0);
		i = 0;
		j = me->lvFiles->GetCount();
		while (i < j)
		{
			if (id == (OSInt)me->lvFiles->GetItem(i))
			{
				me->lvFiles->RemoveItem(i);
				changed = true;
				break;
			}
			i++;
		}
	}
	mutUsage.EndUse();
	if (changed)
	{
		me->SaveList();
		if (me->chkAutoStart->IsChecked())
		{
			id = (Int32)(OSInt)me->lvFiles->GetItem(0);
			if (id)
			{
				me->core->FileStart(id & 0xffffff, id >> 24, me->GetHandle());
			}
		}
	}

	if (me->alarmSet)
	{
		Data::DateTime dt;
		dt.SetCurrTimeUTC();
		if (dt.ToTicks() >= me->alarmTime)
		{
			me->alarmSet = false;
			me->lblAlarm->SetText((const UTF8Char*)"");
			id = (Int32)(OSInt)me->lvFiles->GetItem(0);
			if (id)
			{
				me->core->FileStart(id & 0xffffff, id >> 24, me->GetHandle());
			}
		}
	}
}

void __stdcall SSWR::DownloadMonitor::DownMonMainForm::OnPasteTableClicked(void *userObj)
{
	SSWR::DownloadMonitor::DownMonMainForm *me = (SSWR::DownloadMonitor::DownMonMainForm*)userObj;
	Text::StringBuilderUTF8 sb;
	Win32::Clipboard::GetString(me->GetHandle(), &sb);
	UTF8Char sbuff[32];
	UTF8Char *sarr[2];
	UTF8Char *sarr2[2];
	Bool changed = false;
	UOSInt i;
	UOSInt j;
	sarr[1] = sb.ToString();
	while (true)
	{
		i = Text::StrSplitLine(sarr, 2, sarr[1]);
		if (Text::StrStartsWith(sarr[0], (const UTF8Char*)"https://") && Text::StrSplit(sarr2, 2, sarr[0], '\t') == 2)
		{
			Int32 id = 0;
			Int32 webType = 0;
			id = ParseURL(sarr[0], &webType);

			if (id != 0)
			{
				if (me->core->FileAdd(id, webType, sarr2[1]))
				{
					Sync::MutexUsage mutUsage;
					SSWR::DownloadMonitor::DownMonCore::FileInfo *file = me->core->FileGet(id, webType, &mutUsage);
					if (file != 0)
					{
						Text::StrInt32(sbuff, file->id);
						j = me->lvFiles->AddItem(sbuff, (void*)(OSInt)((file->webType << 24) | file->id));
						me->lvFiles->SetSubItem(j, 1, file->fileName);
						changed = true;
					}
				}
			}
		}
		if (i != 2)
			break;
	}

	if (changed)
	{
		me->SaveList();
	}
}

void __stdcall SSWR::DownloadMonitor::DownMonMainForm::OnPasteHTMLClicked(void *userObj)
{
	SSWR::DownloadMonitor::DownMonMainForm *me = (SSWR::DownloadMonitor::DownMonMainForm*)userObj;
	Data::ArrayList<UInt32> formats;
	UOSInt i;
	UOSInt j;
	UInt32 fmtId = (UInt32)-1;
	UTF8Char sbuff[512];
	UTF8Char *sarr[2];
	Data::ArrayList<const UTF8Char *> urlList;
	Data::ArrayList<const UTF8Char *> descList;
	Win32::Clipboard clipboard(me->GetHandle());
	clipboard.GetDataFormats(&formats);
	i = formats.GetCount();
	while (i-- > 0)
	{
		Win32::Clipboard::GetFormatName(formats.GetItem(i), sbuff, 256);
		if (Text::StrEquals(sbuff, (const UTF8Char*)"text/html"))
		{
			fmtId = formats.GetItem(i);
			break;
		}
	}

	if (fmtId != (UInt32)-1)
	{
		const UTF8Char *desc;
		const UTF8Char *url;
		Text::StringBuilderUTF8 sb;
		if (clipboard.GetDataText(fmtId, &sb))
		{
			sarr[1] = sb.ToString();
			while (true)
			{
				i = Text::StrSplitLine(sarr, 2, sarr[1]);
				if (i == 2 && Text::StrIndexOf(sarr[0], (const UTF8Char*)"<div class=\"post-thumb\">") != INVALID_INDEX)
				{
					desc = 0;
					i = Text::StrSplitLine(sarr, 2, sarr[1]);
					j = Text::StrIndexOf(sarr[0], (const UTF8Char*)"<img class=\"lazyload\" ");
					if (i == 2 && j != INVALID_INDEX)
					{
						sarr[0] = &sarr[0][j + 22];
						j = Text::StrIndexOf(sarr[0], (const UTF8Char*)"alt=\"");
						if (j != INVALID_INDEX)
						{
							sarr[0] = &sarr[0][j + 5];
							j = Text::StrIndexOf(sarr[0], '\"');
							if (j != INVALID_INDEX)
							{
								sarr[0][j] = 0;
								desc = sarr[0];
							}
						}
					}
					if (desc)
					{
						i = Text::StrSplitLine(sarr, 2, sarr[1]);
						j = Text::StrIndexOf(sarr[0], (const UTF8Char*)"<a href=\"");
						if (i == 2 && j != INVALID_INDEX)
						{
							sarr[0] = &sarr[0][j + 9];
							j = Text::StrIndexOf(sarr[0], '\"');
							if (j != INVALID_INDEX)
							{
								sarr[0][j] = 0;
								urlList.Add(sarr[0]);
								descList.Add(desc);
							}
						}
					}
				}
				if (i != 2)
				{
					break;
				}
			}

			if (urlList.GetCount() > 0)
			{
				Bool changed = false;
				Int32 id;
				Int32 webType;
				i = urlList.GetCount();
				while (i-- > 0)
				{
					url = urlList.GetItem(i);
					desc = descList.GetItem(i);

					id = 0;
					webType = 0;
					id = ParseURL(url, &webType);

					if (id != 0)
					{
						if (me->core->FileAdd(id, webType, desc))
						{
							Sync::MutexUsage mutUsage;
							SSWR::DownloadMonitor::DownMonCore::FileInfo *file = me->core->FileGet(id, 3, &mutUsage);
							if (file != 0)
							{
								Text::StrInt32(sbuff, file->id);
								j = me->lvFiles->AddItem(sbuff, (void*)(OSInt)((file->webType << 24) | file->id));
								me->lvFiles->SetSubItem(j, 1, file->fileName);
								changed = true;
							}
						}
					}
				}

				if (changed)
				{
					me->SaveList();
				}
			}
		}
	}
}

void __stdcall SSWR::DownloadMonitor::DownMonMainForm::OnCopyTableClicked(void *userObj)
{
	SSWR::DownloadMonitor::DownMonMainForm *me = (SSWR::DownloadMonitor::DownMonMainForm*)userObj;
	Text::StringBuilderUTF8 sb;
	UOSInt i;
	UOSInt j;
	i = 0;
	j = me->lvFiles->GetCount();
	while (i < j)
	{
		Int32 id = (Int32)(OSInt)me->lvFiles->GetItem(i);
		Sync::MutexUsage mutUsage;
		SSWR::DownloadMonitor::DownMonCore::FileInfo *file = me->core->FileGet(id & 0xffffff, id >> 24, &mutUsage);
		if (file != 0)
		{
			if (file->webType == 2)
			{
				sb.Append((const UTF8Char*)"https://48idol.net/video/");
			}
			else if (file->webType == 1)
			{
				sb.Append((const UTF8Char*)"https://48idol.com/video/");
			}
			else if (file->webType == 3)
			{
				sb.Append((const UTF8Char*)"https://48idol.tv/archive/video/");
			}
			else
			{
				sb.Append((const UTF8Char*)"https://48idol.tv/video/");
			}
			sb.AppendI32(file->id);
			sb.AppendChar('\t', 1);
			sb.Append(file->dbName);
			sb.Append((const UTF8Char*)"\r\n");
		}
		i++;
	}

	Win32::Clipboard::SetString(me->GetHandle(), sb.ToString());
}

void __stdcall SSWR::DownloadMonitor::DownMonMainForm::OnFilesDblClick(void *userObj, UOSInt itemIndex)
{
	SSWR::DownloadMonitor::DownMonMainForm *me = (SSWR::DownloadMonitor::DownMonMainForm*)userObj;
	Int32 id = (Int32)(OSInt)me->lvFiles->GetItem(itemIndex);
	if (id)
	{
		me->core->FileStart(id & 0xffffff, id >> 24, me->GetHandle());
	}
}

void __stdcall SSWR::DownloadMonitor::DownMonMainForm::OnFileEndClicked(void *userObj)
{
	SSWR::DownloadMonitor::DownMonMainForm *me = (SSWR::DownloadMonitor::DownMonMainForm*)userObj;
	Int32 id = (Int32)(OSInt)me->lvFiles->GetSelectedItem();
	if (id > 0)
	{
		if (UI::MessageDialog::ShowYesNoDialog((const UTF8Char*)"Are you sure to remove selected file?", (const UTF8Char*)"Question", me))
		{
			me->core->FileEnd(id & 0xffffff, id >> 24);
			Sync::MutexUsage mutUsage(me->endedMut);
			me->endedList->Add(id);
			mutUsage.EndUse();
		}
	}
}

void __stdcall SSWR::DownloadMonitor::DownMonMainForm::OnWebUpdateClicked(void *userObj)
{
	SSWR::DownloadMonitor::DownMonMainForm *me = (SSWR::DownloadMonitor::DownMonMainForm*)userObj;
	Int32 webType = 4;
	Int32 maxId = me->core->FileGetMaxId(webType);
	OSInt currPage = 1;
	UTF8Char sbuff[32];
	UOSInt i;
	Data::ArrayList<Net::WebSite::WebSite48IdolControl::ItemData*> pageList;
	Data::ArrayList<Net::WebSite::WebSite48IdolControl::ItemData*> totalList;
	Net::WebSite::WebSite48IdolControl::ItemData *item;
	Net::WebSite::WebSite48IdolControl *ctrl;
	Text::EncodingFactory *encFact;
	const UTF8Char *userAgent = Net::UserAgentDB::FindUserAgent(Manage::OSInfo::OT_WINDOWS_NT64, Net::BrowserInfo::BT_FIREFOX);
	NEW_CLASS(encFact, Text::EncodingFactory());
	NEW_CLASS(ctrl, Net::WebSite::WebSite48IdolControl(me->core->GetSocketFactory(), me->core->GetSSLEngine(), encFact, userAgent));
	while (true)
	{
		ctrl->GetTVPageItems(currPage, &pageList);
		i = pageList.GetCount();
		printf("Page %d get, %d items found\r\n", (Int32)currPage, (Int32)i);
		if (i <= 0)
		{
			break;
		}
		while (pageList.GetCount() > 0)
		{
			item = pageList.GetItem(0);
			if (item->id <= maxId)
			{
				break;
			}
			totalList.Add(pageList.RemoveAt(0));
		}
		if (maxId == 0 || pageList.GetCount() > 0)
		{
			ctrl->FreeItems(&pageList);
			break;
		}
		currPage++;
	}
	i = totalList.GetCount();
	if (i > 0)
	{
		Bool changed = false;
		while (i-- > 0)
		{
			item = totalList.GetItem(i);
			if (me->core->FileAdd(item->id, webType, item->title))
			{
				Sync::MutexUsage mutUsage;
				SSWR::DownloadMonitor::DownMonCore::FileInfo *file = me->core->FileGet(item->id, webType, &mutUsage);
				UOSInt j;
				if (file != 0)
				{
					Text::StrInt32(sbuff, file->id);
					j = me->lvFiles->AddItem(sbuff, (void*)(OSInt)((file->webType << 24) | file->id));
					me->lvFiles->SetSubItem(j, 1, file->fileName);
					changed = true;
				}
			}
		}
		ctrl->FreeItems(&totalList);
		if (changed)
		{
			me->SaveList();
		}
	}

	DEL_CLASS(ctrl);
	DEL_CLASS(encFact);
}

void __stdcall SSWR::DownloadMonitor::DownMonMainForm::On30MinutesClicked(void *userObj)
{
	SSWR::DownloadMonitor::DownMonMainForm *me = (SSWR::DownloadMonitor::DownMonMainForm*)userObj;
	UTF8Char sbuff[64];
	Data::DateTime dt;
	dt.SetCurrTime();
	dt.AddMinute(30);
	me->alarmTime = dt.ToTicks();
	me->alarmSet = true;
	dt.ToString(sbuff, "HH:mm:ss.fff");
	me->lblAlarm->SetText(sbuff);
}

void __stdcall SSWR::DownloadMonitor::DownMonMainForm::OnFileEnd(void *userObj, Int32 fileId, Int32 webType)
{
	SSWR::DownloadMonitor::DownMonMainForm *me = (SSWR::DownloadMonitor::DownMonMainForm*)userObj;
	Sync::MutexUsage mutUsage(me->endedMut);
	me->endedList->Add((webType << 24) | fileId);
	mutUsage.EndUse();
}

Int32 SSWR::DownloadMonitor::DownMonMainForm::ParseURL(const UTF8Char *url, Int32 *webType)
{
	Int32 id;
	if (Text::StrStartsWith(url, (const UTF8Char*)"https://48idol.com/video/"))
	{
		id = Text::StrToInt32(&url[25]);
		*webType = 1;
	}
	else if (Text::StrStartsWith(url, (const UTF8Char*)"https://48idol.net/video/"))
	{
		id = Text::StrToInt32(&url[25]);
		*webType = 2;
	}
	else if (Text::StrStartsWith(url, (const UTF8Char*)"https://48idol.tv/archive/video/"))
	{
		id = Text::StrToInt32(&url[32]);
		*webType = 3;
	}
	else if (Text::StrStartsWith(url, (const UTF8Char*)"https://48idol.tv/video/"))
	{
		id = Text::StrToInt32(&url[24]);
		*webType = 4;
	}
	else
	{
		id = 0;
		*webType = 0;
	}
	return id;
}

void SSWR::DownloadMonitor::DownMonMainForm::LoadList()
{
	IO::FileStream *fs;
	Text::UTF8Reader *reader;
	Text::StringBuilderUTF8 sb;

	Net::WebSite::WebSite48IdolControl *ctrl = 0;
	Text::EncodingFactory *encFact = 0;
	const UTF8Char *userAgent = Net::UserAgentDB::FindUserAgent(Manage::OSInfo::OT_WINDOWS_NT64, Net::BrowserInfo::BT_FIREFOX);
	Text::StringBuilderUTF8 sb2;
	Bool updated = false;

	UTF8Char sbuff[32];
	UTF8Char *sarr[2];
	UOSInt i;
	NEW_CLASS(fs, IO::FileStream(this->core->GetListFile(), IO::FileStream::FILE_MODE_READONLY, IO::FileStream::FILE_SHARE_DENY_NONE, IO::FileStream::BT_NORMAL));
	NEW_CLASS(reader, Text::UTF8Reader(fs));
	while (reader->ReadLine(&sb, 4096))
	{
		if (Text::StrStartsWith(sb.ToString(), (const UTF8Char*)"https://") && Text::StrSplit(sarr, 2, sb.ToString(), '\t') == 2)
		{
			Int32 id = 0;
			Int32 webType = 0;
			id = ParseURL(sarr[0], &webType);

			if (id != 0)
			{
				if (!Text::UTF8Util::ValidStr(sarr[1]))
				{
					printf("Invalid char found, id = %d\r\n", id);
					if (ctrl == 0)
					{
						NEW_CLASS(encFact, Text::EncodingFactory());
						NEW_CLASS(ctrl, Net::WebSite::WebSite48IdolControl(this->core->GetSocketFactory(), this->core->GetSSLEngine(), encFact, userAgent));
					}
					sb2.ClearStr();
					if (ctrl->GetVideoName(id, &sb2))
					{
						printf("Name of id %d updated\r\n", id);
						sarr[1] = sb2.ToString();
						updated = true;
					}
				}
				if (this->core->FileAdd(id, webType, sarr[1]))
				{
					Sync::MutexUsage mutUsage;
					SSWR::DownloadMonitor::DownMonCore::FileInfo *file = this->core->FileGet(id, webType, &mutUsage);
					if (file != 0)
					{
						Text::StrInt32(sbuff, file->id);
						i = this->lvFiles->AddItem(sbuff, (void*)(OSInt)((file->webType << 24) | file->id));
						this->lvFiles->SetSubItem(i, 1, file->fileName);
					}
				}
			}
		}

		sb.ClearStr();
	}
	DEL_CLASS(reader);
	DEL_CLASS(fs);
	SDEL_CLASS(ctrl);
	SDEL_CLASS(encFact);
	
	if (updated)
	{
		this->SaveList();
	}
}

void SSWR::DownloadMonitor::DownMonMainForm::SaveList()
{
	IO::FileStream *fs;
	Text::UTF8Writer *writer;
	Text::StringBuilderUTF8 sb;
	UOSInt i;
	UOSInt j;

	NEW_CLASS(fs, IO::FileStream(this->core->GetListFile(), IO::FileStream::FILE_MODE_CREATE, IO::FileStream::FILE_SHARE_DENY_NONE, IO::FileStream::BT_NORMAL));
	NEW_CLASS(writer, Text::UTF8Writer(fs));
	writer->WriteSignature();
	i = 0;
	j = this->lvFiles->GetCount();
	while (i < j)
	{
		Int32 id = (Int32)(OSInt)this->lvFiles->GetItem(i);
		Sync::MutexUsage mutUsage;
		SSWR::DownloadMonitor::DownMonCore::FileInfo *file = this->core->FileGet(id & 0xffffff, id >> 24, &mutUsage);
		if (file != 0)
		{
			sb.ClearStr();
			if (file->webType == 2)
			{
				sb.Append((const UTF8Char*)"https://48idol.net/video/");
			}
			else if (file->webType == 1)
			{
				sb.Append((const UTF8Char*)"https://48idol.com/video/");
			}
			else if (file->webType == 3)
			{
				sb.Append((const UTF8Char*)"https://48idol.tv/archive/video/");
			}
			else
			{
				sb.Append((const UTF8Char*)"https://48idol.tv/video/");
			}
			sb.AppendI32(file->id);
			sb.AppendChar('\t', 1);
			sb.Append(file->dbName);
			writer->WriteLine(sb.ToString());
		}
		i++;
	}
	DEL_CLASS(writer);
	DEL_CLASS(fs);
}

SSWR::DownloadMonitor::DownMonMainForm::DownMonMainForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::DownloadMonitor::DownMonCore *core) : UI::GUIForm(parent, 1024, 768, ui)
{
	this->core = core;
	this->SetText((const UTF8Char*)"Download Monitor");
	this->SetFont(0, 8.25, false);
	this->alarmSet = false;
	this->alarmTime = 0;
	NEW_CLASS(this->endedMut, Sync::Mutex());
	NEW_CLASS(this->endedList, Data::ArrayList<Int32>());
	this->core->SetFileEndHandler(OnFileEnd, this);

	NEW_CLASS(this->pnlButtons, UI::GUIPanel(ui, this));
	this->pnlButtons->SetRect(0, 0, 100, 55, false);
	this->pnlButtons->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblStatus, UI::GUILabel(ui, this->pnlButtons, (const UTF8Char*)"Status"));
	this->lblStatus->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtStatus, UI::GUITextBox(ui, this->pnlButtons, (const UTF8Char*)"Idle"));
	this->txtStatus->SetRect(104, 4, 100, 23, false);
	this->txtStatus->SetReadOnly(true);
	NEW_CLASS(this->chkAutoStart, UI::GUICheckBox(ui, this->pnlButtons, (const UTF8Char*)"AutoStart", false));
	this->chkAutoStart->SetRect(204, 4, 100, 23, false);
	NEW_CLASS(this->btnPasteTable, UI::GUIButton(ui, this->pnlButtons, (const UTF8Char*)"Paste Table"));
	this->btnPasteTable->SetRect(304, 4, 75, 23, false);
	this->btnPasteTable->HandleButtonClick(OnPasteTableClicked, this);
	NEW_CLASS(this->btnPasteHTML, UI::GUIButton(ui, this->pnlButtons, (const UTF8Char*)"Paste HTML"));
	this->btnPasteHTML->SetRect(384, 4, 75, 23, false);
	this->btnPasteHTML->HandleButtonClick(OnPasteHTMLClicked, this);
	NEW_CLASS(this->btnCopyTable, UI::GUIButton(ui, this->pnlButtons, (const UTF8Char*)"Copy Table"));
	this->btnCopyTable->SetRect(464, 4, 75, 23, false);
	this->btnCopyTable->HandleButtonClick(OnCopyTableClicked, this);
	NEW_CLASS(this->btnFileEnd, UI::GUIButton(ui, this->pnlButtons, (const UTF8Char*)"File End"));
	this->btnFileEnd->SetRect(544, 4, 75, 23, false);
	this->btnFileEnd->HandleButtonClick(OnFileEndClicked, this);
	NEW_CLASS(this->btnWebUpdate, UI::GUIButton(ui, this->pnlButtons, (const UTF8Char*)"Web Update"));
	this->btnWebUpdate->SetRect(624, 4, 75, 23, false);
	this->btnWebUpdate->HandleButtonClick(OnWebUpdateClicked, this);
	NEW_CLASS(this->lblAlarm, UI::GUILabel(ui, this->pnlButtons, (const UTF8Char*)""));
	this->lblAlarm->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->btn30Minutes, UI::GUIButton(ui, this->pnlButtons, (const UTF8Char*)"30 Minutes"));
	this->btn30Minutes->SetRect(104, 28, 75, 23, false);
	this->btn30Minutes->HandleButtonClick(On30MinutesClicked, this);

	NEW_CLASS(this->lvFiles, UI::GUIListView(ui, this, UI::GUIListView::LVSTYLE_TABLE, 3));
	this->lvFiles->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvFiles->AddColumn((const UTF8Char*)"Id", 60);
	this->lvFiles->AddColumn((const UTF8Char*)"File Name", 240);
	this->lvFiles->SetFullRowSelect(true);
	this->lvFiles->SetShowGrid(true);
	this->lvFiles->HandleDblClk(OnFilesDblClick, this);
//	this->lvFiles->HandleRightClick(OnFilesRClick, this);

	this->AddTimer(1000, OnTimerTick, this);
	this->LoadList();
}

SSWR::DownloadMonitor::DownMonMainForm::~DownMonMainForm()
{
	DEL_CLASS(this->endedList);
	DEL_CLASS(this->endedMut);
}
