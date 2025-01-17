#include "Stdafx.h"
#include "IO/Path.h"
#include "Math/Math.h"
#include "Media/ICCProfile.h"
#include "SSWR/AVIRead/AVIRColorCustomForm.h"
#include "SSWR/AVIRead/AVIRColorSettingForm.h"
#include "Text/MyString.h"
#include "Text/MyStringFloat.h"
#include "UI/FileDialog.h"
#include "UI/MessageDialog.h"

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnMonProfileChg(void *userObj)
{
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	if (me->inited)
	{
		UTF8Char sbuff[32];
		Media::ColorProfile::CommonProfileType cpt = (Media::ColorProfile::CommonProfileType)(OSInt)me->cboMonProfile->GetSelectedItem();
		if (cpt == Media::ColorProfile::CPT_FILE)
		{
			const Media::IColorHandler::RGBPARAM2 *rgbParam = me->monColor->GetRGBParam();
			me->txtMonTran->SetText(Media::CS::TransferFunc::GetTransferFuncName(rgbParam->monProfile->GetRTranParam()->GetTranType()));
			me->txtMonPrimaries->SetText(Media::ColorProfile::GetNameColorType(rgbParam->monProfile->GetPrimaries()->colorType));
			Text::StrDouble(sbuff, rgbParam->monProfile->GetRTranParam()->GetGamma());
			me->txtMonRGBGamma->SetText(sbuff);
		}
		else
		{
			me->monColor->SetMonProfileType(cpt);
			const Media::IColorHandler::RGBPARAM2 *rgbParam = me->monColor->GetRGBParam();
			me->txtMonTran->SetText(Media::CS::TransferFunc::GetTransferFuncName(rgbParam->monProfile->GetRTranParam()->GetTranType()));
			me->txtMonPrimaries->SetText(Media::ColorProfile::GetNameColorType(rgbParam->monProfile->GetPrimaries()->colorType));
			Text::StrDouble(sbuff, rgbParam->monProfile->GetRTranParam()->GetGamma());
			me->txtMonRGBGamma->SetText(sbuff);
		}
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnMonProfileClicked(void *userObj)
{
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	if (me->inited)
	{
		UI::FileDialog *dlg;
		UTF8Char sbuff[512];
		UTF8Char *sptr;
		NEW_CLASS(dlg, UI::FileDialog(L"SSWR", L"AVIRead", L"ColorMonProfile", false));
		dlg->AddFilter((const UTF8Char*)"*.icc", (const UTF8Char*)"ICC File");
		dlg->AddFilter((const UTF8Char*)"*.icm", (const UTF8Char*)"ICM File");
		dlg->SetAllowMultiSel(false);
		sptr = Media::ICCProfile::GetProfilePath(sbuff);
		*sptr++ = IO::Path::PATH_SEPERATOR;
		*sptr = 0;
		dlg->SetFileName(sbuff);
		if (dlg->ShowDialog(me->GetHandle()))
		{
			if (me->monColor->SetMonProfileFile(dlg->GetFileName()))
			{
				SDEL_TEXT(me->monFileName);
				me->monFileName = Text::StrCopyNew(dlg->GetFileName());

				const Media::IColorHandler::RGBPARAM2 *rgbParam = me->monColor->GetRGBParam();
				me->txtMonTran->SetText(Media::CS::TransferFunc::GetTransferFuncName(rgbParam->monProfile->GetRTranParam()->GetTranType()));
				me->txtMonPrimaries->SetText(Media::ColorProfile::GetNameColorType(rgbParam->monProfile->GetPrimaries()->colorType));
				Text::StrDouble(sbuff, rgbParam->monProfile->GetRTranParam()->GetGamma());
				me->txtMonRGBGamma->SetText(sbuff);
				me->cboMonProfile->SetSelectedIndex(me->cboMonProfile->GetCount() - 1);
			}
		}
		DEL_CLASS(dlg);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnMonCustomClicked(void *userObj)
{
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	if (me->inited)
	{
		UTF8Char sbuff[64];
		SSWR::AVIRead::AVIRColorCustomForm *frm;
		NEW_CLASS(frm, SSWR::AVIRead::AVIRColorCustomForm(0, me->ui, me->core, me->monColor));
		frm->ShowDialog(me);
		DEL_CLASS(frm);

		const Media::IColorHandler::RGBPARAM2 *rgbParam = me->monColor->GetRGBParam();
		me->txtMonTran->SetText(Media::CS::TransferFunc::GetTransferFuncName(rgbParam->monProfile->GetRTranParam()->GetTranType()));
		me->txtMonPrimaries->SetText(Media::ColorProfile::GetNameColorType(rgbParam->monProfile->GetPrimaries()->colorType));
		Text::StrDouble(sbuff, rgbParam->monProfile->GetRTranParam()->GetGamma());
		me->txtMonRGBGamma->SetText(sbuff);
		me->cboMonProfile->SetSelectedIndex(me->cboMonProfile->GetCount() - 2);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnDefVProfileChg(void *userObj)
{
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	if (me->inited)
	{
		me->colorMgr->SetDefVProfile((Media::ColorProfile::CommonProfileType)(OSInt)me->cboDefVProfile->GetSelectedItem());
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnDefPProfileChg(void *userObj)
{
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	if (me->inited)
	{
		me->colorMgr->SetDefPProfile((Media::ColorProfile::CommonProfileType)(OSInt)me->cboDefPProfile->GetSelectedItem());
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::On10BitChanged(void *userObj, Bool newVal)
{
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	if (me->inited)
	{
		me->monColor->Set10BitColor(newVal);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnOKClicked(void *userObj)
{
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Text::StringBuilderUTF8 sb;
	me->txtLuminance->GetText(&sb);
	Double val;
	if (Text::StrToDouble(sb.ToString(), &val))
	{
		me->monColor->SetMonLuminance(val);
	}
	else
	{
		UI::MessageDialog::ShowDialog((const UTF8Char *)"Luminance is not valid", (const UTF8Char *)"Error", me);
		return;
	}
	me->monColor->Save();
	me->colorMgr->SaveDef();
	me->normalClose = true;
	me->SetDialogResult(UI::GUIForm::DR_OK);
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnCancelClicked(void *userObj)
{
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	me->monColor->Load();
	me->colorMgr->LoadDef();
	me->normalClose = true;
	me->SetDialogResult(UI::GUIForm::DR_CANCEL);
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnCalRedBChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Text::StrConcat(Text::StrDouble(sbuff, Math::UOSInt2Double(newPos) * 0.1), (const UTF8Char*)"%");
	me->lblCalRedBV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetRMonBright(Math::UOSInt2Double(newPos) * 0.001);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnCalRedCChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Text::StrConcat(Text::StrDouble(sbuff, Math::UOSInt2Double(newPos) * 0.1), (const UTF8Char*)"%");
	me->lblCalRedCV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetRMonContr(Math::UOSInt2Double(newPos) * 0.001);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnCalRedGChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Double v = Math::UOSInt2Double(newPos) * 0.001;
	Text::StrDouble(sbuff, v);
	me->lblCalRedGV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetRMonGamma(v);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnCalGreenBChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Text::StrConcat(Text::StrDouble(sbuff, Math::UOSInt2Double(newPos) * 0.1), (const UTF8Char*)"%");
	me->lblCalGreenBV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetGMonBright(Math::UOSInt2Double(newPos) * 0.001);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnCalGreenCChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Text::StrConcat(Text::StrDouble(sbuff, Math::UOSInt2Double(newPos) * 0.1), (const UTF8Char*)"%");
	me->lblCalGreenCV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetGMonContr(Math::UOSInt2Double(newPos) * 0.001);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnCalGreenGChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Double v = Math::UOSInt2Double(newPos) * 0.001;
	Text::StrDouble(sbuff, v);
	me->lblCalGreenGV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetGMonGamma(v);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnCalBlueBChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Text::StrConcat(Text::StrDouble(sbuff, Math::UOSInt2Double(newPos) * 0.1), (const UTF8Char*)"%");
	me->lblCalBlueBV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetBMonBright(Math::UOSInt2Double(newPos) * 0.001);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnCalBlueCChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Text::StrConcat(Text::StrDouble(sbuff, Math::UOSInt2Double(newPos) * 0.1), (const UTF8Char*)"%");
	me->lblCalBlueCV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetBMonContr(Math::UOSInt2Double(newPos) * 0.001);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnCalBlueGChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Double v = Math::UOSInt2Double(newPos) * 0.001;
	Text::StrDouble(sbuff, v);
	me->lblCalBlueGV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetBMonGamma(v);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnMonVBrigChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Text::StrConcat(Text::StrDouble(sbuff, Math::UOSInt2Double(newPos) * 0.1), (const UTF8Char*)"%");
	me->lblMonVBrigV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetMonVBright(Math::UOSInt2Double(newPos) * 0.001);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnMonPBrigChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Text::StrConcat(Text::StrDouble(sbuff, Math::UOSInt2Double(newPos) * 0.1), (const UTF8Char*)"%");
	me->lblMonPBrigV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetMonPBright(Math::UOSInt2Double(newPos) * 0.001);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnMonRBrigChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Text::StrConcat(Text::StrDouble(sbuff, Math::UOSInt2Double(newPos) * 0.1), (const UTF8Char*)"%");
	me->lblMonRBrigV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetMonRBright(Math::UOSInt2Double(newPos) * 0.001);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnMonGBrigChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Text::StrConcat(Text::StrDouble(sbuff, Math::UOSInt2Double(newPos) * 0.1), (const UTF8Char*)"%");
	me->lblMonGBrigV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetMonGBright(Math::UOSInt2Double(newPos) * 0.001);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnMonBBrigChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Text::StrConcat(Text::StrDouble(sbuff, Math::UOSInt2Double(newPos) * 0.1), (const UTF8Char*)"%");
	me->lblMonBBrigV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetMonBBright(Math::UOSInt2Double(newPos) * 0.001);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnYUVBrigChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Double v = Math::UOSInt2Double(newPos) * 0.001;
	Text::StrDouble(sbuff, v);
	me->lblYUVBrigV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetYUVBright(v);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnYUVContChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Double v = Math::UOSInt2Double(newPos) * 0.001;
	Text::StrDouble(sbuff, v);
	me->lblYUVContV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetYUVContr(v);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnYUVSatChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Double v = Math::UOSInt2Double(newPos) * 0.001;
	Text::StrDouble(sbuff, v);
	me->lblYUVSatV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetYUVSat(v);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnYUVYGammaChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Double v = Math::UOSInt2Double(newPos) * 0.001;
	Text::StrDouble(sbuff, v);
	me->lblYUVYGammaV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetYGamma(v);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnYUVCGammaChg(void *userObj, UOSInt newPos)
{
	UTF8Char sbuff[32];
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	Double v = Math::UOSInt2Double(newPos) * 0.001;
	Text::StrDouble(sbuff, v);
	me->lblYUVCGammaV->SetText(sbuff);
	if (me->inited)
	{
		me->monColor->SetCGamma(v);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnYUVDefBT601Chg(void *userObj, Bool checked)
{
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	if (checked && me->inited)
	{
		me->colorMgr->SetYUVType(Media::ColorProfile::YUVT_BT601);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnYUVDefBT709Chg(void *userObj, Bool checked)
{
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	if (checked && me->inited)
	{
		me->colorMgr->SetYUVType(Media::ColorProfile::YUVT_BT601);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnYUVDefFCCChg(void *userObj, Bool checked)
{
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	if (checked && me->inited)
	{
		me->colorMgr->SetYUVType(Media::ColorProfile::YUVT_BT709);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnYUVDefBT470BGChg(void *userObj, Bool checked)
{
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	if (checked && me->inited)
	{
		me->colorMgr->SetYUVType(Media::ColorProfile::YUVT_BT470BG);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnYUVDefSMPTE170MChg(void *userObj, Bool checked)
{
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	if (checked && me->inited)
	{
		me->colorMgr->SetYUVType(Media::ColorProfile::YUVT_SMPTE170M);
	}
}

void __stdcall SSWR::AVIRead::AVIRColorSettingForm::OnYUVDefSMPTE240MChg(void *userObj, Bool checked)
{
	SSWR::AVIRead::AVIRColorSettingForm *me = (SSWR::AVIRead::AVIRColorSettingForm*)userObj;
	if (checked && me->inited)
	{
		me->colorMgr->SetYUVType(Media::ColorProfile::YUVT_SMPTE240M);
	}
}

SSWR::AVIRead::AVIRColorSettingForm::AVIRColorSettingForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, void *hMon) : UI::GUIForm(parent, 640, 480, ui)
{
	this->SetText((const UTF8Char*)"Monitor Color Setting");
	this->SetFont(0, 8.25, false);
	this->SetNoResize(true);
	this->core = core;
	this->colorMgr = this->core->GetColorMgr();
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
	this->hMon = hMon;
	this->monFileName = 0;
	Media::MonitorInfo monInfo(this->hMon);
	const UTF8Char *monName = monInfo.GetDesc();
	UTF8Char sbuff[32];

	this->normalClose = false;
	this->inited = false;
	this->monColor = this->colorMgr->GetMonColorManager(monInfo.GetMonitorID());
	const UTF8Char *csptr = this->monColor->GetMonProfileFile();
	if (csptr)
	{
		this->monFileName = Text::StrCopyNew(csptr);
	}

	UOSInt i;
	UOSInt j;
	UOSInt k;
	const Media::IColorHandler::RGBPARAM2 *rgbParam;
	const Media::IColorHandler::YUVPARAM *yuvParam;
	rgbParam = this->monColor->GetRGBParam();
	yuvParam = this->monColor->GetYUVParam();

	NEW_CLASS(this->pnlMonitor, UI::GUIPanel(ui, this));
	this->pnlMonitor->SetRect(0, 0, 100, 23, false);
	this->pnlMonitor->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblMonitor, UI::GUILabel(ui, this->pnlMonitor, (const UTF8Char*)"Monitor"));
	this->lblMonitor->SetRect(0, 0, 100, 23, false);
	NEW_CLASS(this->txtMonitor, UI::GUITextBox(ui, this->pnlMonitor, (const UTF8Char*)"", false));
	this->txtMonitor->SetRect(100, 0, 300, 23, false);
	this->txtMonitor->SetReadOnly(true);
	if (monName)
	{
		this->txtMonitor->SetText(monName);
	}
	NEW_CLASS(this->pnlButtons, UI::GUIPanel(ui, this));
	this->pnlButtons->SetRect(0, 0, 100, 40, false);
	this->pnlButtons->SetDockType(UI::GUIControl::DOCK_BOTTOM);
	NEW_CLASS(this->btnOK, UI::GUIButton(ui, this->pnlButtons, (const UTF8Char*)"OK"));
	this->btnOK->SetRect(216, 8, 75, 23, false);
	this->btnOK->HandleButtonClick(OnOKClicked, this);
	NEW_CLASS(this->btnCancel, UI::GUIButton(ui, this->pnlButtons, (const UTF8Char*)"&Cancel"));
	this->btnCancel->SetRect(304, 8, 75, 23, false);
	this->btnCancel->HandleButtonClick(OnCancelClicked, this);
	NEW_CLASS(this->tcMain, UI::GUITabControl(ui, this));
	this->tcMain->SetDockType(UI::GUIControl::DOCK_FILL);

	this->tpSetting = this->tcMain->AddTabPage((const UTF8Char*)"Setting");
	NEW_CLASS(this->lblMonProfile, UI::GUILabel(ui, this->tpSetting, (const UTF8Char*)"Monitor Profile"));
	this->lblMonProfile->SetRect(0, 0, 150, 23, false);
	NEW_CLASS(this->cboMonProfile, UI::GUIComboBox(ui, this->tpSetting, false);
	this->cboMonProfile->SetRect(150, 0, 150, 23, false));
	this->cboMonProfile->HandleSelectionChange(OnMonProfileChg, this);
	i = 0;
	j = Media::ColorProfile::CPT_LAST;
	while (i < j)
	{
		k = this->cboMonProfile->AddItem(Media::ColorProfile::GetNameCommonProfile((Media::ColorProfile::CommonProfileType)i), (void*)i);
		if (i == rgbParam->monProfileType)
		{
			this->cboMonProfile->SetSelectedIndex(k);
		}
		i++;
	}
	k = this->cboMonProfile->AddItem((const UTF8Char*)"As OS", (void*)Media::ColorProfile::CPT_OS);
	if (rgbParam->monProfileType == Media::ColorProfile::CPT_OS)
	{
		this->cboMonProfile->SetSelectedIndex(k);
	}
	k = this->cboMonProfile->AddItem((const UTF8Char*)"EDID", (void*)Media::ColorProfile::CPT_EDID);
	if (rgbParam->monProfileType == Media::ColorProfile::CPT_EDID)
	{
		this->cboMonProfile->SetSelectedIndex(k);
	}
	k = this->cboMonProfile->AddItem((const UTF8Char*)"CUSTOM", (void*)Media::ColorProfile::CPT_CUSTOM);
	if (rgbParam->monProfileType == Media::ColorProfile::CPT_CUSTOM)
	{
		this->cboMonProfile->SetSelectedIndex(k);
	}
	k = this->cboMonProfile->AddItem((const UTF8Char*)"File", (void*)Media::ColorProfile::CPT_FILE);
	if (rgbParam->monProfileType == Media::ColorProfile::CPT_FILE)
	{
		this->cboMonProfile->SetSelectedIndex(k);
	}
	NEW_CLASS(this->btnMonProfile, UI::GUIButton(ui, this->tpSetting, (const UTF8Char*)"B&rowse"));
	this->btnMonProfile->SetRect(300, 0, 75, 23, false);
	this->btnMonProfile->HandleButtonClick(OnMonProfileClicked, this);
	NEW_CLASS(this->grpMonProfile, UI::GUIGroupBox(ui, this->tpSetting, (const UTF8Char*)"Monitor Profile"));
	this->grpMonProfile->SetRect(0, 24, 500, 72, false);
	NEW_CLASS(this->lblMonTran, UI::GUILabel(ui, this->grpMonProfile, (const UTF8Char*)"Transfer Characteristics"));
	this->lblMonTran->SetRect(0, 0, 150, 23, false);
	NEW_CLASS(this->txtMonTran, UI::GUITextBox(ui, this->grpMonProfile, (const UTF8Char*)"");
	this->txtMonTran->SetRect(150, 0, 150, 23, false));
	this->txtMonTran->SetReadOnly(true);
	NEW_CLASS(this->lblMonRGBGamma, UI::GUILabel(ui, this->grpMonProfile, (const UTF8Char*)"Gamma"));
	this->lblMonRGBGamma->SetRect(300, 0, 80, 23, false);
	NEW_CLASS(this->txtMonRGBGamma, UI::GUITextBox(ui, this->grpMonProfile, (const UTF8Char*)""));
	this->txtMonRGBGamma->SetRect(380, 0, 100, 23, false);
	this->txtMonRGBGamma->SetReadOnly(true);
	NEW_CLASS(this->lblMonPrimaries, UI::GUILabel(ui, this->grpMonProfile, (const UTF8Char*)"Color Primaries"));
	this->lblMonPrimaries->SetRect(0, 24, 150, 23, false);
	NEW_CLASS(this->txtMonPrimaries, UI::GUITextBox(ui, this->grpMonProfile, (const UTF8Char*)"");
	this->txtMonPrimaries->SetRect(150, 24, 150, 23, false));
	this->txtMonPrimaries->SetReadOnly(true);
	NEW_CLASS(this->btnMonCustom, UI::GUIButton(ui, this->grpMonProfile, (const UTF8Char*)"Custom"));
	this->btnMonCustom->SetRect(300, 24, 75, 23, false);
	this->btnMonCustom->HandleButtonClick(OnMonCustomClicked, this);
	NEW_CLASS(this->lblDefVProfile, UI::GUILabel(ui, this->tpSetting, (const UTF8Char*)"Def Video Profile"));
	this->lblDefVProfile->SetRect(0, 104, 150, 23, false);
	NEW_CLASS(this->cboDefVProfile, UI::GUIComboBox(ui, this->tpSetting, false));
	this->cboDefVProfile->SetRect(150, 104, 150, 23, false);
	this->cboDefVProfile->HandleSelectionChange(OnDefVProfileChg, this);
	NEW_CLASS(this->lblDefPProfile, UI::GUILabel(ui, this->tpSetting, (const UTF8Char*)"Def Photo Profile"));
	this->lblDefPProfile->SetRect(0, 128, 150, 23, false);
	NEW_CLASS(this->cboDefPProfile, UI::GUIComboBox(ui, this->tpSetting, false));
	this->cboDefPProfile->SetRect(150, 128, 150, 23, false);
	this->cboDefPProfile->HandleSelectionChange(OnDefPProfileChg, this);
	i = 0;
	j = Media::ColorProfile::CPT_LAST;
	while (i < j)
	{
		k = this->cboDefVProfile->AddItem(Media::ColorProfile::GetNameCommonProfile((Media::ColorProfile::CommonProfileType)i), (void*)i);
		if (i == this->colorMgr->GetDefVProfileType())
		{
			this->cboDefVProfile->SetSelectedIndex(k);
		}
		k = this->cboDefPProfile->AddItem(Media::ColorProfile::GetNameCommonProfile((Media::ColorProfile::CommonProfileType)i), (void*)i);
		if (i == this->colorMgr->GetDefPProfileType())
		{
			this->cboDefPProfile->SetSelectedIndex(k);
		}
		i++;
	}
	NEW_CLASS(this->chk10Bit, UI::GUICheckBox(ui, this->tpSetting, (const UTF8Char*)"10-Bit Color", false));
	this->chk10Bit->SetRect(0, 152, 150, 23, false);
	if (this->monColor->Get10BitColor())
	{
		this->chk10Bit->SetChecked(true);
	}
	this->chk10Bit->HandleCheckedChange(On10BitChanged, this);
	NEW_CLASS(this->lblLuminance, UI::GUILabel(ui, this->tpSetting, (const UTF8Char*)"Luminance"));
	this->lblLuminance->SetRect(0, 176, 100, 23, false);
	NEW_CLASS(this->txtLuminance, UI::GUITextBox(ui, this->tpSetting, (const UTF8Char*)"250"));
	this->txtLuminance->SetRect(100, 176, 50, 23, false);
	NEW_CLASS(this->lblLuminanceUnit, UI::GUILabel(ui, this->tpSetting, (const UTF8Char*)"cd/m2"));
	this->lblLuminanceUnit->SetRect(150, 176, 100, 23, false);
	NEW_CLASS(this->grpYUVDef, UI::GUIGroupBox(ui, this->tpSetting, (const UTF8Char*)"Default YUV Type"));
	this->grpYUVDef->SetRect(0, 208, 280, 96, false);
	NEW_CLASS(this->radYUVDefBT601, UI::GUIRadioButton(ui, this->grpYUVDef, (const UTF8Char*)"BT601", false));
	this->radYUVDefBT601->SetRect(8, 0, 100, 23, false);
	this->radYUVDefBT601->HandleSelectedChange(OnYUVDefBT601Chg, this);
	NEW_CLASS(this->radYUVDefBT709, UI::GUIRadioButton(ui, this->grpYUVDef, (const UTF8Char*)"BT709", false));
	this->radYUVDefBT709->SetRect(8, 24, 100, 23, false);
	this->radYUVDefBT709->HandleSelectedChange(OnYUVDefBT709Chg, this);
	NEW_CLASS(this->radYUVDefFCC, UI::GUIRadioButton(ui, this->grpYUVDef, (const UTF8Char*)"FCC", false));
	this->radYUVDefFCC->SetRect(8, 48, 100, 23, false);
	this->radYUVDefFCC->HandleSelectedChange(OnYUVDefFCCChg, this);
	NEW_CLASS(this->radYUVDefBT470BG, UI::GUIRadioButton(ui, this->grpYUVDef, (const UTF8Char*)"BT470BG", false));
	this->radYUVDefBT470BG->SetRect(144, 0, 100, 23, false);
	this->radYUVDefBT470BG->HandleSelectedChange(OnYUVDefBT470BGChg, this);
	NEW_CLASS(this->radYUVDefSMPTE170M, UI::GUIRadioButton(ui, this->grpYUVDef, (const UTF8Char*)"SMPTE 170M", false));
	this->radYUVDefSMPTE170M->SetRect(144, 24, 100, 23, false);
	this->radYUVDefSMPTE170M->HandleSelectedChange(OnYUVDefSMPTE170MChg, this);
	NEW_CLASS(this->radYUVDefSMPTE240M, UI::GUIRadioButton(ui, this->grpYUVDef, (const UTF8Char*)"SMPTE 240M", false));
	this->radYUVDefSMPTE240M->SetRect(144, 48, 100, 23, false);
	this->radYUVDefSMPTE240M->HandleSelectedChange(OnYUVDefSMPTE240MChg, this);

	this->tpCal = this->tcMain->AddTabPage((const UTF8Char*)"Calibrate");
	NEW_CLASS(this->pnlCalRed, UI::GUIPanel(ui, this->tpCal));
	this->pnlCalRed->SetBGColor(0xffffc0c0);
	this->pnlCalRed->SetRect(0, 0, 608, 104, false);
	this->pnlCalRed->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblCalRedB, UI::GUILabel(ui, this->pnlCalRed, (const UTF8Char*)"Brightness"));
	this->lblCalRedB->SetRect(8, 8, 120, 16, false);
	this->lblCalRedB->SetBGColor(0xffffc0c0);
	NEW_CLASS(this->hsbCalRedB, UI::GUIHScrollBar(ui, this->pnlCalRed, 16));
	this->hsbCalRedB->SetRect(128, 8, 392, 17, false);
	this->hsbCalRedB->InitScrollBar(0, 4099, 1000, 100);
	this->hsbCalRedB->HandlePosChanged(OnCalRedBChg, this);
	NEW_CLASS(this->lblCalRedBV, UI::GUILabel(ui, this->pnlCalRed, (const UTF8Char*)""));
	this->lblCalRedBV->SetRect(520, 8, 64, 16, false);
	NEW_CLASS(this->lblCalRedC, UI::GUILabel(ui, this->pnlCalRed, (const UTF8Char*)"Contrast"));
	this->lblCalRedC->SetRect(8, 32, 120, 16, false);
	NEW_CLASS(this->hsbCalRedC, UI::GUIHScrollBar(ui, this->pnlCalRed, 16));
	this->hsbCalRedC->SetRect(128, 32, 392, 17, false);
	this->hsbCalRedC->InitScrollBar(0, 4099, 1000, 100);
	this->hsbCalRedC->HandlePosChanged(OnCalRedCChg, this);
	NEW_CLASS(this->lblCalRedCV, UI::GUILabel(ui, this->pnlCalRed, (const UTF8Char*)""));
	this->lblCalRedCV->SetRect(520, 32, 64, 16, false);
	NEW_CLASS(this->lblCalRedG, UI::GUILabel(ui, this->pnlCalRed, (const UTF8Char*)"Gamma"));
	this->lblCalRedG->SetRect(8, 56, 120, 16, false);
	NEW_CLASS(this->hsbCalRedG, UI::GUIHScrollBar(ui, this->pnlCalRed, 16));
	this->hsbCalRedG->SetRect(128, 56, 392, 17, false);
	this->hsbCalRedG->InitScrollBar(0, 4099, 1000, 100);
	this->hsbCalRedG->HandlePosChanged(OnCalRedGChg, this);
	NEW_CLASS(this->lblCalRedGV, UI::GUILabel(ui, this->pnlCalRed, (const UTF8Char*)""));
	this->lblCalRedGV->SetRect(520, 56, 64, 16, false);
	NEW_CLASS(this->pnlCalGreen, UI::GUIPanel(ui, this->tpCal));
	this->pnlCalGreen->SetBGColor(0xffc0ffc0);
	this->pnlCalGreen->SetRect(0, 0, 608, 104, false);
	this->pnlCalGreen->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblCalGreenB, UI::GUILabel(ui, this->pnlCalGreen, (const UTF8Char*)"Brightness"));
	this->lblCalGreenB->SetRect(8, 8, 120, 16, false);
	NEW_CLASS(this->hsbCalGreenB, UI::GUIHScrollBar(ui, this->pnlCalGreen, 16));
	this->hsbCalGreenB->SetRect(128, 8, 392, 17, false);
	this->hsbCalGreenB->InitScrollBar(0, 4099, 1000, 100);
	this->hsbCalGreenB->HandlePosChanged(OnCalGreenBChg, this);
	NEW_CLASS(this->lblCalGreenBV, UI::GUILabel(ui, this->pnlCalGreen, (const UTF8Char*)""));
	this->lblCalGreenBV->SetRect(520, 8, 64, 16, false);
	NEW_CLASS(this->lblCalGreenC, UI::GUILabel(ui, this->pnlCalGreen, (const UTF8Char*)"Contrast"));
	this->lblCalGreenC->SetRect(8, 32, 120, 16, false);
	NEW_CLASS(this->hsbCalGreenC, UI::GUIHScrollBar(ui, this->pnlCalGreen, 16));
	this->hsbCalGreenC->SetRect(128, 32, 392, 17, false);
	this->hsbCalGreenC->InitScrollBar(0, 4099, 1000, 100);
	this->hsbCalGreenC->HandlePosChanged(OnCalGreenCChg, this);
	NEW_CLASS(this->lblCalGreenCV, UI::GUILabel(ui, this->pnlCalGreen, (const UTF8Char*)""));
	this->lblCalGreenCV->SetRect(520, 32, 64, 16, false);
	NEW_CLASS(this->lblCalGreenG, UI::GUILabel(ui, this->pnlCalGreen, (const UTF8Char*)"Gamma"));
	this->lblCalGreenG->SetRect(8, 56, 120, 16, false);
	NEW_CLASS(this->hsbCalGreenG, UI::GUIHScrollBar(ui, this->pnlCalGreen, 16));
	this->hsbCalGreenG->SetRect(128, 56, 392, 17, false);
	this->hsbCalGreenG->InitScrollBar(0, 4099, 1000, 100);
	this->hsbCalGreenG->HandlePosChanged(OnCalGreenGChg, this);
	NEW_CLASS(this->lblCalGreenGV, UI::GUILabel(ui, this->pnlCalGreen, (const UTF8Char*)""));
	this->lblCalGreenGV->SetRect(520, 56, 64, 16, false);
	NEW_CLASS(this->pnlCalBlue, UI::GUIPanel(ui, this->tpCal));
	this->pnlCalBlue->SetBGColor(0xffc0c0ff);
	this->pnlCalBlue->SetRect(0, 0, 608, 104, false);
	this->pnlCalBlue->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblCalBlueB, UI::GUILabel(ui, this->pnlCalBlue, (const UTF8Char*)"Brightness"));
	this->lblCalBlueB->SetRect(8, 8, 120, 16, false);
	NEW_CLASS(this->hsbCalBlueB, UI::GUIHScrollBar(ui, this->pnlCalBlue, 16));
	this->hsbCalBlueB->SetRect(128, 8, 392, 17, false);
	this->hsbCalBlueB->InitScrollBar(0, 4099, 1000, 100);
	this->hsbCalBlueB->HandlePosChanged(OnCalBlueBChg, this);
	NEW_CLASS(this->lblCalBlueBV, UI::GUILabel(ui, this->pnlCalBlue, (const UTF8Char*)""));
	this->lblCalBlueBV->SetRect(520, 8, 64, 16, false);
	NEW_CLASS(this->lblCalBlueC, UI::GUILabel(ui, this->pnlCalBlue, (const UTF8Char*)"Contrast"));
	this->lblCalBlueC->SetRect(8, 32, 120, 16, false);
	NEW_CLASS(this->hsbCalBlueC, UI::GUIHScrollBar(ui, this->pnlCalBlue, 16));
	this->hsbCalBlueC->SetRect(128, 32, 392, 17, false);
	this->hsbCalBlueC->InitScrollBar(0, 4099, 1000, 100);
	this->hsbCalBlueC->HandlePosChanged(OnCalBlueCChg, this);
	NEW_CLASS(this->lblCalBlueCV, UI::GUILabel(ui, this->pnlCalBlue, (const UTF8Char*)""));
	this->lblCalBlueCV->SetRect(520, 32, 64, 16, false);
	NEW_CLASS(this->lblCalBlueG, UI::GUILabel(ui, this->pnlCalBlue, (const UTF8Char*)"Gamma"));
	this->lblCalBlueG->SetRect(8, 56, 120, 16, false);
	NEW_CLASS(this->hsbCalBlueG, UI::GUIHScrollBar(ui, this->pnlCalBlue, 16));
	this->hsbCalBlueG->SetRect(128, 56, 392, 17, false);
	this->hsbCalBlueG->InitScrollBar(0, 4099, 1000, 100);
	this->hsbCalBlueG->HandlePosChanged(OnCalBlueGChg, this);
	NEW_CLASS(this->lblCalBlueGV, UI::GUILabel(ui, this->pnlCalBlue, (const UTF8Char*)""));
	this->lblCalBlueGV->SetRect(520, 56, 64, 16, false);

	this->tpRGB = this->tcMain->AddTabPage((const UTF8Char*)"RGB");
	NEW_CLASS(this->lblMonVBrig, UI::GUILabel(ui, this->tpRGB, (const UTF8Char*)"Monitor Video Brightness"));
	this->lblMonVBrig->SetRect(8, 8, 128, 16, false);
	NEW_CLASS(this->hsbMonVBrig, UI::GUIHScrollBar(ui, this->tpRGB, 16));
	this->hsbMonVBrig->SetRect(136, 8, 392, 17, false);
	this->hsbMonVBrig->InitScrollBar(0, 4099, 1000, 100);
	this->hsbMonVBrig->HandlePosChanged(OnMonVBrigChg, this);
	NEW_CLASS(this->lblMonVBrigV, UI::GUILabel(ui, this->tpRGB, (const UTF8Char*)""));
	this->lblMonVBrigV->SetRect(528, 8, 64, 16, false);
	NEW_CLASS(this->lblMonPBrig, UI::GUILabel(ui, this->tpRGB, (const UTF8Char*)"Monitor Photo Brightness"));
	this->lblMonPBrig->SetRect(8, 32, 128, 16, false);
	NEW_CLASS(this->hsbMonPBrig, UI::GUIHScrollBar(ui, this->tpRGB, 16));
	this->hsbMonPBrig->SetRect(136, 32, 392, 17, false);
	this->hsbMonPBrig->InitScrollBar(0, 4099, 1000, 100);
	this->hsbMonPBrig->HandlePosChanged(OnMonPBrigChg, this);
	NEW_CLASS(this->lblMonPBrigV, UI::GUILabel(ui, this->tpRGB, (const UTF8Char*)""));
	this->lblMonPBrigV->SetRect(528, 32, 64, 16, false);
	NEW_CLASS(this->lblMonRBrig, UI::GUILabel(ui, this->tpRGB, (const UTF8Char*)"Monitor R Brightness"));
	this->lblMonRBrig->SetRect(8, 56, 128, 16, false);
	this->lblMonRBrig->SetBGColor(0xffffc0c0);
	NEW_CLASS(this->hsbMonRBrig, UI::GUIHScrollBar(ui, this->tpRGB, 16));
	this->hsbMonRBrig->SetRect(136, 56, 392, 17, false);
	this->hsbMonRBrig->InitScrollBar(0, 4099, 1000, 100);
	this->hsbMonRBrig->HandlePosChanged(OnMonRBrigChg, this);
	NEW_CLASS(this->lblMonRBrigV, UI::GUILabel(ui, this->tpRGB, (const UTF8Char*)""));
	this->lblMonRBrigV->SetRect(528, 56, 64, 16, false);
	NEW_CLASS(this->lblMonGBrig, UI::GUILabel(ui, this->tpRGB, (const UTF8Char*)"Monitor G Brightness"));
	this->lblMonGBrig->SetRect(8, 80, 128, 16, false);
	this->lblMonGBrig->SetBGColor(0xffc0ffc0);
	NEW_CLASS(this->hsbMonGBrig, UI::GUIHScrollBar(ui, this->tpRGB, 16));
	this->hsbMonGBrig->SetRect(136, 80, 392, 17, false);
	this->hsbMonGBrig->InitScrollBar(0, 4099, 1000, 100);
	this->hsbMonGBrig->HandlePosChanged(OnMonGBrigChg, this);
	NEW_CLASS(this->lblMonGBrigV, UI::GUILabel(ui, this->tpRGB, (const UTF8Char*)""));
	this->lblMonGBrigV->SetRect(528, 80, 64, 16, false);
	NEW_CLASS(this->lblMonBBrig, UI::GUILabel(ui, this->tpRGB, (const UTF8Char*)"Monitor B Brightness"));
	this->lblMonBBrig->SetRect(8, 104, 128, 16, false);
	this->lblMonBBrig->SetBGColor(0xffc0c0ff);
	NEW_CLASS(this->hsbMonBBrig, UI::GUIHScrollBar(ui, this->tpRGB, 16));
	this->hsbMonBBrig->SetRect(136, 104, 392, 17, false);
	this->hsbMonBBrig->InitScrollBar(0, 4099, 1000, 100);
	this->hsbMonBBrig->HandlePosChanged(OnMonBBrigChg, this);
	NEW_CLASS(this->lblMonBBrigV, UI::GUILabel(ui, this->tpRGB, (const UTF8Char*)""));
	this->lblMonBBrigV->SetRect(528, 104, 64, 16, false);

	this->tpYUV = this->tcMain->AddTabPage((const UTF8Char*)"YUV");
	NEW_CLASS(this->lblYUVBrig, UI::GUILabel(ui, this->tpYUV, (const UTF8Char*)"Brightness"));
	this->lblYUVBrig->SetRect(8, 8, 120, 16, false);
	NEW_CLASS(this->hsbYUVBrig, UI::GUIHScrollBar(ui, this->tpYUV, 16));
	this->hsbYUVBrig->SetRect(128, 8, 392, 17, false);
	this->hsbYUVBrig->InitScrollBar(0, 4099, 1000, 100);
	this->hsbYUVBrig->HandlePosChanged(OnYUVBrigChg, this);
	NEW_CLASS(this->lblYUVBrigV, UI::GUILabel(ui, this->tpYUV, (const UTF8Char*)""));
	this->lblYUVBrigV->SetRect(520, 8, 64, 16, false);
	NEW_CLASS(this->lblYUVCont, UI::GUILabel(ui, this->tpYUV, (const UTF8Char*)"Contrast"));
	this->lblYUVCont->SetRect(8, 32, 120, 16, false);
	NEW_CLASS(this->hsbYUVCont, UI::GUIHScrollBar(ui, this->tpYUV, 16));
	this->hsbYUVCont->SetRect(128, 32, 392, 17, false);
	this->hsbYUVCont->InitScrollBar(0, 4099, 1000, 100);
	this->hsbYUVCont->HandlePosChanged(OnYUVContChg, this);
	NEW_CLASS(this->lblYUVContV, UI::GUILabel(ui, this->tpYUV, (const UTF8Char*)""));
	this->lblYUVContV->SetRect(520, 32, 64, 16, false);
	NEW_CLASS(this->lblYUVSat, UI::GUILabel(ui, this->tpYUV, (const UTF8Char*)"Saturation"));
	this->lblYUVSat->SetRect(8, 56, 120, 16, false);
	NEW_CLASS(this->hsbYUVSat, UI::GUIHScrollBar(ui, this->tpYUV, 16));
	this->hsbYUVSat->SetRect(128, 56, 392, 17, false);
	this->hsbYUVSat->InitScrollBar(0, 4099, 1000, 100);
	this->hsbYUVSat->HandlePosChanged(OnYUVSatChg, this);
	NEW_CLASS(this->lblYUVSatV, UI::GUILabel(ui, this->tpYUV, (const UTF8Char*)""));
	this->lblYUVSatV->SetRect(520, 56, 64, 16, false);
	NEW_CLASS(this->lblYUVYGamma, UI::GUILabel(ui, this->tpYUV, (const UTF8Char*)"Y-Gamma"));
	this->lblYUVYGamma->SetRect(8, 80, 120, 16, false);
	NEW_CLASS(this->hsbYUVYGamma, UI::GUIHScrollBar(ui, this->tpYUV, 16));
	this->hsbYUVYGamma->SetRect(128, 80, 392, 17, false);
	this->hsbYUVYGamma->InitScrollBar(0, 4099, 1000, 100);
	this->hsbYUVYGamma->HandlePosChanged(OnYUVYGammaChg, this);
	NEW_CLASS(this->lblYUVYGammaV, UI::GUILabel(ui, this->tpYUV, (const UTF8Char*)""));
	this->lblYUVYGammaV->SetRect(520, 80, 64, 16, false);
	NEW_CLASS(this->lblYUVCGamma, UI::GUILabel(ui, this->tpYUV, (const UTF8Char*)"C-Gamma"));
	this->lblYUVCGamma->SetRect(8, 104, 120, 16, false);
	NEW_CLASS(this->hsbYUVCGamma, UI::GUIHScrollBar(ui, this->tpYUV, 16));
	this->hsbYUVCGamma->SetRect(128, 104, 392, 17, false);
	this->hsbYUVCGamma->InitScrollBar(0, 4099, 1000, 100);
	this->hsbYUVCGamma->HandlePosChanged(OnYUVCGammaChg, this);
	NEW_CLASS(this->lblYUVCGammaV, UI::GUILabel(ui, this->tpYUV, (const UTF8Char*)""));
	this->lblYUVCGammaV->SetRect(520, 104, 64, 16, false);

	this->SetDefaultButton(this->btnOK);
	this->SetCancelButton(this->btnCancel);


	this->hsbCalRedB->SetPos((UOSInt)Math::Double2OSInt(rgbParam->MonRBright  * 1000));
	this->hsbCalRedC->SetPos((UOSInt)Math::Double2OSInt(rgbParam->MonRContr * 1000));
	this->hsbCalRedG->SetPos((UOSInt)Math::Double2OSInt(rgbParam->MonRGamma * 1000));
	this->hsbCalGreenB->SetPos((UOSInt)Math::Double2OSInt(rgbParam->MonGBright  * 1000));
	this->hsbCalGreenC->SetPos((UOSInt)Math::Double2OSInt(rgbParam->MonGContr * 1000));
	this->hsbCalGreenG->SetPos((UOSInt)Math::Double2OSInt(rgbParam->MonGGamma * 1000));
	this->hsbCalBlueB->SetPos((UOSInt)Math::Double2OSInt(rgbParam->MonBBright  * 1000));
	this->hsbCalBlueC->SetPos((UOSInt)Math::Double2OSInt(rgbParam->MonBContr * 1000));
	this->hsbCalBlueG->SetPos((UOSInt)Math::Double2OSInt(rgbParam->MonBGamma * 1000));

	this->hsbMonVBrig->SetPos((UOSInt)Math::Double2OSInt(rgbParam->MonVBrightness * 1000));
	this->hsbMonPBrig->SetPos((UOSInt)Math::Double2OSInt(rgbParam->MonPBrightness * 1000));
	this->hsbMonRBrig->SetPos((UOSInt)Math::Double2OSInt(rgbParam->MonRBrightness * 1000));
	this->hsbMonGBrig->SetPos((UOSInt)Math::Double2OSInt(rgbParam->MonGBrightness * 1000));
	this->hsbMonBBrig->SetPos((UOSInt)Math::Double2OSInt(rgbParam->MonBBrightness * 1000));

	this->hsbYUVBrig->SetPos((UOSInt)Math::Double2OSInt(yuvParam->Brightness * 1000));
	this->hsbYUVCont->SetPos((UOSInt)Math::Double2OSInt(yuvParam->Contrast * 1000));
	this->hsbYUVSat->SetPos((UOSInt)Math::Double2OSInt(yuvParam->Saturation * 1000));
	this->hsbYUVYGamma->SetPos((UOSInt)Math::Double2OSInt(yuvParam->YGamma * 1000));
	this->hsbYUVCGamma->SetPos((UOSInt)Math::Double2OSInt(yuvParam->CGamma * 1000));
	switch (this->colorMgr->GetDefYUVType())
	{
	default:
	case Media::ColorProfile::YUVT_BT601:
		this->radYUVDefBT601->Select();
		break;
	case Media::ColorProfile::YUVT_BT709:
		this->radYUVDefBT709->Select();
		break;
	case Media::ColorProfile::YUVT_FCC:
		this->radYUVDefFCC->Select();
		break;
	case Media::ColorProfile::YUVT_BT470BG:
		this->radYUVDefBT470BG->Select();
		break;
	case Media::ColorProfile::YUVT_SMPTE170M:
		this->radYUVDefSMPTE170M->Select();
		break;
	case Media::ColorProfile::YUVT_SMPTE240M:
		this->radYUVDefSMPTE240M->Select();
		break;
	case Media::ColorProfile::YUVT_BT2020:
	case Media::ColorProfile::YUVT_FLAG_YUV_0_255:
	case Media::ColorProfile::YUVT_UNKNOWN:
		break;
	}
	Text::StrDouble(sbuff, rgbParam->monLuminance);
	this->txtLuminance->SetText(sbuff);

	this->inited = true;
	OnMonProfileChg(this);
}

SSWR::AVIRead::AVIRColorSettingForm::~AVIRColorSettingForm()
{
	if (!this->normalClose)
	{
		this->monColor->Load();
	}
	SDEL_TEXT(this->monFileName);
}

void SSWR::AVIRead::AVIRColorSettingForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}
