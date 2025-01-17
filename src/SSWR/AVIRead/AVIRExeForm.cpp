#include "Stdafx.h"
#include "Data/Sort/ArtificialQuickSort.h"
#include "IO/StmData/MemoryData2.h"
#include "Media/BitmapUtil.h"
#include "Media/ImageList.h"
#include "Parser/FileParser/FNTParser.h"
#include "SSWR/AVIRead/AVIRExeForm.h"
#include "Sync/Thread.h"
#include "Text/MyString.h"
#include "Text/StringBuilderUTF8.h"

void SSWR::AVIRead::AVIRExeForm::ParseSess16(Manage::DasmX86_16::DasmX86_16_Sess *sess, Data::ArrayListStrUTF8 *codes, Data::ArrayList<ExeB16Addr*> *parts, Data::ArrayListInt32 *partInd, ExeB16Addr *startAddr, Manage::DasmX86_16 *dasm, UOSInt codeSize)
{
	UTF8Char buff[512];
	UTF8Char *sptr;
	UOSInt buffSize;
	ExeB16Addr *eaddr;
	UOSInt i;
	UInt16 oriIP;
	while (true)
	{
		while (true)
		{
//			sptr = Text::StrHexVal32(buff, (sess->regs.CS << 4) + sess->regs.IP);
			sptr = Text::StrHexVal16(buff, sess->regs.CS);
			sptr = Text::StrConcat(sptr, (const UTF8Char*)":");
			sptr = Text::StrHexVal16(sptr, sess->regs.IP);
			sptr = Text::StrConcat(sptr, (const UTF8Char*)" ");
			oriIP = sess->regs.IP;
			if (oriIP == 0x229)
			{
				oriIP = 0x229;
			}
			Bool isSucc = dasm->DasmNext(sess, sptr, &buffSize);
			if (!isSucc)
			{
				UOSInt sizeLeft = codeSize - sess->regs.IP;
				if (sizeLeft > 16)
					sizeLeft = 16;
				Text::StrHexBytes(Text::StrConcat(&buff[::Text::StrCharCnt(buff)], (const UTF8Char*)"Unknown opcodes: "), &sess->code[oriIP], sizeLeft, ' ');
				codes->Add(Text::StrCopyNew(buff));
				break;
			}
			codes->Add(Text::StrCopyNew(buff));
			if (sess->endStatus != 0)
			{
				UInt32 nextAddr = sess->regs.CS;
				nextAddr = (nextAddr << 16) + sess->endIP;
				OSInt i = sess->jmpAddrs->SortedIndexOf(nextAddr);
				if (i < 0)
				{
					break;
				}
				else
				{
					sess->regs.IP = sess->endIP;
				}
			}
		}
		startAddr->endAddr = sess->endIP;

		if (sess->endStatus == 3)
			break;
		if (sess->endStatus == 2)
			break;
		if (sess->endStatus == 1)
		{
			if (sess->codeSegm != sess->regs.CS)
			{
				break;
			}
			Bool found = false;
			i = parts->GetCount();
			while (i-- > 0)
			{
				eaddr = parts->GetItem(i);
				if (sess->regs.IP >= eaddr->addr && sess->regs.IP < eaddr->endAddr)
				{
					found = true;
					break;
				}
			}
			if (found)
				break;
			NEW_CLASS(codes, Data::ArrayListStrUTF8());
			this->codesList->Add(codes);
			startAddr = MemAlloc(ExeB16Addr, 1);
			startAddr->segm = sess->regs.CS;
			startAddr->addr = sess->regs.IP;
			startAddr->endAddr = startAddr->addr;
			startAddr->codeList = codes;
//			this->lbParts->Items->Add(startAddr);
			parts->Insert(partInd->SortedInsert((startAddr->segm << 16) | startAddr->addr), startAddr);
		}
		else
		{
			break;
		}
	}
}

