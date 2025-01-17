#include "Stdafx.h"
#include "IO/FileStream.h"
#include "IO/Path.h"
#include "SSWR/AVIRead/AVIRLogExtractForm.h"
#include "Text/MyString.h"
#include "Text/UTF8Reader.h"
#include "Text/UTF8Writer.h"
#include "UI/FileDialog.h"
#include "UI/MessageDialog.h"

void __stdcall SSWR::AVIRead::AVIRLogExtractForm::OnSFileClicked(void *userObj)
{
	SSWR::AVIRead::AVIRLogExtractForm *me = (SSWR::AVIRead::AVIRLogExtractForm *)userObj;
	UI::FileDialog *dlg;
	Text::StringBuilderUTF8 sb;
	NEW_CLASS(dlg, UI::FileDialog(L"SSWR", L"AVIRead", L"LogExtractSFile", false));
	dlg->AddFilter((const UTF8Char*)"*.log", (const UTF8Char*)"Log File");
	dlg->SetAllowMultiSel(false);
	me->txtSFile->GetText(&sb);
	if (sb.GetLength() > 0)
	{
		dlg->SetFileName(sb.ToString());
	}
	if (dlg->ShowDialog(me))
	{
		me->txtSFile->SetText(dlg->GetFileName());
	}
	DEL_CLASS(dlg);
}

void __stdcall SSWR::AVIRead::AVIRLogExtractForm::OnOFileClicked(void *userObj)
{
	SSWR::AVIRead::AVIRLogExtractForm *me = (SSWR::AVIRead::AVIRLogExtractForm *)userObj;
	UI::FileDialog *dlg;
	Text::StringBuilderUTF8 sb;
	NEW_CLASS(dlg, UI::FileDialog(L"SSWR", L"AVIRead", L"LogExtractOFile", true));
	dlg->AddFilter((const UTF8Char*)"*.log", (const UTF8Char*)"Log File");
	dlg->AddFilter((const UTF8Char*)"*.txt", (const UTF8Char*)"Text File");
	me->txtOFile->GetText(&sb);
	if (sb.GetLength() > 0)
	{
		dlg->SetFileName(sb.ToString());
	}
	if (dlg->ShowDialog(me))
	{
		me->txtOFile->SetText(dlg->GetFileName());
	}
	DEL_CLASS(dlg);
}

void __stdcall SSWR::AVIRead::AVIRLogExtractForm::OnExtractClicked(void *userObj)
{
	SSWR::AVIRead::AVIRLogExtractForm *me = (SSWR::AVIRead::AVIRLogExtractForm *)userObj;
	UTF8Char sbuff[16];
	Text::StringBuilderUTF8 sb1;
	Text::StringBuilderUTF8 sb2;
	Text::StringBuilderUTF8 sb3;
	Text::StringBuilderUTF8 sbSuffix;
	me->txtSFile->GetText(&sb1);
	me->txtPrefix->GetText(&sb2);
	me->txtOFile->GetText(&sb3);
	me->txtSuffix->GetText(&sbSuffix);
	if (IO::Path::GetPathType(sb1.ToString()) != IO::Path::PT_FILE)
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Source File not found", (const UTF8Char*)"Error", me);
	}
	else if (sb2.GetLength() <= 0)
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Please enter Prefix", (const UTF8Char*)"Error", me);
	}
	else if (sb3.GetLength() <= 0)
	{
		UI::MessageDialog::ShowDialog((const UTF8Char*)"Please enter Output", (const UTF8Char*)"Error", me);
	}
	else
	{
		UOSInt typ = me->cboType->GetSelectedIndex();
		UOSInt i;
		Bool hasData;
		IO::FileStream *fs1;
		IO::FileStream *fs2;
		Text::UTF8Reader *reader;
		Text::UTF8Writer *writer;
		NEW_CLASS(fs1, IO::FileStream(sb1.ToString(), IO::FileStream::FILE_MODE_READONLY, IO::FileStream::FILE_SHARE_DENY_NONE, IO::FileStream::BT_NORMAL));
		NEW_CLASS(reader, Text::UTF8Reader(fs1));
		NEW_CLASS(fs2, IO::FileStream(sb3.ToString(), IO::FileStream::FILE_MODE_CREATE, IO::FileStream::FILE_SHARE_DENY_NONE, IO::FileStream::BT_NORMAL));
		if (fs2->IsError())
		{
			UI::MessageDialog::ShowDialog((const UTF8Char*)"Error in creating Output file", (const UTF8Char*)"Error", me);
		}
		else
		{
			NEW_CLASS(writer, Text::UTF8Writer(fs2));
			writer->WriteSignature();

			sb1.ClearStr();
			hasData = reader->ReadLine(&sb1, 1024);
			while (hasData)
			{
				reader->GetLastLineBreak(sbuff);
				while (hasData && sbuff[0] == 0)
				{
					hasData = reader->ReadLine(&sb1, 1024);
					reader->GetLastLineBreak(sbuff);
				}

				sb1.Append(sbSuffix.ToString());
				i = sb1.IndexOf('\t');
				if (i != INVALID_INDEX)
				{
					if (typ == 0)
					{
						if (Text::StrStartsWith(sb1.ToString() + i + 1, sb2.ToString()))
						{
							writer->WriteLine(sb1.ToString());
						}
					}
					else if (typ == 1)
					{
						if (Text::StrStartsWith(sb1.ToString() + i + 1, sb2.ToString()))
						{
							writer->WriteLine(sb1.ToString() + i + 1 + sb2.GetLength());
						}
					}
				}

				sb1.ClearStr();
				hasData = reader->ReadLine(&sb1, 1024);
			}

			UI::MessageDialog::ShowDialog((const UTF8Char*)"Complete log extract", (const UTF8Char*)"Success", me);
			DEL_CLASS(writer);
		}
		DEL_CLASS(fs2);
		DEL_CLASS(reader);
		DEL_CLASS(fs1);
	}
}

