#include "Stdafx.h"
#include "IO/MODBUSRTUMaster.h"
#include "IO/MODBUSTCPMaster.h"
#include "IO/SerialPort.h"
#include "IO/Device/AMGU4241.h"
#include "IO/Device/SDM120M.h"
#include "SSWR/AVIRead/AVIRMODBUSMasterForm.h"
#include "Sync/Thread.h"
#include "Text/MyStringFloat.h"

void __stdcall SSWR::AVIRead::AVIRMODBUSMasterForm::OnStreamClicked(void *userObj)
{
	SSWR::AVIRead::AVIRMODBUSMasterForm *me = (SSWR::AVIRead::AVIRMODBUSMasterForm *)userObj;
	if (me->stm)
	{
		me->StopStream();
	}
	else
	{
		SSWR::AVIRead::AVIRCore::StreamType st;
		me->stm = me->core->OpenStream(&st, me, 0, false);
		if (me->stm)
		{
			if (me->radMODBUSTCP->IsSelected())
			{
				NEW_CLASS(me->modbus, IO::MODBUSTCPMaster(me->stm));
			}
			else
			{
				NEW_CLASS(me->modbus, IO::MODBUSRTUMaster(me->stm));
			}
			NEW_CLASS(me->modbusCtrl, IO::MODBUSController(me->modbus));
			me->txtStream->SetText(SSWR::AVIRead::AVIRCore::GetStreamTypeName(st));
			me->btnStream->SetText((const UTF8Char*)"&Close");
		}
	}
}

void __stdcall SSWR::AVIRead::AVIRMODBUSMasterForm::OnI32GetClicked(void *userObj)
{
	SSWR::AVIRead::AVIRMODBUSMasterForm *me = (SSWR::AVIRead::AVIRMODBUSMasterForm *)userObj;
	if (me->stm)
	{
		Text::StringBuilderUTF8 sb;
		UInt8 devAddr;
		UInt32 regAddr;
		Int32 val;
		me->txtI32DevAddr->GetText(&sb);
		if (!Text::StrToUInt8(sb.ToString(), &devAddr))
		{
			me->txtI32Value->SetText((const UTF8Char*)"DevAddr");
		}
		sb.ClearStr();
		me->txtI32RegAddr->GetText(&sb);
		if (!Text::StrToUInt32(sb.ToString(), &regAddr))
		{
			me->txtI32Value->SetText((const UTF8Char*)"RegAddr");
		}
		if (me->modbusCtrl->ReadRegisterI32(devAddr, regAddr, &val))
		{
			sb.ClearStr();
			sb.AppendI32(val);
			me->txtI32Value->SetText(sb.ToString());
		}
		else
		{
			me->txtI32Value->SetText((const UTF8Char*)"-");
		}
	}
}

void __stdcall SSWR::AVIRead::AVIRMODBUSMasterForm::OnF32GetClicked(void *userObj)
{
	SSWR::AVIRead::AVIRMODBUSMasterForm *me = (SSWR::AVIRead::AVIRMODBUSMasterForm *)userObj;
	if (me->stm)
	{
		Text::StringBuilderUTF8 sb;
		UInt8 devAddr;
		UInt32 regAddr;
		Single val;
		me->txtF32DevAddr->GetText(&sb);
		if (!Text::StrToUInt8(sb.ToString(), &devAddr))
		{
			me->txtF32Value->SetText((const UTF8Char*)"DevAddr");
		}
		sb.ClearStr();
		me->txtF32RegAddr->GetText(&sb);
		if (!Text::StrToUInt32(sb.ToString(), &regAddr))
		{
			me->txtF32Value->SetText((const UTF8Char*)"RegAddr");
		}
		if (me->modbusCtrl->ReadRegisterF32(devAddr, regAddr, &val))
		{
			sb.ClearStr();
			Text::SBAppendF32(&sb, val);
			me->txtF32Value->SetText(sb.ToString());
		}
		else
		{
			me->txtF32Value->SetText((const UTF8Char*)"-");
		}
	}
}