void SSWR::AVIRead::AVIRExeForm::InitSess16()
{
	Manage::DasmX86_16::DasmX86_16_Regs regs;
	Manage::DasmX86_16 *dasm;
	Manage::DasmX86_16::DasmX86_16_Sess *sess;
	Data::ArrayListStrUTF8 *codes;
	Data::ArrayList<ExeB16Addr*> *parts;
	Data::ArrayListInt32 *partInd;
	UTF8Char sbuff[32];
	ExeB16Addr *eaddr;
	UOSInt codeSize;
	UOSInt i;
	UOSInt j;
	Data::ArrayListUInt32 *funcCalls;
	Data::ArrayListUInt32 *nfuncCalls;

	this->exeFile->GetDOSInitRegs(&regs);
	NEW_CLASS(parts, Data::ArrayList<ExeB16Addr*>());
	NEW_CLASS(partInd, Data::ArrayListInt32());
	NEW_CLASS(this->codesList, Data::ArrayList<Data::ArrayListStrUTF8*>());
	NEW_CLASS(codes, Data::ArrayListStrUTF8());
	this->codesList->Add(codes);
	eaddr = MemAlloc(ExeB16Addr, 1);
	eaddr->segm = regs.CS;
	eaddr->addr = eaddr->endAddr = regs.IP;
	eaddr->codeList = codes;
	//this->lbParts->Items->Add(eaddr);
	
	parts->Insert(partInd->SortedInsert((eaddr->segm << 16) | eaddr->addr), eaddr);
	
	NEW_CLASS(dasm, Manage::DasmX86_16());
	NEW_CLASS(funcCalls, ::Data::ArrayListUInt32());
	NEW_CLASS(nfuncCalls, ::Data::ArrayListUInt32());
	sess = dasm->CreateSess(&regs, this->exeFile->GetDOSCodePtr(&codeSize), this->exeFile->GetDOSCodeSegm());
	if (sess)
	{
		this->ParseSess16(sess, codes, parts, partInd, eaddr, dasm, codeSize);
		nfuncCalls->AddRange(sess->callAddrs);
		nfuncCalls->AddRange(sess->jmpAddrs);
		UOSInt arrSize;
		UInt32 *tmpArr = nfuncCalls->GetArray(&arrSize);
		ArtificialQuickSort_SortUInt32(tmpArr, 0, (OSInt)arrSize - 1);
		dasm->DeleteSess(sess);

		while (nfuncCalls->GetCount() > 0)
		{
			UInt32 faddr = nfuncCalls->GetItem(0);
			nfuncCalls->RemoveAt(0);
			OSInt si = funcCalls->SortedIndexOf(faddr);
			if (si < 0)
			{
				funcCalls->Insert((UOSInt)-si - 1, faddr);
				sess = dasm->CreateSess(&regs, this->exeFile->GetDOSCodePtr(&codeSize), this->exeFile->GetDOSCodeSegm());
				sess->regs.IP = (::UInt16)faddr;
				NEW_CLASS(codes, Data::ArrayListStrUTF8());
				this->codesList->Add(codes);
				eaddr = MemAlloc(ExeB16Addr, 1);
				eaddr->segm = sess->regs.CS;
				eaddr->addr = eaddr->endAddr = sess->regs.IP;
				eaddr->codeList = codes;
//				this->lbParts->Items->Add(eaddr);
				parts->Insert(partInd->SortedInsert((eaddr->segm << 16) | eaddr->addr), eaddr);
                
				this->ParseSess16(sess, codes, parts, partInd, eaddr, dasm, codeSize);
				nfuncCalls->AddRange(sess->callAddrs);
				nfuncCalls->AddRange(sess->jmpAddrs);
				tmpArr = nfuncCalls->GetArray(&arrSize);
				ArtificialQuickSort_SortUInt32(tmpArr, 0, (OSInt)arrSize - 1);
				dasm->DeleteSess(sess);
			}
		}
	}
	DEL_CLASS(funcCalls);
	DEL_CLASS(nfuncCalls);
	DEL_CLASS(dasm);
	DEL_CLASS(partInd);
	this->parts = parts;

	ExeB16Addr *lastAddr = 0;
	i = 0;
	j = parts->GetCount();
	while (i < j)
	{
		eaddr = parts->GetItem(i);
		if (lastAddr && lastAddr->segm == eaddr->segm && lastAddr->addr < eaddr->addr && lastAddr->endAddr >= eaddr->endAddr)
		{
		}
		else
		{
			Text::StrHexVal16(Text::StrConcat(Text::StrHexVal16(sbuff, eaddr->segm), (const UTF8Char*)":"), eaddr->addr);
			this->lb16BitFuncs->AddItem(sbuff, eaddr);
			lastAddr = eaddr;
		}
		i++;
	}
}

void __stdcall SSWR::AVIRead::AVIRExeForm::On16BitFuncsChg(void *userObj)
{
	SSWR::AVIRead::AVIRExeForm *me = (SSWR::AVIRead::AVIRExeForm*)userObj;
	ExeB16Addr *addr = (ExeB16Addr*)me->lb16BitFuncs->GetSelectedItem();
	if (addr)
	{
		UOSInt i;
		UOSInt j;
		me->lb16BitCont->ClearItems();
		i = 0;
		j = addr->codeList->GetCount();
		while (i < j)
		{
			me->lb16BitCont->AddItem(addr->codeList->GetItem(i), 0);
			i++;
		}
	}
	else
	{
		me->lb16BitCont->ClearItems();
	}
}