SSWR::AVIRead::AVIRLogExtractForm::AVIRLogExtractForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core) : UI::GUIForm(parent, 800, 184, ui)
{
	this->SetText((const UTF8Char*)"Log Extract");
	this->SetFont(0, 8.25, false);
	this->SetNoResize(true);
	this->core = core;
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));

	NEW_CLASS(this->lblSFile, UI::GUILabel(ui, this, (const UTF8Char*)"Source File"));
	this->lblSFile->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtSFile, UI::GUITextBox(ui, this, (const UTF8Char*)""));
	this->txtSFile->SetRect(104, 4, 560, 23, false);
	NEW_CLASS(this->btnSFile, UI::GUIButton(ui, this, (const UTF8Char*)"Browse"));
	this->btnSFile->SetRect(660, 4, 75, 23, false);
	this->btnSFile->HandleButtonClick(OnSFileClicked, this);
	NEW_CLASS(this->lblPrefix, UI::GUILabel(ui, this, (const UTF8Char*)"Prefix"));
	this->lblPrefix->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->txtPrefix, UI::GUITextBox(ui, this, (const UTF8Char*)""));
	this->txtPrefix->SetRect(104, 28, 560, 23, false);
	NEW_CLASS(this->lblOFile, UI::GUILabel(ui, this, (const UTF8Char*)"Output"));
	this->lblOFile->SetRect(4, 52, 100, 23, false);
	NEW_CLASS(this->txtOFile, UI::GUITextBox(ui, this, (const UTF8Char*)""));
	this->txtOFile->SetRect(104, 52, 560, 23, false);
	NEW_CLASS(this->btnOFile, UI::GUIButton(ui, this, (const UTF8Char*)"Browse"));
	this->btnOFile->SetRect(660, 52, 75, 23, false);
	this->btnOFile->HandleButtonClick(OnOFileClicked, this);
	NEW_CLASS(this->lblType, UI::GUILabel(ui, this, (const UTF8Char*)"Prefix"));
	this->lblType->SetRect(4, 76, 100, 23, false);
	NEW_CLASS(this->cboType, UI::GUIComboBox(ui, this, false));
	this->cboType->SetRect(104, 76, 200, 23, false);
	this->cboType->AddItem((const UTF8Char*)"Preserve line", 0);
	this->cboType->AddItem((const UTF8Char*)"No Date and prefix", 0);
	this->cboType->SetSelectedIndex(0);
	NEW_CLASS(this->lblSuffix, UI::GUILabel(ui, this, (const UTF8Char*)"Suffix to add"));
	this->lblSuffix->SetRect(4, 100, 100, 23, false);
	NEW_CLASS(this->txtSuffix, UI::GUITextBox(ui, this, (const UTF8Char*)""));
	this->txtSuffix->SetRect(104, 100, 560, 23, false);
	NEW_CLASS(this->btnExtract, UI::GUIButton(ui, this, (const UTF8Char*)"Extract"));
	this->btnExtract->SetRect(200, 124, 75, 23, false);
	this->btnExtract->HandleButtonClick(OnExtractClicked, this);
}

SSWR::AVIRead::AVIRLogExtractForm::~AVIRLogExtractForm()
{
}

void SSWR::AVIRead::AVIRLogExtractForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}
