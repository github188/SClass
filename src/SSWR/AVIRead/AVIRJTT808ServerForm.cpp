#include "Stdafx.h"
#include "Data/ByteTool.h"
#include "SSWR/AVIRead/AVIRJTT808ServerForm.h"
#include "Sync/Thread.h"
#include "Text/MyStringFloat.h"
#include "UI/MessageDialog.h"

typedef struct
{
	void *cliData;
	UOSInt buffSize;
	UInt16 seqId;
	UInt8 recvBuff[4096];
} ClientData;

void __stdcall SSWR::AVIRead::AVIRJTT808ServerForm::OnStartClicked(void *userObj)
{
	SSWR::AVIRead::AVIRJTT808ServerForm *me = (SSWR::AVIRead::AVIRJTT808ServerForm*)userObj;
	if (me->svr)
	{
		me->ServerStop();
		me->txtPort->SetReadOnly(false);
	}
	else
	{
		Text::StringBuilderUTF8 sb;
		UInt16 port;
		me->txtPort->GetText(&sb);
		if (!sb.ToUInt16(&port))
		{
			UI::MessageDialog::ShowDialog((const UTF8Char*)"Port is not valid", (const UTF8Char*)"Error", me);
		}
		else if (port <= 0 || port >= 65536)
		{
			UI::MessageDialog::ShowDialog((const UTF8Char*)"Port is out of range", (const UTF8Char*)"Error", me);
		}
		else
		{
			NEW_CLASS(me->cliMgr, Net::TCPClientMgr(240, OnClientEvent, OnClientData, me, Sync::Thread::GetThreadCnt(), OnClientTimeout));
			NEW_CLASS(me->svr, Net::TCPServer(me->core->GetSocketFactory(), port, me->log, OnClientConn, me, (const UTF8Char*)"TCP: "));
			if (me->svr->IsV4Error())
			{
				UI::MessageDialog::ShowDialog((const UTF8Char*)"Error in starting server", (const UTF8Char*)"Error", me);
			}
			else
			{
				me->txtPort->SetReadOnly(true);
			}
		}
	}
}

void __stdcall SSWR::AVIRead::AVIRJTT808ServerForm::OnLogSelChg(void *userObj)
{
	SSWR::AVIRead::AVIRJTT808ServerForm *me = (SSWR::AVIRead::AVIRJTT808ServerForm*)userObj;
	const UTF8Char *csptr = me->lbLog->GetSelectedItemTextNew();
	if (csptr)
	{
		me->txtLog->SetText(csptr);
		me->lbLog->DelTextNew(csptr);
	}
}

void __stdcall SSWR::AVIRead::AVIRJTT808ServerForm::OnClientEvent(Net::TCPClient *cli, void *userObj, void *cliData, Net::TCPClientMgr::TCPEventType evtType)
{
	SSWR::AVIRead::AVIRJTT808ServerForm *me = (SSWR::AVIRead::AVIRJTT808ServerForm*)userObj;
	ClientData *data = (ClientData*)cliData;
	if (evtType == Net::TCPClientMgr::TCP_EVENT_DISCONNECT)
	{
		UTF8Char sbuff[256];
		UTF8Char *sptr;
		sptr = Text::StrConcat(sbuff, (const UTF8Char*)"Client ");
		sptr = cli->GetRemoteName(sptr);
		sptr = Text::StrConcat(sptr, (const UTF8Char*)" disconnect");
		me->log->LogMessage(sbuff, IO::ILogHandler::LOG_LEVEL_ACTION);

		me->protoHdlr->DeleteStreamData(cli, data->cliData);
		MemFree(data);
		DEL_CLASS(cli);
	}
}