void __stdcall SSWR::AVIRead::AVIRExeForm::OnImportSelChg(void *userObj)
{
	SSWR::AVIRead::AVIRExeForm *me = (SSWR::AVIRead::AVIRExeForm*)userObj;
	UOSInt modIndex = (UOSInt)me->lbImport->GetSelectedItem();
	me->lvImport->ClearItems();
	UOSInt i;
	UOSInt j;
	i = 0;
	j = me->exeFile->GetImportFuncCount(modIndex);
	while (i < j)
	{
		me->lvImport->AddItem(me->exeFile->GetImportFunc(modIndex, i), 0);
		i++;
	}
}

void __stdcall SSWR::AVIRead::AVIRExeForm::OnResourceSelChg(void *userObj)
{
	SSWR::AVIRead::AVIRExeForm *me = (SSWR::AVIRead::AVIRExeForm*)userObj;
	const IO::EXEFile::ResourceInfo *res = (const IO::EXEFile::ResourceInfo*)me->lbResource->GetSelectedItem();
	if (res)
	{
		Text::StringBuilderUTF8 sb;
		sb.Append((const UTF8Char*)"Size = ");
		sb.AppendUOSInt(res->dataSize);
		sb.Append((const UTF8Char*)"\r\nCodePage = ");
		sb.AppendU32(res->codePage);
		sb.Append((const UTF8Char*)"\r\nType = ");
		sb.Append(IO::EXEFile::GetResourceTypeName(res->rt));
		sb.Append((const UTF8Char*)"\r\n");
		IO::EXEFile::GetResourceDesc(res, &sb);
		me->txtResource->SetText(sb.ToString());
	}
	else
	{
		me->txtResource->SetText((const UTF8Char*)"");
	}
}

void __stdcall SSWR::AVIRead::AVIRExeForm::OnResourceDblClk(void *userObj)
{
	SSWR::AVIRead::AVIRExeForm *me = (SSWR::AVIRead::AVIRExeForm*)userObj;
	const IO::EXEFile::ResourceInfo *res = (const IO::EXEFile::ResourceInfo*)me->lbResource->GetSelectedItem();
	if (res)
	{
		if (res->rt == IO::EXEFile::RT_BITMAP)
		{
			Media::StaticImage *simg = Media::BitmapUtil::ParseDIBBuffer(res->data, res->dataSize);
			if (simg)
			{
				Media::ImageList *imgList;
				NEW_CLASS(imgList, Media::ImageList(res->name));
				imgList->AddImage(simg, 0);
				me->core->OpenObject(imgList);
			}
		}
		else if (res->rt == IO::EXEFile::RT_ICON || res->rt == IO::EXEFile::RT_CURSOR)
		{
			IO::StmData::MemoryData2 *fd;
			NEW_CLASS(fd, IO::StmData::MemoryData2(res->data, res->dataSize));
			me->core->LoadData(fd, 0);
			DEL_CLASS(fd);
		}
		else if (res->rt == IO::EXEFile::RT_FONT)
		{
			Media::FontRenderer *font;
			font = Parser::FileParser::FNTParser::ParseFontBuff(me->exeFile->GetSourceNameObj(), res->data, res->dataSize);
			if (font)
			{
				me->core->OpenObject(font);
			}
		}
	}
}

