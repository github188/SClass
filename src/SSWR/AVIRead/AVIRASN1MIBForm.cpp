#include "Stdafx.h"
#include "Data/ByteTool.h"
#include "Net/ASN1OIDDB.h"
#include "Net/ASN1Util.h"
#include "SSWR/AVIRead/AVIRASN1MIBForm.h"
#include "UI/FileDialog.h"
#include "UI/MessageDialog.h"

void __stdcall SSWR::AVIRead::AVIRASN1MIBForm::OnFileDroped(void *userObj, const UTF8Char **files, UOSInt nFiles)
{
	SSWR::AVIRead::AVIRASN1MIBForm *me = (SSWR::AVIRead::AVIRASN1MIBForm*)userObj;
	me->LoadFile(files[0]);
}

void __stdcall SSWR::AVIRead::AVIRASN1MIBForm::OnBrowseClicked(void *userObj)
{
	SSWR::AVIRead::AVIRASN1MIBForm *me = (SSWR::AVIRead::AVIRASN1MIBForm*)userObj;
	Text::StringBuilderUTF8 sb;
	UI::FileDialog *dlg;
	me->txtFile->GetText(&sb);
	NEW_CLASS(dlg, UI::FileDialog(L"SSWR", L"AVIRead", L"ASN1MIB", false));
	if (dlg->ShowDialog(me->GetHandle()))
	{
		me->LoadFile(dlg->GetFileName());
	}
	DEL_CLASS(dlg);
}

void __stdcall SSWR::AVIRead::AVIRASN1MIBForm::OnObjectsSelChg(void *userObj)
{
	SSWR::AVIRead::AVIRASN1MIBForm *me = (SSWR::AVIRead::AVIRASN1MIBForm*)userObj;
	Net::ASN1MIB::ObjectInfo *obj = (Net::ASN1MIB::ObjectInfo*)me->lvObjects->GetSelectedItem();
	me->lvObjectsVal->ClearItems();
	if (obj)
	{
		UOSInt i = 0;
		UOSInt j = obj->valName->GetCount();
		while (i < j)
		{
			me->lvObjectsVal->AddItem(obj->valName->GetItem(i), 0);
			me->lvObjectsVal->SetSubItem(i, 1, obj->valCont->GetItem(i));
			i++;
		}
	}
}

void SSWR::AVIRead::AVIRASN1MIBForm::LoadFile(const UTF8Char *fileName)
{
	Text::StringBuilderUTF8 sb;
	this->txtFile->SetText(fileName);
	this->mib->UnloadAll();
	sb.ClearStr();
	sb.Append((const UTF8Char*)"Error in loading MIB file: ");
	if (this->mib->LoadFile(fileName, &sb))
	{

	}
	else
	{
		UI::MessageDialog::ShowDialog(sb.ToString(), (const UTF8Char*)"SNMP MIB", this);
	}
	this->lvObjects->ClearItems();
	this->lvOID->ClearItems();
	UOSInt i;
	UOSInt j;
	Net::ASN1MIB::ObjectInfo *obj;
	Net::ASN1MIB::ModuleInfo *module = this->mib->GetModuleByFileName(fileName);
	if (module == 0)
	{
		module = this->mib->GetGlobalModule();
	}
	Data::ArrayList<Net::ASN1MIB::ObjectInfo *> *objList = module->objValues;
	i = 0;
	j = objList->GetCount();
	while (i < j)
	{
		obj = objList->GetItem(i);
		this->lvObjects->AddItem(obj->objectName, obj);
		if (obj->oidLen > 0)
		{
			sb.ClearStr();
			Net::ASN1Util::OIDToString(obj->oid, obj->oidLen, &sb);
			this->lvObjects->SetSubItem(i, 1, sb.ToString());
		}
		if (obj->typeName)
		{
			this->lvObjects->SetSubItem(i, 2, obj->typeName);
		}
		if (obj->typeVal)
		{
			this->lvObjects->SetSubItem(i, 3, obj->typeVal);
		}
		i++;
	}
	Text::StringBuilderUTF8 sbOIDText;
	i = 0;
	j = module->oidList->GetCount();
	while (i < j)
	{
		obj = module->oidList->GetItem(i);
		sb.ClearStr();
		Net::ASN1Util::OIDToString(obj->oid, obj->oidLen, &sb);
		this->lvOID->AddItem(sb.ToString(), obj);
		Net::ASN1OIDDB::OIDInfo *entry = Net::ASN1OIDDB::OIDGetEntry(obj->oid, obj->oidLen);
		if (entry)
		{
			this->lvOID->SetSubItem(i, 1, (const UTF8Char*)entry->name);
		}
		this->lvOID->SetSubItem(i, 2, obj->objectName);

		Net::ASN1Util::OIDToCPPCode(obj->oid, obj->oidLen, obj->objectName, &sbOIDText);
		i++;
	}
	this->txtOIDText->SetText(sbOIDText.ToString());
}