void __stdcall SSWR::AVIRead::AVIRJTT808ServerForm::OnClientData(Net::TCPClient *cli, void *userObj, void *cliData, const UInt8 *buff, UOSInt size)
{
	SSWR::AVIRead::AVIRJTT808ServerForm *me = (SSWR::AVIRead::AVIRJTT808ServerForm*)userObj;
	ClientData *data = (ClientData*)cliData;

	UTF8Char sbuff[256];
	UTF8Char *sptr;
	sptr = Text::StrConcat(sbuff, (const UTF8Char*)"Received ");
	sptr = Text::StrUOSInt(sptr, size);
	sptr = Text::StrConcat(sptr, (const UTF8Char*)" bytes");
	me->log->LogMessage(sbuff, IO::ILogHandler::LOG_LEVEL_ACTION);

	MemCopyNO(&data->recvBuff[data->buffSize], buff, size);
	data->buffSize += size;
	
	size = me->protoHdlr->ParseProtocol(cli, data, data->cliData, data->recvBuff, data->buffSize);
	if (size >= 2048 || size <= 0)
	{
		data->buffSize = 0;
	}
	else if (size != data->buffSize)
	{
		MemCopyO(data->recvBuff, &data->recvBuff[data->buffSize - size], size);
		data->buffSize = size;
	}
}

void __stdcall SSWR::AVIRead::AVIRJTT808ServerForm::OnClientTimeout(Net::TCPClient *cli, void *userObj, void *cliData)
{
}

void __stdcall SSWR::AVIRead::AVIRJTT808ServerForm::OnClientConn(UInt32 *s, void *userObj)
{
	SSWR::AVIRead::AVIRJTT808ServerForm *me = (SSWR::AVIRead::AVIRJTT808ServerForm*)userObj;
	Net::TCPClient *cli;
	ClientData *data;
	NEW_CLASS(cli, Net::TCPClient(me->core->GetSocketFactory(), s));
	data = MemAlloc(ClientData, 1);
	data->buffSize = 0;
	data->seqId = 0;
	data->cliData = me->protoHdlr->CreateStreamData(cli);
	me->cliMgr->AddClient(cli, data);
}

void SSWR::AVIRead::AVIRJTT808ServerForm::ServerStop()
{
	if (this->svr)
	{
		DEL_CLASS(this->svr);
		DEL_CLASS(this->cliMgr);
		this->svr = 0;
		this->cliMgr = 0;
	}
}

SSWR::AVIRead::AVIRJTT808ServerForm::AVIRJTT808ServerForm(UI::GUIClientControl *parent, UI::GUICore *ui, SSWR::AVIRead::AVIRCore *core) : UI::GUIForm(parent, 1024, 768, ui)
{
	this->SetFont(0, 8.25, false);
	this->SetText((const UTF8Char*)"JT/T808 Server");

	this->core = core;
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));

	NEW_CLASS(this->tcMain, UI::GUITabControl(ui, this));
	this->tcMain->SetDockType(UI::GUIControl::DOCK_FILL);

	this->tpStatus = this->tcMain->AddTabPage((const UTF8Char*)"Status");
	NEW_CLASS(this->lblPort, UI::GUILabel(ui, this->tpStatus, (const UTF8Char*)"Port"));
	this->lblPort->SetRect(4, 4, 100, 23, false);
	NEW_CLASS(this->txtPort, UI::GUITextBox(ui, this->tpStatus, (const UTF8Char*)"42463"));
	this->txtPort->SetRect(104, 4, 100, 23, false);
	NEW_CLASS(this->btnStart, UI::GUIButton(ui, this->tpStatus, (const UTF8Char*)"Start"));
	this->btnStart->SetRect(204, 4, 75, 23, false);
	this->btnStart->HandleButtonClick(OnStartClicked, this);

	this->tpLog = this->tcMain->AddTabPage((const UTF8Char*)"Log");
	NEW_CLASS(this->txtLog, UI::GUITextBox(ui, this->tpLog, (const UTF8Char*)""));
	this->txtLog->SetRect(0, 0, 100, 23, false);
	this->txtLog->SetDockType(UI::GUIControl::DOCK_BOTTOM);
	this->txtLog->SetReadOnly(true);
	NEW_CLASS(this->lbLog, UI::GUIListBox(ui, this->tpLog, false));
	this->lbLog->SetDockType(UI::GUIControl::DOCK_FILL);
	this->lbLog->HandleSelectionChange(OnLogSelChg, this);

	NEW_CLASS(this->protoHdlr, IO::ProtoHdlr::ProtoJTT808Handler(this, 0));
	NEW_CLASS(this->log, IO::LogTool());
	NEW_CLASS(this->logger, UI::ListBoxLogger(this, this->lbLog, 100, false));
	this->logger->SetTimeFormat("yyyy-MM-dd HH:mm:ss.fff");
	this->log->AddLogHandler(this->logger, IO::ILogHandler::LOG_LEVEL_RAW);
	this->svr = 0;
	this->cliMgr = 0;
}