SSWR::AVIRead::AVIRExeForm::AVIRExeForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core, IO::EXEFile *exeFile) : UI::GUIForm(parent, 1024, 768, ui)
{
	UTF8Char sbuff[512];
	this->SetFont(0, 8.25, false);
	this->exeFile = exeFile;
	this->core = core;
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
	Text::StrConcat(Text::StrConcat(sbuff, (const UTF8Char*)"EXE Form - "), exeFile->GetSourceNameObj());
	this->SetText(sbuff);

	NEW_CLASS(this->tcEXE, UI::GUITabControl(ui, this));
	this->tcEXE->SetDockType(UI::GUIControl::DOCK_FILL);

	this->tpProp = this->tcEXE->AddTabPage((const UTF8Char*)"Properties");
	NEW_CLASS(this->lvProp, UI::GUIListView(ui, this->tpProp, UI::GUIListView::LVSTYLE_TABLE, 2));
	this->lvProp->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvProp->SetShowGrid(true);
	this->lvProp->SetFullRowSelect(true);
	this->lvProp->AddColumn((const UTF8Char*)"Name", 250);
	this->lvProp->AddColumn((const UTF8Char*)"Value", 250);

	UOSInt i;
	UOSInt j;
	UOSInt k;
	const UTF8Char *csptr;
	i = 0;
	j = this->exeFile->GetPropCount();
	while (i < j)
	{
		csptr = this->exeFile->GetPropName(i);
		k = this->lvProp->AddItem(csptr, 0);
		this->lvProp->SetSubItem(k, 1, this->exeFile->GetPropValue(i));

		i++;
	}

	this->tpImport = this->tcEXE->AddTabPage((const UTF8Char*)"Import");
	NEW_CLASS(this->lbImport, UI::GUIListBox(ui, this->tpImport, false));
	this->lbImport->SetRect(0, 0, 150, 23, false);
	this->lbImport->SetDockType(UI::GUIControl::DOCK_LEFT);
	this->lbImport->HandleSelectionChange(OnImportSelChg, this);
	NEW_CLASS(this->hspImport, UI::GUIHSplitter(ui, this->tpImport, 3, false));
	NEW_CLASS(this->lvImport, UI::GUIListView(ui, this->tpImport, UI::GUIListView::LVSTYLE_TABLE, 1));
	this->lvImport->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lvImport->SetShowGrid(true);
	this->lvImport->SetFullRowSelect(true);
	this->lvImport->AddColumn((const UTF8Char*)"Func Name", 200);
	i = 0;
	j = this->exeFile->GetImportCount();
	while (i < j)
	{
		this->lbImport->AddItem(this->exeFile->GetImportName(i), (void*)i);
		i++;
	}

	this->tpExport = this->tcEXE->AddTabPage((const UTF8Char*)"Export");
	NEW_CLASS(this->lbExport, UI::GUIListBox(ui, this->tpExport, false));
	this->lbExport->SetDockType(UI::GUIControl::DOCK_FILL);
	i = 0;
	j = this->exeFile->GetExportCount();
	while (i < j)
	{
		this->lbExport->AddItem(this->exeFile->GetExportName(i), (void*)i);
		i++;
	}

	this->tpResource = this->tcEXE->AddTabPage((const UTF8Char*)"Resource");
	NEW_CLASS(this->lbResource, UI::GUIListBox(ui, this->tpResource, false));
	this->lbResource->SetRect(0, 0, 200, 100, false);
	this->lbResource->SetDockType(UI::GUIControl::DOCK_LEFT);
	this->lbResource->HandleSelectionChange(OnResourceSelChg, this);
	this->lbResource->HandleDoubleClicked(OnResourceDblClk, this);
	NEW_CLASS(this->hspResource, UI::GUIHSplitter(ui, this->tpResource, 3, false));
	NEW_CLASS(this->txtResource, UI::GUITextBox(ui, this->tpResource, (const UTF8Char*)"", true));
	this->txtResource->SetReadOnly(true);
	this->txtResource->SetDockType(UI::GUIControl::DOCK_FILL);
	i = 0;
	j = this->exeFile->GetResourceCount();
	while (i < j)
	{
		const IO::EXEFile::ResourceInfo *res;
		res = this->exeFile->GetResource(i);
		this->lbResource->AddItem(res->name, (void*)res);
		i++;
	}

	if (this->exeFile->HasDOS())
	{
		this->tp16Bit = this->tcEXE->AddTabPage((const UTF8Char*)"16 Bit");
		NEW_CLASS(this->pnl16BitInfo, UI::GUIPanel(ui, this->tp16Bit));
		this->pnl16BitInfo->SetRect(0, 0, 100, 40, false);
		this->pnl16BitInfo->SetDockType(UI::GUIControl::DOCK_TOP);
		NEW_CLASS(this->lb16BitFuncs, UI::GUIListBox(ui, this->tp16Bit, false));
		this->lb16BitFuncs->SetRect(0, 0, 120, 208, false);
		this->lb16BitFuncs->SetDockType(UI::GUIControl::DOCK_LEFT);
		this->lb16BitFuncs->HandleSelectionChange(On16BitFuncsChg, this);
		NEW_CLASS(this->hsp16Bit, UI::GUIHSplitter(ui, this->tp16Bit, 3, false));
		NEW_CLASS(this->lb16BitCont, UI::GUIListBox(ui, this->tp16Bit, false));
		this->lb16BitCont->SetDockType(UI::GUIControl::DOCK_FILL);

		this->InitSess16();
	}
}

SSWR::AVIRead::AVIRExeForm::~AVIRExeForm()
{
	UOSInt j;
	UOSInt i;
	DEL_CLASS(this->exeFile);
	if (this->parts)
	{
		ExeB16Addr *addr;
		i = this->parts->GetCount();
		while (i-- > 0)
		{
			addr = this->parts->GetItem(i);
			MemFree(addr);
		}
		DEL_CLASS(this->parts);
	}
	if (this->codesList)
	{
		Data::ArrayListStrUTF8 *codes;
		i = this->codesList->GetCount();
		while (i-- > 0)
		{
			codes = this->codesList->GetItem(i);
			j = codes->GetCount();
			while (j-- > 0)
			{
				Text::StrDelNew(codes->GetItem(j));
			}
			DEL_CLASS(codes);
		}
		DEL_CLASS(this->codesList);
	}
}

void SSWR::AVIRead::AVIRExeForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}