SSWR::AVIRead::AVIRASN1MIBForm::AVIRASN1MIBForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core) : UI::GUIForm(parent, 1024, 768, ui)
{
	this->SetFont(0, 8.25, false);
	this->SetText((const UTF8Char*)"ASN.1 MIB");

	this->core = core;
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
	NEW_CLASS(this->mib, Net::ASN1MIB());

	NEW_CLASS(this->pnlRequest, UI::GUIPanel(ui, this));
	this->pnlRequest->SetRect(0, 0, 100, 31, false);
	this->pnlRequest->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblFile, UI::GUILabel(ui, this->pnlRequest, (const UTF8Char*)"MIB File"));
	this->lblFile->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtFile, UI::GUITextBox(ui, this->pnlRequest, (const UTF8Char*)""));
	this->txtFile->SetRect(104, 4, 500, 23, false);
	this->txtFile->SetReadOnly(true);
	NEW_CLASS(this->btnBrowse, UI::GUIButton(ui, this->pnlRequest, (const UTF8Char*)"B&rowse"));
	this->btnBrowse->SetRect(604, 4, 75, 23, false);
	this->btnBrowse->HandleButtonClick(OnBrowseClicked, this);
	NEW_CLASS(this->tcMain, UI::GUITabControl(ui, this));
	this->tcMain->SetDockType(UI::GUIControl::DOCK_FILL);

	this->tpObjects = this->tcMain->AddTabPage((const UTF8Char*)"Objects");
	NEW_CLASS(this->lvObjectsVal, UI::GUIListView(ui, this->tpObjects, UI::GUIListView::LVSTYLE_TABLE, 2));
	this->lvObjectsVal->SetRect(0, 0, 100, 150, false);
	this->lvObjectsVal->SetDockType(UI::GUIControl::DOCK_BOTTOM);
	this->lvObjectsVal->SetShowGrid(true);
	this->lvObjectsVal->SetFullRowSelect(true);
	this->lvObjectsVal->AddColumn((const UTF8Char*)"Name", 200);
	this->lvObjectsVal->AddColumn((const UTF8Char*)"Value", 600);
	NEW_CLASS(this->vspObjects, UI::GUIVSplitter(ui, this->tpObjects, 3, true));
	NEW_CLASS(this->lvObjects, UI::GUIListView(ui, this->tpObjects, UI::GUIListView::LVSTYLE_TABLE, 4));
	this->lvObjects->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvObjects->SetShowGrid(true);
	this->lvObjects->SetFullRowSelect(true);
	this->lvObjects->AddColumn((const UTF8Char*)"Name", 200);
	this->lvObjects->AddColumn((const UTF8Char*)"OID", 200);
	this->lvObjects->AddColumn((const UTF8Char*)"Type", 200);
	this->lvObjects->AddColumn((const UTF8Char*)"Value", 200);
	this->lvObjects->HandleSelChg(OnObjectsSelChg, this);

	this->tpOID = this->tcMain->AddTabPage((const UTF8Char*)"OID");
	NEW_CLASS(this->lvOID, UI::GUIListView(ui, this->tpOID, UI::GUIListView::LVSTYLE_TABLE, 3));
	this->lvOID->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvOID->SetShowGrid(true);
	this->lvOID->SetFullRowSelect(true);
	this->lvOID->AddColumn((const UTF8Char*)"OID", 200);
	this->lvOID->AddColumn((const UTF8Char*)"DB Name", 200);
	this->lvOID->AddColumn((const UTF8Char*)"Name", 200);

	this->tpOIDText = this->tcMain->AddTabPage((const UTF8Char*)"OIDText");
	NEW_CLASS(this->txtOIDText, UI::GUITextBox(ui, this->tpOIDText, (const UTF8Char*)"", true));
	this->txtOIDText->SetDockType(UI::GUIControl::DOCK_FILL);
	this->txtOIDText->SetReadOnly(true);

	this->HandleDropFiles(OnFileDroped, this);
}

SSWR::AVIRead::AVIRASN1MIBForm::~AVIRASN1MIBForm()
{
	DEL_CLASS(this->mib);
}

void SSWR::AVIRead::AVIRASN1MIBForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}
