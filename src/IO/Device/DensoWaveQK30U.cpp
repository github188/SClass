#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ByteTool.h"
#include "Data/DateTime.h"
#include "IO/Device/DensoWaveQK30U.h"
#include "Sync/Thread.h"
#include "Text/Encoding.h"
#include "Text/MyString.h"
#define RECVBUFFSIZE 256

UInt32 __stdcall IO::Device::DensoWaveQK30U::RecvThread(void *userObj)
{
	IO::Device::DensoWaveQK30U *me = (IO::Device::DensoWaveQK30U*)userObj;
	UInt8 buff[256];
	UTF8Char *sbuff;
	OSInt recvSize;
	OSInt i;
	Bool found;
	me->recvRunning = true;
	sbuff = MemAlloc(UTF8Char, RECVBUFFSIZE + 1);
	while (!me->recvToStop)
	{
		recvSize = me->stm->Read(buff, 256);
		if (recvSize <= 0)
		{
			Sync::Thread::Sleep(10);
		}
		else
		{
			me->recvMut->Lock();
			if (me->recvSize >= RECVBUFFSIZE)
			{
			}
			else if (me->recvSize + recvSize > RECVBUFFSIZE)
			{
				MemCopyNO(&me->recvBuff[me->recvSize], buff, RECVBUFFSIZE - me->recvSize);
				me->recvSize = RECVBUFFSIZE;
				me->recvEvt->Set();
			}
			else
			{
				MemCopyNO(&me->recvBuff[me->recvSize], buff, recvSize);
				me->recvSize += recvSize;
				me->recvEvt->Set();
			}
			if (me->currMode == IO::Device::DensoWaveQK30U::MT_SCAN)
			{
				while (me->recvSize > 0)
				{
					found = false;
					i = 0;
					while (i < me->recvSize)
					{
						if (me->recvBuff[i] == 13)
						{
							Text::StrConcatC(sbuff, me->recvBuff, i);
							if (me->scanHdlr)
							{
								me->scanHdlr(me->scanHdlrObj, sbuff);
							}
//							me->stm->Write((UInt8*)"READOFF\r", 8);
//							Sync::Thread::Sleep(me->scanDelay);
//							me->stm->Write((UInt8*)"READON\r", 7);

							found = true;
							if (i + 1 >= me->recvSize)
							{
								me->recvSize = 0;
							}
							else
							{
								MemCopyO(me->recvBuff, &me->recvBuff[i + 1], me->recvSize - i - 1);
							}
							break;
						}
						i++;
					}
					if (!found)
						break;
				}
			}
			me->recvMut->Unlock();
		}
	}
	MemFree(sbuff);
	me->recvRunning = false;
	return 0;
}

Bool IO::Device::DensoWaveQK30U::ScanModeStart()
{
	Bool succ;
	this->reqMut->Lock();
	succ = this->stm->Write((UInt8*)"R\r", 2) == 2;
	this->reqMut->Unlock();
	return succ;
}

Bool IO::Device::DensoWaveQK30U::ScanModeEnd()
{
	Bool succ;
	this->reqMut->Lock();
	succ = this->stm->Write((UInt8*)"Z\r", 2) == 2;
	this->reqMut->Unlock();
	return succ;
}

Bool IO::Device::DensoWaveQK30U::SettingModeStart()
{
	return true;
}

Bool IO::Device::DensoWaveQK30U::SettingModeEnd()
{
	return true;
}

void IO::Device::DensoWaveQK30U::ToIdleMode()
{
	if (this->currMode == IO::Device::DensoWaveQK30U::MT_SCAN)
	{
		this->ScanModeEnd();
		this->currMode = IO::Device::DensoWaveQK30U::MT_IDLE;
	}
	else if (this->currMode == IO::Device::DensoWaveQK30U::MT_SETTING)
	{
		this->SettingModeEnd();
		this->currMode = IO::Device::DensoWaveQK30U::MT_IDLE;
	}
}