void __stdcall SSWR::AVIRead::AVIRMODBUSMasterForm::OnDeviceAddClicked(void *userObj)
{
	SSWR::AVIRead::AVIRMODBUSMasterForm *me = (SSWR::AVIRead::AVIRMODBUSMasterForm *)userObj;
	if (me->stm)
	{
		UInt8 devAddr;
		Text::StringBuilderUTF8 sb;
		me->txtDeviceAddr->GetText(&sb);
		if (!Text::StrToUInt8(sb.ToString(), &devAddr))
		{
			return;
		}
		DeviceType dt = (DeviceType)(OSInt)me->cboDevice->GetSelectedItem();
		switch (dt)
		{
		case DT_SDM120:
			IO::Device::SDM120M::GetDataEntries(devAddr, OnMODBUSEntry, me);
			break;
		case DT_AMGU4241:
			IO::Device::AMGU4241::GetDataEntries(devAddr, OnMODBUSEntry, me);
			break;
		}
	}
}

void __stdcall SSWR::AVIRead::AVIRMODBUSMasterForm::OnTimerTick(void *userObj)
{
	SSWR::AVIRead::AVIRMODBUSMasterForm *me = (SSWR::AVIRead::AVIRMODBUSMasterForm *)userObj;
	if (me->stm)
	{
		UTF8Char sbuff[64];
		UTF8Char *sptr;
		MODBUSEntry *entry;
		UOSInt i = 0;
		UOSInt j = me->entryList->GetCount();
		while (i < j)
		{
			Single f32Val;
			Int32 i32Val;
			UInt16 u16Val;
			entry = me->entryList->GetItem(i);
			switch (entry->dt)
			{
			case IO::MODBUSController::DT_F32:
				if (me->modbusCtrl->ReadRegisterF32(entry->devAddr, entry->regAddr, &f32Val))
				{
					sptr = Text::StrDouble(sbuff, f32Val / (Double)entry->denorm);
				}
				else
				{
					sptr = Text::StrConcat(sbuff, (const UTF8Char*)"-");
				}
				break;
			case IO::MODBUSController::DT_I32:
				if (me->modbusCtrl->ReadRegisterI32(entry->devAddr, entry->regAddr, &i32Val))
				{
					if (entry->denorm == 1)
					{
						sptr = Text::StrInt32(sbuff, i32Val);
					}
					else if (entry->denorm == 0)
					{
						sptr = Text::StrHexVal32(sbuff, (UInt32)i32Val);
					}
					else
					{
						sptr = Text::StrDouble(sbuff, i32Val / (Double)entry->denorm);
					}
				}
				else
				{
					sptr = Text::StrConcat(sbuff, (const UTF8Char*)"-");
				}
				break;
			case IO::MODBUSController::DT_U16:
				if (me->modbusCtrl->ReadRegisterU16(entry->devAddr, entry->regAddr, &u16Val))
				{
					if (entry->denorm == 1)
					{
						sptr = Text::StrUInt16(sbuff, u16Val);
					}
					else if (entry->denorm == 0)
					{
						sptr = Text::StrHexVal16(sbuff, u16Val);
					}
					else
					{
						sptr = Text::StrDouble(sbuff, u16Val / (Double)entry->denorm);
					}
				}
				else
				{
					sptr = Text::StrConcat(sbuff, (const UTF8Char*)"-");
				}
				break;
			case IO::MODBUSController::DT_IU32:
				if (me->modbusCtrl->ReadRegisterII32(entry->devAddr, entry->regAddr, &i32Val))
				{
					if (entry->denorm == 1)
					{
						sptr = Text::StrUInt32(sbuff, (UInt32)i32Val);
					}
					else if (entry->denorm == 0)
					{
						sptr = Text::StrHexVal32(sbuff, (UInt32)i32Val);
					}
					else
					{
						sptr = Text::StrDouble(sbuff, ((UInt32)i32Val) / (Double)entry->denorm);
					}
				}
				else
				{
					sptr = Text::StrConcat(sbuff, (const UTF8Char*)"-");
				}
				break;
			case IO::MODBUSController::DT_IU16:
				if (me->modbusCtrl->ReadRegisterIU16(entry->devAddr, entry->regAddr, &u16Val))
				{
					if (entry->denorm == 1)
					{
						sptr = Text::StrUInt16(sbuff, u16Val);
					}
					else if (entry->denorm == 0)
					{
						sptr = Text::StrHexVal16(sbuff, u16Val);
					}
					else
					{
						sptr = Text::StrDouble(sbuff, u16Val / (Double)entry->denorm);
					}
				}
				else
				{
					sptr = Text::StrConcat(sbuff, (const UTF8Char*)"-");
				}
				break;
			case IO::MODBUSController::DT_OS16:
				if (me->modbusCtrl->ReadRegisterU16(entry->devAddr, entry->regAddr, &u16Val))
				{
					if (u16Val & 0x8000)
					{
						i32Val = -(u16Val & 0x7fff);
					}
					else
					{
						i32Val = u16Val;
					}
					if (entry->denorm == 1)
					{
						sptr = Text::StrInt32(sbuff, i32Val);
					}
					else if (entry->denorm == 0)
					{
						sptr = Text::StrHexVal16(sbuff, (UInt16)i32Val);
					}
					else
					{
						sptr = Text::StrDouble(sbuff, i32Val / (Double)entry->denorm);
					}
				}
				else
				{
					sptr = Text::StrConcat(sbuff, (const UTF8Char*)"-");
				}
				break;
			default:
				sptr = Text::StrConcat(sbuff, (const UTF8Char*)"-");
				break;
			}
			Text::StrConcat(sptr, Math::Unit::UnitBase::GetUnitShortName(entry->vt, entry->unit));

			me->lvDevice->SetSubItem(entry->lvIndex, 3, sbuff);
			i++;
		}
	}
}

