#include "Stdafx.h"
#include "IO/Path.h"
#include "Math/Math.h"
#include "SSWR/AVIRead/AVIRImageBatchForm.h"
#include "Text/MyString.h"
#include "Text/MyStringFloat.h"
#include "UI/FolderDialog.h"
#include "UI/MessageDialog.h"

typedef enum
{
	MNU_EXPORT_SEL = 101,
	MNU_EXPORT_TIF,
	MNU_MOVE_UP,
	MNU_MOVE_DOWN,
	MNU_SEL_ALL
} MenuItems;

void __stdcall SSWR::AVIRead::AVIRImageBatchForm::OnFolderClicked(void *userObj)
{
	SSWR::AVIRead::AVIRImageBatchForm *me = (SSWR::AVIRead::AVIRImageBatchForm*)userObj;
	const UTF8Char *path;
	UTF8Char sbuff[512];
	UTF8Char *sptr;
	UI::FolderDialog *dlg;
	path = me->icMain->GetFolder();
	NEW_CLASS(dlg, UI::FolderDialog(L"sswr", L"AVIRead", L"ImageBatch"));
	if (path)
	{
		dlg->SetFolder(path);
	}
	if (dlg->ShowDialog(me->GetHandle()))
	{
		path = dlg->GetFolder();
		sptr = Text::StrConcat(sbuff, path);
		if (sptr[-1] != IO::Path::PATH_SEPERATOR)
		{
			*sptr++ = IO::Path::PATH_SEPERATOR;
		}
		Text::StrConcat(sptr, IO::Path::ALL_FILES);
		UOSInt fileCnt = 0;
		IO::Path::FindFileSession *sess = IO::Path::FindFile(sbuff);
		if (sess)
		{
			IO::Path::PathType pt;
			while (IO::Path::FindNextFile(sptr, sess, 0, &pt, 0))
			{
				if (pt == IO::Path::PT_FILE)
				{
					fileCnt++;
				}
			}
			IO::Path::FindFileClose(sess);
		}
		me->selCnt = fileCnt;
		me->prgMain->ProgressStart((const UTF8Char*)"Loading", fileCnt);
		me->icMain->SetFolder(path);
		me->lblFolder->SetText(path);
		me->pbMain->SetImage(0, false);
		SDEL_CLASS(me->dispImage);
		SDEL_CLASS(me->previewImage);
		SDEL_CLASS(me->filteredImage);
	}
	DEL_CLASS(dlg);
}

void __stdcall SSWR::AVIRead::AVIRImageBatchForm::OnImageChanged(void *userObj, const UTF8Char *fileName, const SSWR::AVIRead::AVIRImageControl::ImageSetting *setting)
{
	SSWR::AVIRead::AVIRImageBatchForm *me = (SSWR::AVIRead::AVIRImageBatchForm*)userObj;
	if (fileName == 0)
	{
		SDEL_CLASS(me->dispImage);
		SDEL_CLASS(me->previewImage);
		SDEL_CLASS(me->filteredImage);
		me->pbMain->SetImage(0, false);
	}
	else
	{
		Media::StaticImage *img = me->icMain->LoadImage(fileName);
		me->pbMain->SetImage(0, false);
		SDEL_CLASS(me->dispImage);
		SDEL_CLASS(me->previewImage);
		SDEL_CLASS(me->filteredImage);
		me->dispImage = img;
		if (img)
		{
			UOSInt w;
			UOSInt h;
			img->To32bpp();
			me->pbMain->GetSizeP(&w, &h);
			me->resizer->SetTargetWidth(w);
			me->resizer->SetTargetHeight(h);
			me->resizer->SetDestProfile(img->info->color);
			me->previewImage = me->resizer->ProcessToNew(img);
			me->filteredImage = me->previewImage->CreateStaticImage();
			me->initPos = true;
			me->hsbBright->SetPos((UOSInt)Math::Double2OSInt(setting->brightness * 1000));
			me->hsbContr->SetPos((UOSInt)Math::Double2OSInt(setting->contrast * 100));
			me->hsbGamma->SetPos((UOSInt)Math::Double2OSInt(setting->gamma * 100));
			me->hsbHDRLev->SetPos((setting->flags & 240) >> 4);
			me->initPos = false;
			me->UpdatePreview();
		}
	}
}