Bool IO::Device::DensoWaveQK30U::WaitForReply(Int32 timeToWait)
{
	Data::DateTime startTime;
	Data::DateTime currTime;
	Int32 t;
	OSInt currBuffSize = 0;
	OSInt startIndex;
	OSInt i;
	startTime.SetCurrTimeUTC();
	i = 0;
	while (true)
	{
		currTime.SetCurrTimeUTC();
		t = (Int32)currTime.DiffMS(&startTime);
		if (currBuffSize != this->recvSize)
		{
			currBuffSize = this->recvSize;
			startIndex = 0;
			while (i < currBuffSize)
			{
				if (this->recvBuff[i] == 13)
				{
					this->recvBuff[i] = 0;
					if (Text::StrEquals((Char*)&this->recvBuff[startIndex], "OK"))
						return true;
					else 
						return false;
				}
				i++;
			}
		}
		if (t >= timeToWait)
		{
			return false;
		}
		this->recvEvt->Wait(timeToWait - t);
	}
}

Bool IO::Device::DensoWaveQK30U::WaitForReplyVal(Int32 timeToWait, Int32 *retVal)
{
	Data::DateTime startTime;
	Data::DateTime currTime;
	Int32 t;
	OSInt currBuffSize = 0;
	OSInt startIndex;
	OSInt i;
	startTime.SetCurrTimeUTC();
	i = 0;
	while (true)
	{
		currTime.SetCurrTimeUTC();
		t = (Int32)currTime.DiffMS(&startTime);
		if (currBuffSize != this->recvSize)
		{
			currBuffSize = this->recvSize;
			startIndex = 0;
			while (i < currBuffSize)
			{
				if (this->recvBuff[i] == 13 || this->recvBuff[i] == 0)
				{
					this->recvBuff[i] = 0;
					return Text::StrToInt32((Char*)&this->recvBuff[startIndex], retVal);
				}
				i++;
			}
		}
		if (t >= timeToWait)
		{
			return false;
		}
		this->recvEvt->Wait(timeToWait - t);
	}
}

Int32 IO::Device::DensoWaveQK30U::ReadCommand(const Char *cmdStr, UOSInt cmdLen)
{
	Int32 result;
	this->reqMut->Lock();
	this->recvMut->Lock();
	this->recvSize = 0;
	this->recvMut->Unlock();
	if (this->stm->Write((const UInt8*)cmdStr, cmdLen) == cmdLen)
	{
		if (!this->WaitForReplyVal(1000, &result))
		{
			result = -1;
		}
	}
	else
	{
		result = -1;
	}
	this->reqMut->Unlock();
	return result;
}

Bool IO::Device::DensoWaveQK30U::WriteCommand(const Char *cmdStr, UOSInt cmdLen)
{
	Bool succ = false;
	this->reqMut->Lock();
	this->recvMut->Lock();
	this->recvSize = 0;
	this->recvMut->Unlock();
	if (this->stm->Write((UInt8*)cmdStr, cmdLen) == cmdLen)
	{
		if (this->WaitForReply(1000))
		{
			succ = true;
		}
	}
	this->reqMut->Unlock();
	return succ;
}

IO::Device::DensoWaveQK30U::DensoWaveQK30U(IO::Stream *stm) : IO::CodeScanner((const UTF8Char*)"Denso Wave QK30-U")
{
	this->stm = stm;
	this->scanDelay = 1000;

	NEW_CLASS(this->reqMut, Sync::Mutex());
	NEW_CLASS(this->recvMut, Sync::Mutex());
	NEW_CLASS(this->recvEvt, Sync::Event(true, (const UTF8Char*)"IO.TVControl.recvEvt"));
	this->recvBuff = MemAlloc(UInt8, RECVBUFFSIZE);
	this->recvSize = 0;
	this->recvRunning = true;
	this->recvToStop = false;
	this->currMode = IO::Device::DensoWaveQK30U::MT_IDLE;
	this->scanHdlr = 0;
	this->scanHdlrObj = 0;

	Sync::Thread::Create(RecvThread, this);
}