void __stdcall SSWR::AVIRead::AVIRMODBUSMasterForm::OnMODBUSEntry(void *userObj, const UTF8Char *name, UInt8 devAddr, UInt32 regAddr, IO::MODBUSController::DataType dt, Math::Unit::UnitBase::ValueType vt, Int32 unit, Int32 denorm)
{
	SSWR::AVIRead::AVIRMODBUSMasterForm *me = (SSWR::AVIRead::AVIRMODBUSMasterForm *)userObj;
	UTF8Char sbuff[32];
	MODBUSEntry *entry = MemAlloc(MODBUSEntry, 1);
	entry->name = Text::StrCopyNew(name);
	entry->devAddr = devAddr;
	entry->regAddr = regAddr;
	entry->dt = dt;
	entry->vt = vt;
	entry->unit = unit;
	entry->denorm = denorm;
	Text::StrUInt16(sbuff, devAddr);
	entry->lvIndex = me->lvDevice->AddItem(sbuff, entry);
	entry->val = 0;
	me->entryList->Add(entry);
	Text::StrUInt16(sbuff, (UInt16)regAddr);
	me->lvDevice->SetSubItem(entry->lvIndex, 1, sbuff);
	me->lvDevice->SetSubItem(entry->lvIndex, 2, name);
	me->lvDevice->SetSubItem(entry->lvIndex, 3, (const UTF8Char*)"-");
}

void SSWR::AVIRead::AVIRMODBUSMasterForm::StopStream()
{
	if (this->stm)
	{
		DEL_CLASS(this->modbusCtrl);
		DEL_CLASS(this->modbus);
		DEL_CLASS(this->stm);
		this->stm = 0;
		this->txtStream->SetText((const UTF8Char*)"-");
		this->btnStream->SetText((const UTF8Char*)"&Open");
	}
}