void __stdcall SSWR::AVIRead::AVIRImageBatchForm::OnColorChg(void *userObj, UOSInt newPos)
{
	SSWR::AVIRead::AVIRImageBatchForm *me = (SSWR::AVIRead::AVIRImageBatchForm*)userObj;
	UTF8Char sbuff[256];

	Double bvalue = Math::UOSInt2Double(me->hsbBright->GetPos()) * 0.1;
	Double cvalue = Math::UOSInt2Double(me->hsbContr->GetPos());
	Double gvalue = Math::UOSInt2Double(me->hsbGamma->GetPos());
	UOSInt hdrLev = me->hsbHDRLev->GetPos();
	Text::StrConcat(Text::StrDouble(sbuff, bvalue), (const UTF8Char*)"%");
	me->lblBrightV->SetText(sbuff);
	Text::StrConcat(Text::StrDouble(sbuff, cvalue), (const UTF8Char*)"%");
	me->lblContrV->SetText(sbuff);
	Text::StrConcat(Text::StrDouble(sbuff, gvalue), (const UTF8Char*)"%");
	me->lblGammaV->SetText(sbuff);
	Text::StrUOSInt(sbuff, hdrLev);
	me->lblHDRLevV->SetText(sbuff);

	if (!me->initPos)
	{
		SSWR::AVIRead::AVIRImageControl::ImageSetting setting;
		setting.brightness = bvalue * 0.01;
		setting.contrast = cvalue * 0.01;
		setting.gamma = gvalue * 0.01;
		setting.flags = (Int32)(hdrLev << 4);
		me->icMain->UpdateImgSetting(&setting);
		me->UpdatePreview();
	}
}

Bool __stdcall SSWR::AVIRead::AVIRImageBatchForm::OnFormClosing(void *userObj, CloseReason reason)
{
	SSWR::AVIRead::AVIRImageBatchForm *me = (SSWR::AVIRead::AVIRImageBatchForm*)userObj;
	if (me->icMain->IsLoadingDir())
	{
		return !UI::MessageDialog::ShowYesNoDialog((const UTF8Char*)"Still loading directory, setting will be lost. Continue?", (const UTF8Char*)"Question", me);
	}
	return false;
}

void __stdcall SSWR::AVIRead::AVIRImageBatchForm::OnProgressUpdated(void *userObj, UOSInt finCnt)
{
	SSWR::AVIRead::AVIRImageBatchForm *me = (SSWR::AVIRead::AVIRImageBatchForm*)userObj;
	me->prgMain->ProgressUpdate(finCnt, me->selCnt);
	if (finCnt >= me->selCnt)
	{
		me->prgMain->ProgressEnd();
	}
}

void SSWR::AVIRead::AVIRImageBatchForm::UpdatePreview()
{
	if (this->filteredImage)
	{
		SSWR::AVIRead::AVIRImageControl::ImageSetting setting;

		setting.brightness = Math::UOSInt2Double(this->hsbBright->GetPos()) * 0.001;
		setting.contrast = Math::UOSInt2Double(this->hsbContr->GetPos()) * 0.01;
		setting.gamma = Math::UOSInt2Double(this->hsbGamma->GetPos()) * 0.01;
		setting.flags = (Int32)(this->hsbHDRLev->GetPos() << 4);
		this->icMain->ApplySetting(this->previewImage, this->filteredImage, &setting);
		this->pbMain->SetImage(this->filteredImage, true);
	}
}