IO::Device::DensoWaveQK30U::~DensoWaveQK30U()
{
	this->ToIdleMode();

	this->recvToStop = true;
	this->stm->Close();
	while (this->recvRunning)
	{
		Sync::Thread::Sleep(10);
	}
//	DEL_CLASS(this->nextTime);
	DEL_CLASS(this->recvEvt);
	DEL_CLASS(this->recvMut);
	DEL_CLASS(this->reqMut);
	MemFree(this->recvBuff);
	DEL_CLASS(this->stm);
}

void IO::Device::DensoWaveQK30U::SetCurrMode(ModeType currMode)
{
	if (this->currMode == currMode)
		return;
	this->ToIdleMode();
	if (currMode == IO::Device::DensoWaveQK30U::MT_SCAN)
	{
		this->currMode = currMode;
		this->recvMut->Lock();
		this->recvSize = 0;
		this->recvMut->Unlock();
		this->ScanModeStart();
	}
	else if (currMode == IO::Device::DensoWaveQK30U::MT_SETTING)
	{
		this->currMode = currMode;
		this->SettingModeStart();
	}
}

Bool IO::Device::DensoWaveQK30U::SoftReset()
{
	Bool succ;
	this->reqMut->Lock();
	this->recvMut->Lock();
	this->recvSize = 0;
	this->recvMut->Unlock();
	succ = this->stm->Write((UInt8*)"RESET\r", 6) == 6;
	this->reqMut->Unlock();
	if (succ)
	{
		this->currMode = IO::Device::DensoWaveQK30U::MT_IDLE;
	}
	return succ;
}

Bool IO::Device::DensoWaveQK30U::ResetDefault()
{
	Bool succ;
	this->reqMut->Lock();
	this->recvMut->Lock();
	this->recvSize = 0;
	this->recvMut->Unlock();
	succ = this->stm->Write((UInt8*)"DEFAULT\r", 8) == 8;
	this->reqMut->Unlock();
	if (succ)
	{
		this->currMode = IO::Device::DensoWaveQK30U::MT_IDLE;
	}
	return succ;
}

void IO::Device::DensoWaveQK30U::HandleCodeScanned(ScanHandler hdlr, void *userObj)
{
	this->scanHdlr = hdlr;
	this->scanHdlrObj = userObj;
}