SSWR::AVIRead::AVIRMODBUSMasterForm::AVIRMODBUSMasterForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core) : UI::GUIForm(parent, 576, 480, ui)
{
	this->SetText((const UTF8Char*)"MODBUS Master");
	this->SetFont(0, 8.25, false);
	
	this->core = core;
	this->stm = 0;
	NEW_CLASS(this->entryList, Data::ArrayList<MODBUSEntry*>());
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));

	NEW_CLASS(this->grpStream, UI::GUIGroupBox(ui, this, (const UTF8Char*)"Stream"));
	this->grpStream->SetRect(0, 0, 100, 72, false);
	this->grpStream->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblStream, UI::GUILabel(ui, this->grpStream, (const UTF8Char*)"Stream Type"));
	this->lblStream->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtStream, UI::GUITextBox(ui, this->grpStream, (const UTF8Char*)"-"));
	this->txtStream->SetRect(104, 4, 200, 23, false);
	this->txtStream->SetReadOnly(true);
	NEW_CLASS(this->radMODBUSRTU, UI::GUIRadioButton(ui, this->grpStream, (const UTF8Char*)"MODBUS RTU", true));
	this->radMODBUSRTU->SetRect(104, 28, 100, 23, false);
	NEW_CLASS(this->radMODBUSTCP, UI::GUIRadioButton(ui, this->grpStream, (const UTF8Char*)"MODBUS TCP", false));
	this->radMODBUSTCP->SetRect(204, 28, 100, 23, false);
	NEW_CLASS(this->btnStream, UI::GUIButton(ui, this->grpStream, (const UTF8Char*)"&Open"));
	this->btnStream->SetRect(304, 4, 75, 23, false);
	this->btnStream->HandleButtonClick(OnStreamClicked, this);
	NEW_CLASS(this->tcMain, UI::GUITabControl(ui, this));
	this->tcMain->SetDockType(UI::GUIControl::DOCK_FILL);
	
	this->tpGetValue = this->tcMain->AddTabPage((const UTF8Char*)"GetValue");
	NEW_CLASS(this->lblDevAddr, UI::GUILabel(ui, this->tpGetValue, (const UTF8Char*)"Dev Addr"));
	this->lblDevAddr->SetRect(104, 4, 100, 23, false);
	NEW_CLASS(this->lblRegAddr, UI::GUILabel(ui, this->tpGetValue, (const UTF8Char*)"Reg Addr"));
	this->lblRegAddr->SetRect(204, 4, 100, 23, false);
	NEW_CLASS(this->lblValue, UI::GUILabel(ui, this->tpGetValue, (const UTF8Char*)"Value"));
	this->lblValue->SetRect(384, 4, 100, 23, false);
	NEW_CLASS(this->lblI32Name, UI::GUILabel(ui, this->tpGetValue, (const UTF8Char*)"I32"));
	this->lblI32Name->SetRect(4, 28, 100, 23, false);
	NEW_CLASS(this->txtI32DevAddr, UI::GUITextBox(ui, this->tpGetValue, (const UTF8Char*)"1"));
	this->txtI32DevAddr->SetRect(104, 28, 100, 23, false);
	NEW_CLASS(this->txtI32RegAddr, UI::GUITextBox(ui, this->tpGetValue, (const UTF8Char*)"30001"));
	this->txtI32RegAddr->SetRect(204, 28, 100, 23, false);
	NEW_CLASS(this->btnI32Get, UI::GUIButton(ui, this->tpGetValue, (const UTF8Char*)"Get"));
	this->btnI32Get->SetRect(304, 28, 75, 23, false);
	this->btnI32Get->HandleButtonClick(OnI32GetClicked, this);
	NEW_CLASS(this->txtI32Value, UI::GUITextBox(ui, this->tpGetValue, (const UTF8Char*)""));
	this->txtI32Value->SetRect(384, 28, 200, 23, false);
	this->txtI32Value->SetReadOnly(true);
	NEW_CLASS(this->lblF32Name, UI::GUILabel(ui, this->tpGetValue, (const UTF8Char*)"F32"));
	this->lblF32Name->SetRect(4, 52, 100, 23, false);
	NEW_CLASS(this->txtF32DevAddr, UI::GUITextBox(ui, this->tpGetValue, (const UTF8Char*)"1"));
	this->txtF32DevAddr->SetRect(104, 52, 100, 23, false);
	NEW_CLASS(this->txtF32RegAddr, UI::GUITextBox(ui, this->tpGetValue, (const UTF8Char*)"30001"));
	this->txtF32RegAddr->SetRect(204, 52, 100, 23, false);
	NEW_CLASS(this->btnF32Get, UI::GUIButton(ui, this->tpGetValue, (const UTF8Char*)"Get"));
	this->btnF32Get->SetRect(304, 52, 75, 23, false);
	this->btnF32Get->HandleButtonClick(OnF32GetClicked, this);
	NEW_CLASS(this->txtF32Value, UI::GUITextBox(ui, this->tpGetValue, (const UTF8Char*)""));
	this->txtF32Value->SetRect(384, 52, 200, 23, false);
	this->txtF32Value->SetReadOnly(true);

	this->tpDevice = this->tcMain->AddTabPage((const UTF8Char*)"Device");
	NEW_CLASS(this->pnlDevice, UI::GUIPanel(ui, this->tpDevice));
	this->pnlDevice->SetRect(0, 0, 100, 31, false);
	this->pnlDevice->SetDockType(UI::GUIControl::DOCK_TOP);
	NEW_CLASS(this->lblDeviceAddr, UI::GUILabel(ui, this->pnlDevice, (const UTF8Char*)"Addr"));
	this->lblDeviceAddr->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtDeviceAddr, UI::GUITextBox(ui, this->pnlDevice, (const UTF8Char*)"1"));
	this->txtDeviceAddr->SetRect(104, 4, 100, 23, false);
	NEW_CLASS(this->lblDevice, UI::GUILabel(ui, this->pnlDevice, (const UTF8Char*)"Device"));
	this->lblDevice->SetRect(204, 4, 100, 23, false);
	NEW_CLASS(this->cboDevice, UI::GUIComboBox(ui, this->pnlDevice, false));
	this->cboDevice->SetRect(304, 4, 100, 23, false);
	DeviceType dt = DT_FIRST;
	while (dt <= DT_LAST)
	{
		this->cboDevice->AddItem(DeviceTypeGetName(dt), (void*)(OSInt)dt);
		dt = (DeviceType)(dt + 1);
	}
	this->cboDevice->SetSelectedIndex(0);
	NEW_CLASS(this->btnDeviceAdd, UI::GUIButton(ui, this->pnlDevice, (const UTF8Char*)"Add"));
	this->btnDeviceAdd->SetRect(404, 4, 75, 23, false);
	this->btnDeviceAdd->HandleButtonClick(OnDeviceAddClicked, this);
	NEW_CLASS(this->lvDevice, UI::GUIListView(ui, this->tpDevice, UI::GUIListView::LVSTYLE_TABLE, 4));
	this->lvDevice->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvDevice->SetFullRowSelect(true);
	this->lvDevice->SetShowGrid(true);
	this->lvDevice->AddColumn((const UTF8Char*)"DevAddr", 60);
	this->lvDevice->AddColumn((const UTF8Char*)"RegAddr", 60);
	this->lvDevice->AddColumn((const UTF8Char*)"Name", 150);
	this->lvDevice->AddColumn((const UTF8Char*)"Value", 150);

	this->AddTimer(10000, OnTimerTick, this);
}

SSWR::AVIRead::AVIRMODBUSMasterForm::~AVIRMODBUSMasterForm()
{
	StopStream();
	MODBUSEntry *entry;
	UOSInt i = this->entryList->GetCount();
	while (i-- > 0)
	{
		entry = this->entryList->GetItem(i);
		Text::StrDelNew(entry->name);
		MemFree(entry);
	}
	DEL_CLASS(this->entryList);
}

void SSWR::AVIRead::AVIRMODBUSMasterForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}

const UTF8Char *SSWR::AVIRead::AVIRMODBUSMasterForm::DeviceTypeGetName(DeviceType dt)
{
	switch (dt)
	{
	case DT_SDM120:
		return (const UTF8Char*)"SDM120";
	case DT_AMGU4241:
		return (const UTF8Char*)"AMGU4241";
	default:
		return (const UTF8Char*)"Unknown";
	}
}