SSWR::AVIRead::AVIRImageBatchForm::AVIRImageBatchForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core) : UI::GUIForm(parent, 1024, 768, ui)
{
	this->SetFont(0, 8.25, false);
	this->SetText((const UTF8Char*)"Image batch tool");
	this->SetFormState(UI::GUIForm::FS_MAXIMIZED);

	this->core = core;
	this->colorSess = this->core->GetColorMgr()->CreateSess(this->GetHMonitor());
	this->dispImage = 0;
	this->previewImage = 0;
	this->filteredImage = 0;
	this->selCnt = 0;
	this->initPos = false;
	Media::ColorProfile srcProfile(Media::ColorProfile::CPT_SRGB);
	Media::ColorProfile destProfile(Media::ColorProfile::CPT_SRGB);
	NEW_CLASS(this->resizer, Media::Resizer::LanczosResizer8_C8(4, 3, &srcProfile, &destProfile, this->colorSess, Media::AT_NO_ALPHA));
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));

	NEW_CLASS(this->pnlCtrl, UI::GUIPanel(ui, this));
	this->pnlCtrl->SetRect(0, 0, 100, 24, false);
	this->pnlCtrl->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->btnFolder, UI::GUIButton(ui, this->pnlCtrl, (const UTF8Char*)"Folder..."));
	this->btnFolder->SetRect(0, 0, 75, 23, false);
	this->btnFolder->SetDockType(UI::GUIControl::DOCK_LEFT);
	this->btnFolder->HandleButtonClick(OnFolderClicked, this);
	NEW_CLASS(this->lblFolder, UI::GUILabel(ui, this->pnlCtrl, (const UTF8Char*)""));
	this->lblFolder->SetRect(0, 0, 480, 23, false);
	this->lblFolder->SetDockType(UI::GUIControl::DOCK_LEFT);
	NEW_CLASS(this->prgMain, UI::GUIProgressBar(ui, this->pnlCtrl, 1));
	this->prgMain->SetDockType(UI::GUIControl::DOCK_FILL);

	NEW_CLASS(this->icMain, SSWR::AVIRead::AVIRImageControl(ui, this, this->core, this, this->colorSess));
	this->icMain->SetRect(0, 0, 200, 100, false);
	this->icMain->SetDockType(UI::GUIControl::DOCK_LEFT);
	this->icMain->SetDispImageHandler(OnImageChanged, this);
	this->icMain->SetProgressHandler(OnProgressUpdated, this);
//	NEW_CLASS(this->hspLeft, UI::GUIHSplitter(ui, this, 3, false));
	NEW_CLASS(this->pnlImage, UI::GUIPanel(ui, this));
	this->pnlImage->SetRect(0, 0, 200, 100, false);
	this->pnlImage->SetDockType(UI::GUIControl::DOCK_BOTTOM);
	NEW_CLASS(this->pbMain, UI::GUIPictureBoxDD(ui, this, this->colorSess, true, false));
	this->pbMain->SetDockType(UI::GUIControl::DOCK_FILL);

	NEW_CLASS(this->lblBright, UI::GUILabel(ui, this->pnlImage, (const UTF8Char*)"Brightness"));
	this->lblBright->SetRect(0, 0, 100, 23, false);
	NEW_CLASS(this->hsbBright, UI::GUIHScrollBar(ui, this->pnlImage, 16));
	this->hsbBright->SetRect(100, 0, 400, 23, false);
	this->hsbBright->InitScrollBar(0, 10009, 1, 10);
	this->hsbBright->SetPos(1000);
	this->hsbBright->HandlePosChanged(OnColorChg, this);
	NEW_CLASS(this->lblBrightV, UI::GUILabel(ui, this->pnlImage, (const UTF8Char*)""));
	this->lblBrightV->SetRect(500, 0,100, 23, false);
	NEW_CLASS(this->lblContr, UI::GUILabel(ui, this->pnlImage, (const UTF8Char*)"Contrast"));
	this->lblContr->SetRect(0, 24, 100, 23, false);
	NEW_CLASS(this->hsbContr, UI::GUIHScrollBar(ui, this->pnlImage, 16));
	this->hsbContr->SetRect(100, 24, 400, 23, false);
	this->hsbContr->InitScrollBar(0, 1609, 1, 10);
	this->hsbContr->SetPos(100);
	this->hsbContr->HandlePosChanged(OnColorChg, this);
	NEW_CLASS(this->lblContrV, UI::GUILabel(ui, this->pnlImage, (const UTF8Char*)""));
	this->lblContrV->SetRect(500, 24,100, 23, false);
	NEW_CLASS(this->lblGamma, UI::GUILabel(ui, this->pnlImage, (const UTF8Char*)"Gamma"));
	this->lblGamma->SetRect(0, 48, 100, 23, false);
	NEW_CLASS(this->hsbGamma, UI::GUIHScrollBar(ui, this->pnlImage, 16));
	this->hsbGamma->SetRect(100, 48, 400, 23, false);
	this->hsbGamma->InitScrollBar(0, 1009, 1, 10);
	this->hsbGamma->SetPos(100);
	this->hsbGamma->HandlePosChanged(OnColorChg, this);
	NEW_CLASS(this->lblGammaV, UI::GUILabel(ui, this->pnlImage, (const UTF8Char*)""));
	this->lblGammaV->SetRect(500, 48, 100, 23, false);
	NEW_CLASS(this->lblHDRLev, UI::GUILabel(ui, this->pnlImage, (const UTF8Char*)"HDR Lev"));
	this->lblHDRLev->SetRect(0, 72, 100, 23, false);
	NEW_CLASS(this->hsbHDRLev, UI::GUIHScrollBar(ui, this->pnlImage, 16));
	this->hsbHDRLev->SetRect(100, 72, 400, 23, false);
	this->hsbHDRLev->InitScrollBar(0, 15, 1, 1);
	this->hsbHDRLev->SetPos(0);
	this->hsbHDRLev->HandlePosChanged(OnColorChg, this);
	NEW_CLASS(this->lblHDRLevV, UI::GUILabel(ui, this->pnlImage, (const UTF8Char*)""));
	this->lblHDRLevV->SetRect(500, 72, 100, 23, false);