OSInt IO::Device::DensoWaveQK30U::GetCommandList(Data::ArrayList<DeviceCommand> *cmdList)
{
	OSInt initCnt = cmdList->GetCount();
/*	cmdList->Add(DC_GET_READ_MODE);
	cmdList->Add(DC_SET_READ_MODE);
	cmdList->Add(DC_GET_BW_MODE);
	cmdList->Add(DC_SET_BW_MODE);
	cmdList->Add(DC_GET_READ_REPEAT_TIME);
	cmdList->Add(DC_SET_READ_REPEAT_TIME);
	cmdList->Add(DC_GET_BRIGHTNESS);
	cmdList->Add(DC_SET_BRIGHTNESS);
	cmdList->Add(DC_GET_SHT_SIGNAL);
	cmdList->Add(DC_SET_SHT_SIGNAL);
	cmdList->Add(DC_GET_SCAN_MODE);
	cmdList->Add(DC_SET_SCAN_MODE);
	cmdList->Add(DC_GET_SHUTTER_TIME);
	cmdList->Add(DC_SET_SHUTTER_TIME);
	cmdList->Add(DC_GET_GAIN);
	cmdList->Add(DC_SET_GAIN);
	cmdList->Add(DC_GET_LED_LEVEL);
	cmdList->Add(DC_SET_LED_LEVEL);
	cmdList->Add(DC_GET_LED_MODE);
	cmdList->Add(DC_SET_LED_MODE);
	cmdList->Add(DC_GET_OUTPUT_TIMING);
	cmdList->Add(DC_SET_OUTPUT_TIMING);
	cmdList->Add(DC_GET_UNREAD_DATA_SEND);
	cmdList->Add(DC_SET_UNREAD_DATA_SEND);
	cmdList->Add(DC_GET_INDIR_TIME);
	cmdList->Add(DC_SET_INDIR_TIME);
	cmdList->Add(DC_GET_TRIGGER_DELAY);
	cmdList->Add(DC_SET_TRIGGER_DELAY);
	cmdList->Add(DC_GET_SIGNAL_ON_DUR);
	cmdList->Add(DC_SET_SIGNAL_ON_DUR);
	cmdList->Add(DC_GET_SIGNAL_DELAY);
	cmdList->Add(DC_SET_SIGNAL_DELAY);
	cmdList->Add(DC_GET_LIGHT_LED);
	cmdList->Add(DC_SET_LIGHT_LED);
	cmdList->Add(DC_GET_MARKER_LIGHT);
	cmdList->Add(DC_SET_MARKER_LIGHT);
	cmdList->Add(DC_GET_DECODE_TIME_LIMIT);
	cmdList->Add(DC_SET_DECODE_TIME_LIMIT);
	cmdList->Add(DC_GET_OUTPUT1_TYPE);
	cmdList->Add(DC_SET_OUTPUT1_TYPE);
	cmdList->Add(DC_GET_OUTPUT2_TYPE);
	cmdList->Add(DC_SET_OUTPUT2_TYPE);
	cmdList->Add(DC_GET_AUTO_SENSE_MODE);
	cmdList->Add(DC_SET_AUTO_SENSE_MODE);
	cmdList->Add(DC_GET_CONT_READ_MODE_B);
	cmdList->Add(DC_SET_CONT_READ_MODE_B);
	cmdList->Add(DC_GET_QRCODE);
	cmdList->Add(DC_SET_QRCODE);
	cmdList->Add(DC_GET_MICRO_QRCODE);
	cmdList->Add(DC_SET_MICRO_QRCODE);
	cmdList->Add(DC_GET_PDF417);
	cmdList->Add(DC_SET_PDF417);
	cmdList->Add(DC_GET_DATAMATRIX);
	cmdList->Add(DC_SET_DATAMATRIX);
	cmdList->Add(DC_GET_BARCODE);
	cmdList->Add(DC_SET_BARCODE);
	cmdList->Add(DC_GET_INTERLEAVED_2OF5);
	cmdList->Add(DC_SET_INTERLEAVED_2OF5);
	cmdList->Add(DC_GET_CODABAR);
	cmdList->Add(DC_SET_CODABAR);
	cmdList->Add(DC_GET_CODABAR_START_STOP);
	cmdList->Add(DC_SET_CODEBAR_START_STOP);
	cmdList->Add(DC_GET_CODE39);
	cmdList->Add(DC_SET_CODE39);
	cmdList->Add(DC_GET_CODE128);
	cmdList->Add(DC_SET_CODE128);
	cmdList->Add(DC_GET_QRCODE_REVERSE);
	cmdList->Add(DC_SET_QRCODE_REVERSE);
	cmdList->Add(DC_GET_QRLINK_CODE);
	cmdList->Add(DC_SET_QRLINK_CODE);
	cmdList->Add(DC_GET_GS1_DATABAR);
	cmdList->Add(DC_SET_GS1_DATABAR);
	cmdList->Add(DC_GET_GS1_COMPOSITE);
	cmdList->Add(DC_SET_GS1_COMPOSITE);
	cmdList->Add(DC_GET_MICRO_PDF417);
	cmdList->Add(DC_SET_MICRO_PDF417);
	cmdList->Add(DC_GET_BARCODE_READ_MODE);
	cmdList->Add(DC_SET_BARCODE_READ_MODE);
	cmdList->Add(DC_GET_SQRC);
	cmdList->Add(DC_SET_SQRC);
	cmdList->Add(DC_GET_SQRC_KEY_UNMATCH);
	cmdList->Add(DC_SET_SQRC_KEY_UNMATCH);
	cmdList->Add(DC_GET_SQRC_KEY_MATCH);
	cmdList->Add(DC_SET_SQRC_KEY_MATCH);
	cmdList->Add(DC_GET_IQRCODE_SQUARE);
	cmdList->Add(DC_SET_IQRCODE_SQUARE);
	cmdList->Add(DC_GET_IQRCODE_RECT);
	cmdList->Add(DC_SET_IQRCODE_RECT);
	cmdList->Add(DC_GET_AZTEC_FULL);
	cmdList->Add(DC_SET_AZTEC_FULL);
	cmdList->Add(DC_GET_AZTEC_COMPACT);
	cmdList->Add(DC_SET_AZTEC_COMPACT);
	cmdList->Add(DC_GET_MENU_READ);
	cmdList->Add(DC_SET_MENU_READ);
	cmdList->Add(DC_GET_COMM_SEQ);
	cmdList->Add(DC_SET_COMM_SEQ);
	cmdList->Add(DC_GET_BAUD_RATE);
	cmdList->Add(DC_SET_BAUD_RATE);
	cmdList->Add(DC_GET_CODE_MARK);
	cmdList->Add(DC_SET_CODE_MARK);
	cmdList->Add(DC_GET_LINE_NUM);
	cmdList->Add(DC_SET_LINE_NUM);
	cmdList->Add(DC_GET_BCC);
	cmdList->Add(DC_SET_BCC);
	cmdList->Add(DC_GET_CTS_SIGNAL);
	cmdList->Add(DC_SET_CTS_SIGNAL);
	cmdList->Add(DC_GET_CTS_TIME);
	cmdList->Add(DC_SET_CTS_TIME);
	cmdList->Add(DC_GET_ACK_NAK_TIME);
	cmdList->Add(DC_SET_ACK_NAK_TIME);
	cmdList->Add(DC_GET_RECV_HDR);
	cmdList->Add(DC_SET_RECV_HDR);
	cmdList->Add(DC_GET_RECV_TERMINATOR);
	cmdList->Add(DC_GET_SEND_TERMINATOR);
	cmdList->Add(DC_GET_BUZZER);
	cmdList->Add(DC_SET_BUZZER);
	cmdList->Add(DC_GET_READ_ERR_BUZZER);
	cmdList->Add(DC_SET_READ_ERR_BUZZER);
	cmdList->Add(DC_GET_MAGIC_KEY);
	cmdList->Add(DC_SET_MAGIC_KEY);
	cmdList->Add(DC_GET_POWER_ON_BUZZER);
	cmdList->Add(DC_SET_POWER_ON_BUZZER);
	cmdList->Add(DC_GET_BUZZER_OFF);
	cmdList->Add(DC_SET_BUZZER_OFF);*/
	return cmdList->GetCount() - initCnt;
}

const UTF8Char *IO::Device::DensoWaveQK30U::GetCommandName(DeviceCommand dcmd)
{
	return (const UTF8Char*)"Unknown";
}

IO::Device::DensoWaveQK30U::CommandType IO::Device::DensoWaveQK30U::GetCommandParamType(DeviceCommand dcmd, Int32 *minVal, Int32 *maxVal)
{
	switch (dcmd)
	{
	case 0:
	default:
		return IO::Device::DensoWaveQK30U::CT_UNKNOWN;
	}
}

const UTF8Char *IO::Device::DensoWaveQK30U::GetCommandParamName(DeviceCommand dcmd, Int32 cmdVal)
{
	switch (dcmd)
	{
	case 0:
	default:
		return 0;
	}
}

Int32 IO::Device::DensoWaveQK30U::GetCommand(DeviceCommand dcmd)
{
	return -1;
}

Bool IO::Device::DensoWaveQK30U::SetCommand(DeviceCommand dcmd, Int32 val)
{
	return false;
}