SSWR::AVIRead::AVIRJTT808ServerForm::~AVIRJTT808ServerForm()
{
	this->ServerStop();
	DEL_CLASS(this->log);
	DEL_CLASS(this->logger);
	DEL_CLASS(this->protoHdlr);
}

void SSWR::AVIRead::AVIRJTT808ServerForm::OnMonitorChanged()
{
	this->SetDPI(this->core->GetMonitorHDPI(this->GetHMonitor()), this->core->GetMonitorDDPI(this->GetHMonitor()));
}

void SSWR::AVIRead::AVIRJTT808ServerForm::DataParsed(IO::Stream *stm, void *stmObj, Int32 cmdType, Int32 seqId, const UInt8 *cmd, UOSInt cmdSize)
{
	ClientData *data = (ClientData*)stmObj;
	UInt8 packet[256];
	UInt8 tmpPacket[64];
	UTF8Char sbuff[512];
	UTF8Char *sptr;
	sptr = Text::StrConcat(sbuff, (const UTF8Char*)"Packet: seq = ");
	sptr = Text::StrInt32(sptr, seqId);
	sptr = Text::StrConcat(sptr, (const UTF8Char*)", type = 0x");
	sptr = Text::StrHexVal16(sptr, (UInt16)cmdType);
	sptr = Text::StrConcat(sptr, (const UTF8Char*)", size = ");
	sptr = Text::StrUOSInt(sptr, cmdSize);
	sptr = Text::StrConcat(sptr, (const UTF8Char*)", devId = ");
	sptr = Text::StrHexBytes(sptr, &cmd[5], 6, 0);
	this->log->LogMessage(sbuff, IO::ILogHandler::LOG_LEVEL_ACTION);

	UOSInt contSize;
	UOSInt i;
	OSInt j;
	UInt8 c;
	const UInt8 *packetContent = this->protoHdlr->GetPacketContent(cmd, &contSize);

	switch (cmdType)
	{
	case 2: //Keep Alive
		this->log->LogMessage((const UTF8Char*)"Cmd: Keep Alive", IO::ILogHandler::LOG_LEVEL_ACTION);
		break;
	case 3: //Logout
		this->log->LogMessage((const UTF8Char*)"Cmd: Device Logout", IO::ILogHandler::LOG_LEVEL_ACTION);
		break;
	case 0x100: //Login
		sptr = Text::StrConcat(sbuff, (const UTF8Char*)"Cmd: Login County = ");
		sptr = Text::StrUInt32(sptr, ReadMUInt16(&packetContent[0]));
		sptr = Text::StrConcat(sptr, (const UTF8Char*)", City = ");
		sptr = Text::StrUInt32(sptr, ReadMUInt16(&packetContent[2]));
		sptr = Text::StrConcat(sptr, (const UTF8Char*)", Make = ");
		i = 5;
		j = 4;
		while (i-- > 0)
		{
			c = packetContent[j++];
			if (c == 0)
				break;
			*sptr++ = c;
		}
		sptr = Text::StrConcat(sptr, (const UTF8Char*)", Model = ");
		i = 20;
		j = 9;
		while (i-- > 0)
		{
			c = packetContent[j++];
			if (c == 0)
				break;
			*sptr++ = c;
		}
		sptr = Text::StrConcat(sptr, (const UTF8Char*)", SN = ");
		i = 7;
		j = 29;
		while (i-- > 0)
		{
			c = packetContent[j++];
			if (c == 0)
				break;
			*sptr++ = c;
		}
		sptr = Text::StrConcat(sptr, (const UTF8Char*)", Color = ");
		sptr = Text::StrUInt32(sptr, packetContent[36]);
		sptr = Text::StrConcat(sptr, (const UTF8Char*)", License = ");
		i = contSize - 37;
		j = 37;
		while (i-- > 0)
		{
			c = packetContent[j++];
			if (c == 0)
				break;
			*sptr++ = c;
		}
		*sptr = 0;
		this->log->LogMessage(sbuff, IO::ILogHandler::LOG_LEVEL_ACTION);


		WriteMInt16(&tmpPacket[0], seqId);
		tmpPacket[2] = 0;
		tmpPacket[3] = '0';
		tmpPacket[4] = '0';
		tmpPacket[5] = '0';
		tmpPacket[6] = '0';
		tmpPacket[7] = '0';
		tmpPacket[8] = '0';
		i = this->protoHdlr->BuildPacket(packet, 0x8100, data->seqId++, tmpPacket, 9, data->cliData);
		stm->Write(packet, i);
		break;
	case 0x102: //Authentication
		this->log->LogMessage((const UTF8Char*)"Cmd: Authentication", IO::ILogHandler::LOG_LEVEL_ACTION);
		WriteMInt16(&tmpPacket[0], seqId);
		WriteMInt16(&tmpPacket[2], 0x102);
		tmpPacket[4] = 0;
		i = this->protoHdlr->BuildPacket(packet, 0x8001, data->seqId++, tmpPacket, 5, data->cliData);
		stm->Write(packet, i);
		break;
	case 0x200:
		sptr = Text::StrConcat(sbuff, (const UTF8Char*)"Cmd: Location: Alert = 0x");
		sptr = Text::StrHexVal32(sptr, ReadMUInt32(&packetContent[0]));
		sptr = Text::StrConcat(sptr, (const UTF8Char*)", Status = 0x");
		sptr = Text::StrHexVal32(sptr, ReadMUInt32(&packetContent[4]));
		sptr = Text::StrConcat(sptr, (const UTF8Char*)", Latitude = ");
		sptr = Text::StrDouble(sptr, ReadMUInt32(&packetContent[8]) * 0.000001);
		sptr = Text::StrConcat(sptr, (const UTF8Char*)", Longitude = ");
		sptr = Text::StrDouble(sptr, ReadMUInt32(&packetContent[12]) * 0.000001);
		sptr = Text::StrConcat(sptr, (const UTF8Char*)", Altitude = ");
		sptr = Text::StrUInt32(sptr, ReadMUInt16(&packetContent[16]));
		sptr = Text::StrConcat(sptr, (const UTF8Char*)", Speed = ");
		sptr = Text::StrDouble(sptr, ReadMUInt16(&packetContent[18]) * 0.1);
		sptr = Text::StrConcat(sptr, (const UTF8Char*)", Dir = ");
		sptr = Text::StrUInt32(sptr, ReadMUInt16(&packetContent[20]));
		{
			Data::DateTime dt;
			dt.SetValue((UInt16)(2000 + Data::ByteTool::GetBCD8(packetContent[22])), Data::ByteTool::GetBCD8(packetContent[23]), Data::ByteTool::GetBCD8(packetContent[24]), Data::ByteTool::GetBCD8(packetContent[25]), Data::ByteTool::GetBCD8(packetContent[26]), Data::ByteTool::GetBCD8(packetContent[27]), 0, 32);
			sptr = Text::StrConcat(sptr, (const UTF8Char*)", Time = ");
			sptr = dt.ToString(sptr);
		}
		i = 28;
		while (i < contSize)
		{
			if (i + 2 + packetContent[i + 1] > contSize)
				break;
			switch (packetContent[i])
			{
			case 1:
				sptr = Text::StrConcat(sptr, (const UTF8Char*)", Mileage = ");
				if (packetContent[i + 1] == 4)
				{
					sptr = Text::StrDouble(sptr, ReadMUInt32(&packetContent[i + 2]) * 0.1);
				}
				break;
			case 2:
				sptr = Text::StrConcat(sptr, (const UTF8Char*)", Fuel Level = ");
				if (packetContent[i + 1] == 2)
				{
					sptr = Text::StrDouble(sptr, ReadMUInt16(&packetContent[i + 2]) * 0.1);
					sptr = Text::StrConcat(sptr, (const UTF8Char*)"L");
				}
				break;
			case 3:
				sptr = Text::StrConcat(sptr, (const UTF8Char*)", Record Speed = ");
				if (packetContent[i + 1] == 2)
				{
					sptr = Text::StrDouble(sptr, ReadMUInt16(&packetContent[i + 2]) * 0.1);
				}
				break;
			case 0x25:
				sptr = Text::StrConcat(sptr, (const UTF8Char*)", Status2 = ");
				if (packetContent[i + 1] == 4)
				{
					sptr = Text::StrConcat(sptr, (const UTF8Char*)"0x");
					sptr = Text::StrHexVal32(sptr, ReadMUInt32(&packetContent[i + 2]));
				}
				break;
			case 0x2A:
				sptr = Text::StrConcat(sptr, (const UTF8Char*)", IOStatus = ");
				if (packetContent[i + 1] == 2)
				{
					sptr = Text::StrConcat(sptr, (const UTF8Char*)"0x");
					sptr = Text::StrHexVal16(sptr, ReadMUInt16(&packetContent[i + 2]));
				}
				break;
			case 0x2B:
				sptr = Text::StrConcat(sptr, (const UTF8Char*)", ADC: ");
				if (packetContent[i + 1] == 4)
				{
					sptr = Text::StrConcat(sptr, (const UTF8Char*)"AD0 = ");
					sptr = Text::StrUInt16(sptr, ReadMUInt16(&packetContent[i + 4]));
					sptr = Text::StrConcat(sptr, (const UTF8Char*)", AD1 = ");
					sptr = Text::StrUInt16(sptr, ReadMUInt16(&packetContent[i + 2]));
				}
				break;
			case 0x30:
				sptr = Text::StrConcat(sptr, (const UTF8Char*)", GSM RSSI = ");
				if (packetContent[i + 1] == 1)
				{
					sptr = Text::StrUInt16(sptr, packetContent[i + 2]);
				}
				break;
			case 0x31:
				sptr = Text::StrConcat(sptr, (const UTF8Char*)", GNSS Cnt = ");
				if (packetContent[i + 1] == 1)
				{
					sptr = Text::StrUInt16(sptr, packetContent[i + 2]);
				}
				break;
			default:
				sptr = Text::StrConcat(sptr, (const UTF8Char*)", Unk Type (0x");
				sptr = Text::StrHexByte(sptr, packetContent[i]);
				sptr = Text::StrConcat(sptr, (const UTF8Char*)")");
				break;
			}
			i += (UOSInt)packetContent[i + 1] + 2;
		}
		this->log->LogMessage(sbuff, IO::ILogHandler::LOG_LEVEL_ACTION);
		break;
	case 0x704:
		sptr = Text::StrConcat(sbuff, (const UTF8Char*)"Cmd: Batch Upload, Count = ");
		sptr = Text::StrUInt32(sptr, ReadMUInt16(&packetContent[0]));
		sptr = Text::StrConcat(sptr, (const UTF8Char*)", Buffer Data = ");
		sptr = Text::StrUInt16(sptr, packetContent[2]);
		this->log->LogMessage(sbuff, IO::ILogHandler::LOG_LEVEL_ACTION);
		j = 3;
		//////////////////////////////////////
		break;

	case 0x900:
		sptr = Text::StrConcat(sbuff, (const UTF8Char*)"Cmd: Wrapped Data, type = 0x");
		sptr = Text::StrHexByte(sptr, packetContent[0]);
		sptr = Text::StrConcat(sptr, (const UTF8Char*)", data = ");
		sptr = Text::StrHexBytes(sptr, &packetContent[1], contSize - 1, ' ');
		this->log->LogMessage(sbuff, IO::ILogHandler::LOG_LEVEL_ACTION);
		break;

	default:
		this->log->LogMessage((const UTF8Char*)"Cmd: Unknown", IO::ILogHandler::LOG_LEVEL_ACTION);
		break;
	}
}

void SSWR::AVIRead::AVIRJTT808ServerForm::DataSkipped(IO::Stream *stm, void *stmObj, const UInt8 *buff, UOSInt buffSize)
{
	UTF8Char sbuff[256];
	UTF8Char *sptr;
	sptr = Text::StrConcat(sbuff, (const UTF8Char*)"Data skipped ");
	sptr = Text::StrUOSInt(sptr, buffSize);
	sptr = Text::StrConcat(sptr, (const UTF8Char*)" bytes");
	this->log->LogMessage(sbuff, IO::ILogHandler::LOG_LEVEL_ACTION);
}