/*	NEW_CLASS(this->btnCrop, UI::GUIButton(ui, this->pnlImage, L"Crop"));
	this->btnCrop->SetRect(632, 72, 75, 23, false);*/

	UI::GUIMainMenu *mmnu;
	UI::GUIMenu *mnu;
	NEW_CLASS(mmnu, UI::GUIMainMenu());
	mnu = mmnu->AddSubMenu((const UTF8Char*)"&Export");
	mnu->AddItem((const UTF8Char*)"Export Selected", MNU_EXPORT_SEL, UI::GUIMenu::KM_CONTROL, UI::GUIControl::GK_S);
	mnu->AddItem((const UTF8Char*)"Export as TIF", MNU_EXPORT_TIF, UI::GUIMenu::KM_CONTROL, UI::GUIControl::GK_T);
	mnu = mmnu->AddSubMenu((const UTF8Char*)"&Navigate");
	mnu->AddItem((const UTF8Char*)"Move Up", MNU_MOVE_UP, UI::GUIMenu::KM_NONE, UI::GUIControl::GK_UP);
	mnu->AddItem((const UTF8Char*)"Move Down", MNU_MOVE_DOWN, UI::GUIMenu::KM_NONE, UI::GUIControl::GK_DOWN);
	mnu->AddSeperator();
	mnu->AddItem((const UTF8Char*)"Select All", MNU_SEL_ALL, UI::GUIMenu::KM_CONTROL, UI::GUIControl::GK_A);
	this->SetMenu(mmnu);
	this->SetClosingHandler(OnFormClosing, this);
}

SSWR::AVIRead::AVIRImageBatchForm::~AVIRImageBatchForm()
{
	if (!this->icMain->IsLoadingDir())
	{
		this->icMain->SaveSetting();
	}
	this->pbMain->SetImage(0, false);
	SDEL_CLASS(this->dispImage);
	SDEL_CLASS(this->previewImage);
	SDEL_CLASS(this->filteredImage);
	DEL_CLASS(this->resizer);
	this->ClearChildren();
	this->core->GetColorMgr()->DeleteSess(this->colorSess);
}

void SSWR::AVIRead::AVIRImageBatchForm::EventMenuClicked(UInt16 cmdId)
{
	switch (cmdId)
	{
	case MNU_EXPORT_SEL:
		this->icMain->SetExportFormat(SSWR::AVIRead::AVIRImageControl::EF_JPG);
		this->prgMain->ProgressStart((const UTF8Char*)"Test", this->selCnt = this->icMain->ExportSelected());
		break;
	case MNU_EXPORT_TIF:
		this->icMain->SetExportFormat(SSWR::AVIRead::AVIRImageControl::EF_TIF);
		this->prgMain->ProgressStart((const UTF8Char*)"Test", this->selCnt = this->icMain->ExportSelected());
		break;
	case MNU_MOVE_UP:
		this->icMain->MoveUp();
		break;
	case MNU_MOVE_DOWN:
		this->icMain->MoveDown();
		break;
	case MNU_SEL_ALL:
		this->icMain->SelectAll();
		break;
	}
}

void SSWR::AVIRead::AVIRImageBatchForm::OnMonitorChanged()
{
	this->colorSess->ChangeMonitor(this->GetHMonitor());
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}
