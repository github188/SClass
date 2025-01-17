#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ByteTool.h"
#include "Data/DateTime.h"
#include "IO/Device/DensoWaveQB30.h"
#include "Sync/MutexUsage.h"
#include "Sync/Thread.h"
#include "Text/Encoding.h"
#include "Text/MyString.h"
#define RECVBUFFSIZE 256

UInt32 __stdcall IO::Device::DensoWaveQB30::RecvThread(void *userObj)
{
	IO::Device::DensoWaveQB30 *me = (IO::Device::DensoWaveQB30*)userObj;
	UInt8 buff[256];
	UTF8Char *sbuff;
	UOSInt recvSize;
	UOSInt i;
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
			Sync::MutexUsage mutUsage(me->recvMut);
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
			if (me->currMode == IO::Device::DensoWaveQB30::MT_SCAN)
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
							me->stm->Write((UInt8*)"READOFF\r", 8);
							Sync::Thread::Sleep(me->scanDelay);
							me->stm->Write((UInt8*)"READON\r", 7);

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
			mutUsage.EndUse();
		}
	}
	MemFree(sbuff);
	me->recvRunning = false;
	return 0;
}

Bool IO::Device::DensoWaveQB30::ScanModeStart()
{
	Bool succ;
	Sync::MutexUsage mutUsage(this->reqMut);
	succ = this->stm->Write((UInt8*)"READON\r", 7) == 7;
	mutUsage.EndUse();
	return succ;
}

Bool IO::Device::DensoWaveQB30::ScanModeEnd()
{
	Bool succ;
	Sync::MutexUsage mutUsage(this->reqMut);
	succ = this->stm->Write((UInt8*)"READOFF\r", 8) == 8;
	mutUsage.EndUse();
	return succ;
}

Bool IO::Device::DensoWaveQB30::SettingModeStart()
{
	Bool succ;
	Sync::MutexUsage mutUsage(this->reqMut);
	succ = this->stm->Write((UInt8*)"START\r", 6) == 6;
	mutUsage.EndUse();
	return succ;
}

Bool IO::Device::DensoWaveQB30::SettingModeEnd()
{
	Bool succ;
	Sync::MutexUsage mutUsage(this->reqMut);
	succ = this->stm->Write((UInt8*)"END\r", 4) == 4;
	mutUsage.EndUse();
	return succ;
}

void IO::Device::DensoWaveQB30::ToIdleMode()
{
	if (this->currMode == IO::Device::DensoWaveQB30::MT_SCAN)
	{
		this->ScanModeEnd();
		this->currMode = IO::Device::DensoWaveQB30::MT_IDLE;
	}
	else if (this->currMode == IO::Device::DensoWaveQB30::MT_SETTING)
	{
		this->SettingModeEnd();
		this->currMode = IO::Device::DensoWaveQB30::MT_IDLE;
	}
}

Bool IO::Device::DensoWaveQB30::WaitForReply(UInt32 timeToWait)
{
	Data::DateTime startTime;
	Data::DateTime currTime;
	Int32 t;
	UOSInt currBuffSize = 0;
	UOSInt startIndex;
	UOSInt i;
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
		if (t >= (Int32)timeToWait)
		{
			return false;
		}
		this->recvEvt->Wait(timeToWait - (UInt32)t);
	}
}

Bool IO::Device::DensoWaveQB30::WaitForReplyVal(UInt32 timeToWait, Int32 *retVal)
{
	Data::DateTime startTime;
	Data::DateTime currTime;
	Int32 t;
	UOSInt currBuffSize = 0;
	UOSInt startIndex;
	UOSInt i;
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
		if (t >= (Int32)timeToWait)
		{
			return false;
		}
		this->recvEvt->Wait(timeToWait - (UInt32)t);
	}
}

Int32 IO::Device::DensoWaveQB30::ReadCommand(const Char *cmdStr, UOSInt cmdLen)
{
	Int32 result;
	Sync::MutexUsage mutUsage(this->reqMut);
	Sync::MutexUsage recvMutUsage(this->recvMut);
	this->recvSize = 0;
	recvMutUsage.EndUse();
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
	mutUsage.EndUse();
	return result;
}

Bool IO::Device::DensoWaveQB30::WriteCommand(const Char *cmdStr, UOSInt cmdLen)
{
	Bool succ = false;
	Sync::MutexUsage mutUsage(this->reqMut);
	Sync::MutexUsage recvMutUsage(this->recvMut);
	this->recvSize = 0;
	recvMutUsage.EndUse();
	if (this->stm->Write((UInt8*)cmdStr, cmdLen) == cmdLen)
	{
		if (this->WaitForReply(1000))
		{
			succ = true;
		}
	}
	mutUsage.EndUse();
	return succ;
}

IO::Device::DensoWaveQB30::DensoWaveQB30(IO::Stream *stm) : IO::CodeScanner((const UTF8Char*)"Denso Wave QB30")
{
	this->stm = stm;
	this->scanDelay = 1000;
//	NEW_CLASS(this->nextTime, Data::DateTime());
//	this->nextTime->SetCurrTimeUTC();

	NEW_CLASS(this->reqMut, Sync::Mutex());
	NEW_CLASS(this->recvMut, Sync::Mutex());
	NEW_CLASS(this->recvEvt, Sync::Event(true, (const UTF8Char*)"IO.Device.DensoWaveQB30.recvEvt"));
	this->recvBuff = MemAlloc(UInt8, RECVBUFFSIZE);
	this->recvSize = 0;
	this->recvRunning = true;
	this->recvToStop = false;
	this->currMode = IO::Device::DensoWaveQB30::MT_IDLE;
	this->scanHdlr = 0;
	this->scanHdlrObj = 0;

	Sync::Thread::Create(RecvThread, this);
}

IO::Device::DensoWaveQB30::~DensoWaveQB30()
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

void IO::Device::DensoWaveQB30::SetCurrMode(ModeType currMode)
{
	if (this->currMode == currMode)
		return;
	this->ToIdleMode();
	if (currMode == IO::Device::DensoWaveQB30::MT_SCAN)
	{
		this->currMode = currMode;
		Sync::MutexUsage mutUsage(this->recvMut);
		this->recvSize = 0;
		mutUsage.EndUse();
		this->ScanModeStart();
	}
	else if (currMode == IO::Device::DensoWaveQB30::MT_SETTING)
	{
		this->currMode = currMode;
		this->SettingModeStart();
	}
}

Bool IO::Device::DensoWaveQB30::SoftReset()
{
	Bool succ;
	Sync::MutexUsage mutUsage(this->reqMut);
	Sync::MutexUsage recvMutUsage(this->recvMut);
	this->recvSize = 0;
	recvMutUsage.EndUse();
	succ = this->stm->Write((UInt8*)"RESET\r", 6) == 6;
	mutUsage.EndUse();
	if (succ)
	{
		this->currMode = IO::Device::DensoWaveQB30::MT_IDLE;
	}
	return succ;
}

Bool IO::Device::DensoWaveQB30::ResetDefault()
{
	Bool succ;
	Sync::MutexUsage mutUsage(this->reqMut);
	Sync::MutexUsage recvMutUsage(this->recvMut);
	this->recvSize = 0;
	recvMutUsage.EndUse();
	succ = this->stm->Write((UInt8*)"DEFAULT\r", 8) == 8;
	mutUsage.EndUse();
	if (succ)
	{
		this->currMode = IO::Device::DensoWaveQB30::MT_IDLE;
	}
	return succ;
}

void IO::Device::DensoWaveQB30::HandleCodeScanned(ScanHandler hdlr, void *userObj)
{
	this->scanHdlr = hdlr;
	this->scanHdlrObj = userObj;
}

UOSInt IO::Device::DensoWaveQB30::GetCommandList(Data::ArrayList<DeviceCommand> *cmdList)
{
	UOSInt initCnt = cmdList->GetCount();
	cmdList->Add(DC_GET_READ_MODE);
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
	cmdList->Add(DC_SET_BUZZER_OFF);
	return cmdList->GetCount() - initCnt;
}

const UTF8Char *IO::Device::DensoWaveQB30::GetCommandName(DeviceCommand dcmd)
{
	switch (dcmd)
	{
	case DC_GET_READ_MODE:
		return (const UTF8Char*)"Get Read Mode";
	case DC_SET_READ_MODE:
		return (const UTF8Char*)"Set Read Mode";
	case DC_GET_BW_MODE:
		return (const UTF8Char*)"Get B/W Mode";
	case DC_SET_BW_MODE:
		return (const UTF8Char*)"Set B/W Mode";
	case DC_GET_READ_REPEAT_TIME:
		return (const UTF8Char*)"Get Read Repeat Time";
	case DC_SET_READ_REPEAT_TIME:
		return (const UTF8Char*)"Set Read Repeat Time";
	case DC_GET_BRIGHTNESS:
		return (const UTF8Char*)"Get Brightness";
	case DC_SET_BRIGHTNESS:
		return (const UTF8Char*)"Set Brightness";
	case DC_GET_SHT_SIGNAL:
		return (const UTF8Char*)"Get SHT Signal";
	case DC_SET_SHT_SIGNAL:
		return (const UTF8Char*)"Set SHT Signal";
	case DC_GET_SCAN_MODE:
		return (const UTF8Char*)"Get Scan Mode";
	case DC_SET_SCAN_MODE:
		return (const UTF8Char*)"Set Scan Mode";
	case DC_GET_SHUTTER_TIME:
		return (const UTF8Char*)"Get Shutter Time";
	case DC_SET_SHUTTER_TIME:
		return (const UTF8Char*)"Set Shutter Time";
	case DC_GET_GAIN:
		return (const UTF8Char*)"Get Gain";
	case DC_SET_GAIN:
		return (const UTF8Char*)"Set Gain";
	case DC_GET_LED_LEVEL:
		return (const UTF8Char*)"Get LED Level";
	case DC_SET_LED_LEVEL:
		return (const UTF8Char*)"Set LED Level";
	case DC_GET_LED_MODE:
		return (const UTF8Char*)"Get LED Mode";
	case DC_SET_LED_MODE:
		return (const UTF8Char*)"Set LED Mode";
	case DC_GET_OUTPUT_TIMING:
		return (const UTF8Char*)"Get Data Output Timing";
	case DC_SET_OUTPUT_TIMING:
		return (const UTF8Char*)"Set Data Output Timing";
	case DC_GET_UNREAD_DATA_SEND:
		return (const UTF8Char*)"Get Unread Code Transfer Mode";
	case DC_SET_UNREAD_DATA_SEND:
		return (const UTF8Char*)"Set Unread Code Transfer Mode";
	case DC_GET_INDIR_TIME:
		return (const UTF8Char*)"Get Indirect Mode One-shot Time";
	case DC_SET_INDIR_TIME:
		return (const UTF8Char*)"Set Indirect Mode One-shot Time";
	case DC_GET_TRIGGER_DELAY:
		return (const UTF8Char*)"Get Trigger Signal Input Delay Time";
	case DC_SET_TRIGGER_DELAY:
		return (const UTF8Char*)"Set Trigger Signal Input Delay Time";
	case DC_GET_SIGNAL_ON_DUR:
		return (const UTF8Char*)"Get Signal Output ON Time Range";
	case DC_SET_SIGNAL_ON_DUR:
		return (const UTF8Char*)"Set Signal Output ON Time Range";
	case DC_GET_SIGNAL_DELAY:
		return (const UTF8Char*)"Get Signal Output Delay Time";
	case DC_SET_SIGNAL_DELAY:
		return (const UTF8Char*)"Set Signal Output Delay Time";
	case DC_GET_LIGHT_LED:
		return (const UTF8Char*)"Get Light LED";
	case DC_SET_LIGHT_LED:
		return (const UTF8Char*)"Set Light LED";
	case DC_GET_MARKER_LIGHT:
		return (const UTF8Char*)"Get Marker Light";
	case DC_SET_MARKER_LIGHT:
		return (const UTF8Char*)"Set Marker Light";
	case DC_GET_DECODE_TIME_LIMIT:
		return (const UTF8Char*)"Get Decode Time Limit";
	case DC_SET_DECODE_TIME_LIMIT:
		return (const UTF8Char*)"Set Decode Time Limit";
	case DC_GET_OUTPUT1_TYPE:
		return (const UTF8Char*)"Get Output 1 Setting";
	case DC_SET_OUTPUT1_TYPE:
		return (const UTF8Char*)"Set Output 1 Setting";
	case DC_GET_OUTPUT2_TYPE:
		return (const UTF8Char*)"Get Output 2 Setting";
	case DC_SET_OUTPUT2_TYPE:
		return (const UTF8Char*)"Set Output 2 Setting";
	case DC_GET_AUTO_SENSE_MODE:
		return (const UTF8Char*)"Get Auto-Sense Mode Read Command";
	case DC_SET_AUTO_SENSE_MODE:
		return (const UTF8Char*)"Set Auto-Sense Mode Read Command";
	case DC_GET_CONT_READ_MODE_B:
		return (const UTF8Char*)"Get Continuous Read Mode B Read Command";
	case DC_SET_CONT_READ_MODE_B:
		return (const UTF8Char*)"Set Continuous Read Mode B Read Command";
	case DC_GET_QRCODE:
		return (const UTF8Char*)"Get QR Code Reading";
	case DC_SET_QRCODE:
		return (const UTF8Char*)"Set QR Code Reading";
	case DC_GET_MICRO_QRCODE:
		return (const UTF8Char*)"Get Micro QR Code Reading";
	case DC_SET_MICRO_QRCODE:
		return (const UTF8Char*)"Set Micro QR Code Reading";
	case DC_GET_PDF417:
		return (const UTF8Char*)"Get PDF417 Reading";
	case DC_SET_PDF417:
		return (const UTF8Char*)"Set PDF417 Reading";
	case DC_GET_DATAMATRIX:
		return (const UTF8Char*)"Get DataMatrix Reading";
	case DC_SET_DATAMATRIX:
		return (const UTF8Char*)"Set DataMatrix Reading";
	case DC_GET_BARCODE:
		return (const UTF8Char*)"Get UPC-A, UPC-E, EAN-13, EAN-8 Code Reading";
	case DC_SET_BARCODE:
		return (const UTF8Char*)"Set UPC-A, UPC-E, EAN-13, EAN-8 Code Reading";
	case DC_GET_INTERLEAVED_2OF5:
		return (const UTF8Char*)"Get Interleaved 2of5 Code Reading";
	case DC_SET_INTERLEAVED_2OF5:
		return (const UTF8Char*)"Set Interleaved 2of5 Code Reading";
	case DC_GET_CODABAR:
		return (const UTF8Char*)"Get CODEBAR(NW-7) Code Reading";
	case DC_SET_CODABAR:
		return (const UTF8Char*)"Set CODEBAR(NW-7) Code Reading";
	case DC_GET_CODABAR_START_STOP:
		return (const UTF8Char*)"Get CODEBAR(NW-7) Start/Stop Code Send";
	case DC_SET_CODEBAR_START_STOP:
		return (const UTF8Char*)"Set CODEBAR(NW-7) Start/Stop Code Send";
	case DC_GET_CODE39:
		return (const UTF8Char*)"Get CODE39 Code Reading";
	case DC_SET_CODE39:
		return (const UTF8Char*)"Set CODE39 Code Reading";
	case DC_GET_CODE128:
		return (const UTF8Char*)"Get CODE128, GSI-128 Code Reading";
	case DC_SET_CODE128:
		return (const UTF8Char*)"Set CODE128, GSI-128 Code Reading";
	case DC_GET_QRCODE_REVERSE:
		return (const UTF8Char*)"Get QR Code, Micro QR Code, SQRC, iQR, DataMatrix, iQR, Aztec Invert";
	case DC_SET_QRCODE_REVERSE:
		return (const UTF8Char*)"Set QR Code, Micro QR Code, SQRC, iQR, DataMatrix, iQR, Aztec Invert";
	case DC_GET_QRLINK_CODE:
		return (const UTF8Char*)"Get QR Link Code Mode Setting";
	case DC_SET_QRLINK_CODE:
		return (const UTF8Char*)"Set QR Link Code Mode Setting";
	case DC_GET_GS1_DATABAR:
		return (const UTF8Char*)"Get GS1 DataBar Reading";
	case DC_SET_GS1_DATABAR:
		return (const UTF8Char*)"Set GS1 DataBar Reading";
	case DC_GET_GS1_COMPOSITE:
		return (const UTF8Char*)"Get GS1 Composite Reading";
	case DC_SET_GS1_COMPOSITE:
		return (const UTF8Char*)"Set GS1 Composite Reading";
	case DC_GET_MICRO_PDF417:
		return (const UTF8Char*)"Get Micro PDF417 Reading";
	case DC_SET_MICRO_PDF417:
		return (const UTF8Char*)"Set Micro PDF417 Reading";
	case DC_GET_BARCODE_READ_MODE:
		return (const UTF8Char*)"Get Barcode Reader Mode";
	case DC_SET_BARCODE_READ_MODE:
		return (const UTF8Char*)"Set Barcode Reader Mode";
	case DC_GET_SQRC:
		return (const UTF8Char*)"Get SQRC Reading";
	case DC_SET_SQRC:
		return (const UTF8Char*)"Set SQRC Reading";
	case DC_GET_SQRC_KEY_UNMATCH:
		return (const UTF8Char*)"Get SQRC Key Match Action";
	case DC_SET_SQRC_KEY_UNMATCH:
		return (const UTF8Char*)"Set SQRC Key Match Action";
	case DC_GET_SQRC_KEY_MATCH:
		return (const UTF8Char*)"Get SQRC Key Unmatch Action";
	case DC_SET_SQRC_KEY_MATCH:
		return (const UTF8Char*)"Set SQRC Key Unmatch Action";
	case DC_GET_IQRCODE_SQUARE:
		return (const UTF8Char*)"Get iQR Code(Square) Reading";
	case DC_SET_IQRCODE_SQUARE:
		return (const UTF8Char*)"Set iQR Code(Square) Reading";
	case DC_GET_IQRCODE_RECT:
		return (const UTF8Char*)"Get iQR Code(Rectangle) Reading";
	case DC_SET_IQRCODE_RECT:
		return (const UTF8Char*)"Set iQR Code(Rectangle) Reading";
	case DC_GET_AZTEC_FULL:
		return (const UTF8Char*)"Get Aztec Code(Full-range) Reading";
	case DC_SET_AZTEC_FULL:
		return (const UTF8Char*)"Set Aztec Code(Full-range) Reading";
	case DC_GET_AZTEC_COMPACT:
		return (const UTF8Char*)"Get Aztec Code(Compact) Reading";
	case DC_SET_AZTEC_COMPACT:
		return (const UTF8Char*)"Set Aztec Code(Compact) Reading";
	case DC_GET_MENU_READ:
		return (const UTF8Char*)"Get Menu Reading";
	case DC_SET_MENU_READ:
		return (const UTF8Char*)"Set Menu Reading";
	case DC_GET_COMM_SEQ:
		return (const UTF8Char*)"Get Communication Sequence";
	case DC_SET_COMM_SEQ:
		return (const UTF8Char*)"Set Communication Sequence";
	case DC_GET_BAUD_RATE:
		return (const UTF8Char*)"Get Baud Rate";
	case DC_SET_BAUD_RATE:
		return (const UTF8Char*)"Set Baud Rate";
	case DC_GET_CODE_MARK:
		return (const UTF8Char*)"Get Code Mark Transmission";
	case DC_SET_CODE_MARK:
		return (const UTF8Char*)"Set Code Mark Transmission";
	case DC_GET_LINE_NUM:
		return (const UTF8Char*)"Get Line Number Transmission";
	case DC_SET_LINE_NUM:
		return (const UTF8Char*)"Set Line Number Transmission";
	case DC_GET_BCC:
		return (const UTF8Char*)"Get BCC Transmission";
	case DC_SET_BCC:
		return (const UTF8Char*)"Set BCC Transmission";
	case DC_GET_CTS_SIGNAL:
		return (const UTF8Char*)"Get CTS Signal Control";
	case DC_SET_CTS_SIGNAL:
		return (const UTF8Char*)"Set CTS Signal Control";
	case DC_GET_CTS_TIME:
		return (const UTF8Char*)"Get CTS Interval";
	case DC_SET_CTS_TIME:
		return (const UTF8Char*)"Set CTS Interval";
	case DC_GET_ACK_NAK_TIME:
		return (const UTF8Char*)"Get ACK/NAK Response Time";
	case DC_SET_ACK_NAK_TIME:
		return (const UTF8Char*)"Set ACK/NAK Response Time";
	case DC_GET_RECV_HDR:
		return (const UTF8Char*)"Get Receive Header";
	case DC_SET_RECV_HDR:
		return (const UTF8Char*)"Set Receive Header";
	case DC_GET_RECV_TERMINATOR:
		return (const UTF8Char*)"Get Receive Terminator";
	case DC_GET_SEND_TERMINATOR:
		return (const UTF8Char*)"Get Send Terminator";
	case DC_GET_BUZZER:
		return (const UTF8Char*)"Get Buzzer Sound";
	case DC_SET_BUZZER:
		return (const UTF8Char*)"Set Buzzer Sound";
	case DC_GET_READ_ERR_BUZZER:
		return (const UTF8Char*)"Get Read Error Buzzer";
	case DC_SET_READ_ERR_BUZZER:
		return (const UTF8Char*)"Set Read Error Buzzer";
	case DC_GET_MAGIC_KEY:
		return (const UTF8Char*)"Get Magic Key Function";
	case DC_SET_MAGIC_KEY:
		return (const UTF8Char*)"Set Magic Key Function";
	case DC_GET_POWER_ON_BUZZER:
		return (const UTF8Char*)"Get Power On Buzzer";
	case DC_SET_POWER_ON_BUZZER:
		return (const UTF8Char*)"Set Power On Buzzer";
	case DC_GET_BUZZER_OFF:
		return (const UTF8Char*)"Get Force Buzzer Off";
	case DC_SET_BUZZER_OFF:
		return (const UTF8Char*)"Set Force Buzzer Off";
	default:
		return (const UTF8Char*)"Unknown";
	}
}

IO::Device::DensoWaveQB30::CommandType IO::Device::DensoWaveQB30::GetCommandParamType(DeviceCommand dcmd, Int32 *minVal, Int32 *maxVal)
{
	switch (dcmd)
	{
	case DC_GET_READ_MODE:
		*minVal = 0;
		*maxVal = 6;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_READ_MODE:
		*minVal = 0;
		*maxVal = 6;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_BW_MODE:
		*minVal = 0;
		*maxVal = 2;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_BW_MODE:
		*minVal = 0;
		*maxVal = 2;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_READ_REPEAT_TIME:
		*minVal = 0;
		*maxVal = 99;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND;
	case DC_SET_READ_REPEAT_TIME:
		*minVal = 0;
		*maxVal = 99;
		return IO::Device::DensoWaveQB30::CT_SET_COMMAND;
	case DC_GET_BRIGHTNESS:
		*minVal = 0;
		*maxVal = 3;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_BRIGHTNESS:
		*minVal = 0;
		*maxVal = 3;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_SHT_SIGNAL:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_SHT_SIGNAL:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_SCAN_MODE:
		*minVal = 0;
		*maxVal = 2;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_SCAN_MODE:
		*minVal = 0;
		*maxVal = 2;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_SHUTTER_TIME:
		*minVal = 1;
		*maxVal = 99;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND;
	case DC_SET_SHUTTER_TIME:
		*minVal = 1;
		*maxVal = 99;
		return IO::Device::DensoWaveQB30::CT_SET_COMMAND;
	case DC_GET_GAIN:
		*minVal = 0;
		*maxVal = 15;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND;
	case DC_SET_GAIN:
		*minVal = 0;
		*maxVal = 15;
		return IO::Device::DensoWaveQB30::CT_SET_COMMAND;
	case DC_GET_LED_LEVEL:
		*minVal = 0;
		*maxVal = 2;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_LED_LEVEL:
		*minVal = 0;
		*maxVal = 2;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_LED_MODE:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_LED_MODE:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_OUTPUT_TIMING:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_OUTPUT_TIMING:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_UNREAD_DATA_SEND:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_UNREAD_DATA_SEND:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_INDIR_TIME:
		*minVal = 10;
		*maxVal = 999;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND;
	case DC_SET_INDIR_TIME:
		*minVal = 10;
		*maxVal = 999;
		return IO::Device::DensoWaveQB30::CT_SET_COMMAND;
	case DC_GET_TRIGGER_DELAY:
		*minVal = 0;
		*maxVal = 999;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND;
	case DC_SET_TRIGGER_DELAY:
		*minVal = 0;
		*maxVal = 999;
		return IO::Device::DensoWaveQB30::CT_SET_COMMAND;
	case DC_GET_SIGNAL_ON_DUR:
		*minVal = 0;
		*maxVal = 255;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND;
	case DC_SET_SIGNAL_ON_DUR:
		*minVal = 0;
		*maxVal = 255;
		return IO::Device::DensoWaveQB30::CT_SET_COMMAND;
	case DC_GET_SIGNAL_DELAY:
		*minVal = 0;
		*maxVal = 99;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND;
	case DC_SET_SIGNAL_DELAY:
		*minVal = 0;
		*maxVal = 99;
		return IO::Device::DensoWaveQB30::CT_SET_COMMAND;
	case DC_GET_LIGHT_LED:
		*minVal = 0;
		*maxVal = 2;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_LIGHT_LED:
		*minVal = 0;
		*maxVal = 2;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_MARKER_LIGHT:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_MARKER_LIGHT:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_DECODE_TIME_LIMIT:
		*minVal = 0;
		*maxVal = 999;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND;
	case DC_SET_DECODE_TIME_LIMIT:
		*minVal = 0;
		*maxVal = 999;
		return IO::Device::DensoWaveQB30::CT_SET_COMMAND;
	case DC_GET_OUTPUT1_TYPE:
		*minVal = 0;
		*maxVal = 5;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_OUTPUT1_TYPE:
		*minVal = 0;
		*maxVal = 5;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_OUTPUT2_TYPE:
		*minVal = 0;
		*maxVal = 6;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_OUTPUT2_TYPE:
		*minVal = 0;
		*maxVal = 6;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_AUTO_SENSE_MODE:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_AUTO_SENSE_MODE:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_CONT_READ_MODE_B:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_CONT_READ_MODE_B:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_QRCODE:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_QRCODE:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_MICRO_QRCODE:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_MICRO_QRCODE:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_PDF417:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_PDF417:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_DATAMATRIX:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_DATAMATRIX:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_BARCODE:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_BARCODE:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_INTERLEAVED_2OF5:
		*minVal = 0;
		*maxVal = 3;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_INTERLEAVED_2OF5:
		*minVal = 0;
		*maxVal = 3;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_CODABAR:
		*minVal = 0;
		*maxVal = 3;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_CODABAR:
		*minVal = 0;
		*maxVal = 3;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_CODABAR_START_STOP:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_CODEBAR_START_STOP:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_CODE39:
		*minVal = 0;
		*maxVal = 3;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_CODE39:
		*minVal = 0;
		*maxVal = 3;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_CODE128:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_CODE128:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_QRCODE_REVERSE:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_QRCODE_REVERSE:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_QRLINK_CODE:
		*minVal = 0;
		*maxVal = 2;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_QRLINK_CODE:
		*minVal = 0;
		*maxVal = 2;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_GS1_DATABAR:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_GS1_DATABAR:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_GS1_COMPOSITE:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_GS1_COMPOSITE:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_MICRO_PDF417:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_MICRO_PDF417:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_BARCODE_READ_MODE:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_BARCODE_READ_MODE:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_SQRC:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_SQRC:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_SQRC_KEY_UNMATCH:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_SQRC_KEY_UNMATCH:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_SQRC_KEY_MATCH:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_SQRC_KEY_MATCH:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_IQRCODE_SQUARE:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_IQRCODE_SQUARE:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_IQRCODE_RECT:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_IQRCODE_RECT:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_AZTEC_FULL:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_AZTEC_FULL:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_AZTEC_COMPACT:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_AZTEC_COMPACT:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_MENU_READ:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_MENU_READ:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_COMM_SEQ:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_COMM_SEQ:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_BAUD_RATE:
		*minVal = 0;
		*maxVal = 5;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_BAUD_RATE:
		*minVal = 0;
		*maxVal = 5;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_CODE_MARK:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_CODE_MARK:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_LINE_NUM:
		*minVal = 0;
		*maxVal = 2;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_LINE_NUM:
		*minVal = 0;
		*maxVal = 2;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_BCC:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_BCC:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_CTS_SIGNAL:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_CTS_SIGNAL:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_CTS_TIME:
		*minVal = 1;
		*maxVal = 99;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND;
	case DC_SET_CTS_TIME:
		*minVal = 1;
		*maxVal = 99;
		return IO::Device::DensoWaveQB30::CT_SET_COMMAND;
	case DC_GET_ACK_NAK_TIME:
		*minVal = 1;
		*maxVal = 99;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND;
	case DC_SET_ACK_NAK_TIME:
		*minVal = 1;
		*maxVal = 99;
		return IO::Device::DensoWaveQB30::CT_SET_COMMAND;
	case DC_GET_RECV_HDR:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_RECV_HDR:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_RECV_TERMINATOR:
		*minVal = 0;
		*maxVal = 3;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_GET_SEND_TERMINATOR:
		*minVal = 0;
		*maxVal = 5;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_GET_BUZZER:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_BUZZER:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_READ_ERR_BUZZER:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_READ_ERR_BUZZER:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_MAGIC_KEY:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_MAGIC_KEY:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_POWER_ON_BUZZER:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_POWER_ON_BUZZER:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	case DC_GET_BUZZER_OFF:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_GET_COMMAND_NAME;
	case DC_SET_BUZZER_OFF:
		*minVal = 0;
		*maxVal = 1;
		return IO::Device::DensoWaveQB30::CT_SELECT_COMMAND;
	default:
		return IO::Device::DensoWaveQB30::CT_UNKNOWN;
	}
}

const UTF8Char *IO::Device::DensoWaveQB30::GetCommandParamName(DeviceCommand dcmd, Int32 cmdVal)
{
	switch (dcmd)
	{
	case DC_GET_READ_MODE:
	case DC_SET_READ_MODE:
		if (cmdVal == -1)
			return (const UTF8Char*)"Unknown";
		if (cmdVal == 0)
			return (const UTF8Char*)"Continuous Read Mode A";
		if (cmdVal == 1)
			return (const UTF8Char*)"Continuous Read Mode B";
		if (cmdVal == 2)
			return (const UTF8Char*)"Direct Trigger Mode";
		if (cmdVal == 3)
			return (const UTF8Char*)"Indirect Trigger Mode";
		if (cmdVal == 4)
			return (const UTF8Char*)"Direct Software Trigger Mode";
		if (cmdVal == 5)
			return (const UTF8Char*)"Indirect Software Trigger Mode";
		if (cmdVal == 6)
			return (const UTF8Char*)"Auto Sense Mode";
		return 0;
	case DC_GET_BW_MODE:
	case DC_SET_BW_MODE:
		if (cmdVal == 0)
			return (const UTF8Char*)"Normal Code";
		if (cmdVal == 1)
			return (const UTF8Char*)"Inverted Code";
		if (cmdVal == 2)
			return (const UTF8Char*)"Auto Detect";
		return 0;
	case DC_GET_READ_REPEAT_TIME:
	case DC_SET_READ_REPEAT_TIME:
		return 0;
	case DC_GET_BRIGHTNESS:
	case DC_SET_BRIGHTNESS:
		if (cmdVal == 0)
			return (const UTF8Char*)"Auto";
		if (cmdVal == 1)
			return (const UTF8Char*)"Fixed Shutter, Auto Gain";
		if (cmdVal == 2)
			return (const UTF8Char*)"Fixed Shutter, Fixed Gain";
		return 0;
	case DC_GET_SHT_SIGNAL:
	case DC_SET_SHT_SIGNAL:
		if (cmdVal == 0)
			return (const UTF8Char*)"Tr OFF";
		if (cmdVal == 1)
			return (const UTF8Char*)"Tr ON";
		return 0;
	case DC_GET_SCAN_MODE:
	case DC_SET_SCAN_MODE:
		if (cmdVal == 0)
			return (const UTF8Char*)"Capture Mode Off";
		if (cmdVal == 1)
			return (const UTF8Char*)"Scan Entry Mode";
		if (cmdVal == 2)
			return (const UTF8Char*)"File Entry Mode";
		return 0;
	case DC_GET_SHUTTER_TIME:
	case DC_SET_SHUTTER_TIME:
		return 0;
	case DC_GET_GAIN:
	case DC_SET_GAIN:
		return 0;
	case DC_GET_LED_LEVEL:
	case DC_SET_LED_LEVEL:
		if (cmdVal == 0)
			return (const UTF8Char*)"Strong";
		if (cmdVal == 1)
			return (const UTF8Char*)"Medium";
		if (cmdVal == 2)
			return (const UTF8Char*)"Weak";
		return 0;
	case DC_GET_LED_MODE:
	case DC_SET_LED_MODE:
		if (cmdVal == 0)
			return (const UTF8Char*)"LED Off";
		if (cmdVal == 1)
			return (const UTF8Char*)"LED On";
		return 0;
	case DC_GET_OUTPUT_TIMING:
	case DC_SET_OUTPUT_TIMING:
		if (cmdVal == 0)
			return (const UTF8Char*)"Sync";
		if (cmdVal == 1)
			return (const UTF8Char*)"Async";
		return 0;
	case DC_GET_UNREAD_DATA_SEND:
	case DC_SET_UNREAD_DATA_SEND:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Disallow";
		return 0;
	case DC_GET_INDIR_TIME:
	case DC_SET_INDIR_TIME:
		return 0;
	case DC_GET_TRIGGER_DELAY:
	case DC_SET_TRIGGER_DELAY:
		return 0;
	case DC_GET_SIGNAL_ON_DUR:
	case DC_SET_SIGNAL_ON_DUR:
		return 0;
	case DC_GET_SIGNAL_DELAY:
	case DC_SET_SIGNAL_DELAY:
		return 0;
	case DC_GET_LIGHT_LED:
	case DC_SET_LIGHT_LED:
		if (cmdVal == 0)
			return (const UTF8Char*)"Always Off";
		if (cmdVal == 1)
			return (const UTF8Char*)"Auto";
		if (cmdVal == 2)
			return (const UTF8Char*)"Always On";
		return 0;
	case DC_GET_MARKER_LIGHT:
	case DC_SET_MARKER_LIGHT:
		if (cmdVal == 0)
			return (const UTF8Char*)"Always Off";
		if (cmdVal == 1)
			return (const UTF8Char*)"Normal";
		return 0;
	case DC_GET_DECODE_TIME_LIMIT:
	case DC_SET_DECODE_TIME_LIMIT:
		return 0;
	case DC_GET_OUTPUT1_TYPE:
	case DC_SET_OUTPUT1_TYPE:
		if (cmdVal == 0)
			return (const UTF8Char*)"Unused";
		if (cmdVal == 1)
			return (const UTF8Char*)"Output OK Signal";
		if (cmdVal == 2)
			return (const UTF8Char*)"Output NG Signal";
		if (cmdVal == 3)
			return (const UTF8Char*)"Output Scan OK Signal";
		if (cmdVal == 4)
			return (const UTF8Char*)"Output Scan NG Signal";
		if (cmdVal == 5)
			return (const UTF8Char*)"Output READY Signal";
		return 0;
	case DC_GET_OUTPUT2_TYPE:
	case DC_SET_OUTPUT2_TYPE:
		if (cmdVal == 0)
			return (const UTF8Char*)"Unused";
		if (cmdVal == 1)
			return (const UTF8Char*)"Output OK Signal";
		if (cmdVal == 2)
			return (const UTF8Char*)"Output NG Signal";
		if (cmdVal == 3)
			return (const UTF8Char*)"Output Scan OK Signal";
		if (cmdVal == 4)
			return (const UTF8Char*)"Output Scan NG Signal";
		if (cmdVal == 5)
			return (const UTF8Char*)"Output READY Signal";
		if (cmdVal == 6)
			return (const UTF8Char*)"Output SHT Signal";
		return 0;
	case DC_GET_AUTO_SENSE_MODE:
	case DC_SET_AUTO_SENSE_MODE:
		if (cmdVal == 0)
			return (const UTF8Char*)"Disable";
		if (cmdVal == 1)
			return (const UTF8Char*)"Enable";
		return 0;
	case DC_GET_CONT_READ_MODE_B:
	case DC_SET_CONT_READ_MODE_B:
		if (cmdVal == 0)
			return (const UTF8Char*)"Disable";
		if (cmdVal == 1)
			return (const UTF8Char*)"Enable";
		return 0;
	case DC_GET_QRCODE:
	case DC_SET_QRCODE:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Disallow";
		return 0;
	case DC_GET_MICRO_QRCODE:
	case DC_SET_MICRO_QRCODE:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Disallow";
		return 0;
	case DC_GET_PDF417:
	case DC_SET_PDF417:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Disallow";
		return 0;
	case DC_GET_DATAMATRIX:
	case DC_SET_DATAMATRIX:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Disallow";
		return 0;
	case DC_GET_BARCODE:
	case DC_SET_BARCODE:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Disallow";
		return 0;
	case DC_GET_INTERLEAVED_2OF5:
	case DC_SET_INTERLEAVED_2OF5:
		if (cmdVal == 0)
			return (const UTF8Char*)"No C/D, Allow read";
		if (cmdVal == 1)
			return (const UTF8Char*)"Has C/D, Allow read, C/D Transfer Allowed";
		if (cmdVal == 2)
			return (const UTF8Char*)"Has C/D, Allow read, C/D Transfer not Allowed";
		if (cmdVal == 3)
			return (const UTF8Char*)"Read not allowed";
		return 0;
	case DC_GET_CODABAR:
	case DC_SET_CODABAR:
		if (cmdVal == 0)
			return (const UTF8Char*)"No C/D, Allow read";
		if (cmdVal == 1)
			return (const UTF8Char*)"Has C/D, Allow read, C/D Transfer Allowed";
		if (cmdVal == 2)
			return (const UTF8Char*)"Has C/D, Allow read, C/D Transfer not Allowed";
		if (cmdVal == 3)
			return (const UTF8Char*)"Read not allowed";
		return 0;
	case DC_GET_CODABAR_START_STOP:
	case DC_SET_CODEBAR_START_STOP:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Disallow";
		return 0;
	case DC_GET_CODE39:
	case DC_SET_CODE39:
		if (cmdVal == 0)
			return (const UTF8Char*)"No C/D, Allow read";
		if (cmdVal == 1)
			return (const UTF8Char*)"Has C/D, Allow read, C/D Transfer Allowed";
		if (cmdVal == 2)
			return (const UTF8Char*)"Has C/D, Allow read, C/D Transfer not Allowed";
		if (cmdVal == 3)
			return (const UTF8Char*)"Read not allowed";
		return 0;
	case DC_GET_CODE128:
	case DC_SET_CODE128:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Disallow";
		return 0;
	case DC_GET_QRCODE_REVERSE:
	case DC_SET_QRCODE_REVERSE:
		if (cmdVal == 0)
			return (const UTF8Char*)"Normal Code";
		if (cmdVal == 1)
			return (const UTF8Char*)"Inverted Code";
		return 0;
	case DC_GET_QRLINK_CODE:
	case DC_SET_QRLINK_CODE:
		if (cmdVal == 0)
			return (const UTF8Char*)"Non-editing Mode";
		if (cmdVal == 1)
			return (const UTF8Char*)"All Editing Mode";
		if (cmdVal == 1)
			return (const UTF8Char*)"Editing Mode";
		return 0;
	case DC_GET_GS1_DATABAR:
	case DC_SET_GS1_DATABAR:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Disallow";
		return 0;
	case DC_GET_GS1_COMPOSITE:
	case DC_SET_GS1_COMPOSITE:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Disallow";
		return 0;
	case DC_GET_MICRO_PDF417:
	case DC_SET_MICRO_PDF417:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Disallow";
		return 0;
	case DC_GET_BARCODE_READ_MODE:
	case DC_SET_BARCODE_READ_MODE:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Disallow";
		return 0;
	case DC_GET_SQRC:
	case DC_SET_SQRC:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Disallow";
		return 0;
	case DC_GET_SQRC_KEY_UNMATCH:
	case DC_SET_SQRC_KEY_UNMATCH:
		if (cmdVal == 0)
			return (const UTF8Char*)"Reading not allowed";
		if (cmdVal == 1)
			return (const UTF8Char*)"Only Output Public Data";
		return 0;
	case DC_GET_SQRC_KEY_MATCH:
	case DC_SET_SQRC_KEY_MATCH:
		if (cmdVal == 0)
			return (const UTF8Char*)"Output Public Data+Private Data";
		if (cmdVal == 1)
			return (const UTF8Char*)"Only Output Private Data";
		return 0;
	case DC_GET_IQRCODE_SQUARE:
	case DC_SET_IQRCODE_SQUARE:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Disallow";
		return 0;
	case DC_GET_IQRCODE_RECT:
	case DC_SET_IQRCODE_RECT:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Disallow";
		return 0;
	case DC_GET_AZTEC_FULL:
	case DC_SET_AZTEC_FULL:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Disallow";
		return 0;
	case DC_GET_AZTEC_COMPACT:
	case DC_SET_AZTEC_COMPACT:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Disallow";
		return 0;
	case DC_GET_MENU_READ:
	case DC_SET_MENU_READ:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Disallow";
		return 0;
	case DC_GET_COMM_SEQ:
	case DC_SET_COMM_SEQ:
		if (cmdVal == 0)
			return (const UTF8Char*)"No Protocol";
		if (cmdVal == 1)
			return (const UTF8Char*)"ACK/NCK";
		return 0;
	case DC_GET_BAUD_RATE:
	case DC_SET_BAUD_RATE:
		if (cmdVal == 0)
			return (const UTF8Char*)"4800bps";
		if (cmdVal == 1)
			return (const UTF8Char*)"9600bps";
		if (cmdVal == 2)
			return (const UTF8Char*)"19200bps";
		if (cmdVal == 3)
			return (const UTF8Char*)"38400bps";
		if (cmdVal == 4)
			return (const UTF8Char*)"57600bps";
		if (cmdVal == 5)
			return (const UTF8Char*)"115200bps";
		return 0;
	case DC_GET_CODE_MARK:
	case DC_SET_CODE_MARK:
		if (cmdVal == 0)
			return (const UTF8Char*)"Not Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Allow";
		return 0;
	case DC_GET_LINE_NUM:
	case DC_SET_LINE_NUM:
		if (cmdVal == 0)
			return (const UTF8Char*)"Not Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Transfer in 2 Lines";
		if (cmdVal == 2)
			return (const UTF8Char*)"Transfer in 4 Lines";
		return 0;
	case DC_GET_BCC:
	case DC_SET_BCC:
		if (cmdVal == 0)
			return (const UTF8Char*)"Not Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Allow";
		return 0;
	case DC_GET_CTS_SIGNAL:
	case DC_SET_CTS_SIGNAL:
		if (cmdVal == 0)
			return (const UTF8Char*)"No Control";
		if (cmdVal == 1)
			return (const UTF8Char*)"Has Control";
		return 0;
	case DC_GET_CTS_TIME:
	case DC_SET_CTS_TIME:
		return 0;
	case DC_GET_ACK_NAK_TIME:
	case DC_SET_ACK_NAK_TIME:
		return 0;
	case DC_GET_RECV_HDR:
	case DC_SET_RECV_HDR:
		if (cmdVal == 0)
			return (const UTF8Char*)"No";
		if (cmdVal == 1)
			return (const UTF8Char*)"STX";
		return 0;
	case DC_GET_RECV_TERMINATOR:
		if (cmdVal == 0)
			return (const UTF8Char*)"ETX";
		if (cmdVal == 1)
			return (const UTF8Char*)"CR";
		if (cmdVal == 2)
			return (const UTF8Char*)"LF";
		if (cmdVal == 3)
			return (const UTF8Char*)"CR+LF";
		return 0;
	case DC_GET_SEND_TERMINATOR:
		if (cmdVal == 0)
			return (const UTF8Char*)"No";
		if (cmdVal == 1)
			return (const UTF8Char*)"ETX";
		if (cmdVal == 2)
			return (const UTF8Char*)"CR";
		if (cmdVal == 3)
			return (const UTF8Char*)"LF";
		if (cmdVal == 4)
			return (const UTF8Char*)"CR+LF";
		if (cmdVal == 5)
			return (const UTF8Char*)"Custom";
		return 0;
	case DC_GET_BUZZER:
	case DC_SET_BUZZER:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Not Allow";
		return 0;
	case DC_GET_READ_ERR_BUZZER:
	case DC_SET_READ_ERR_BUZZER:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Not Allow";
		return 0;
	case DC_GET_MAGIC_KEY:
	case DC_SET_MAGIC_KEY:
		if (cmdVal == 0)
			return (const UTF8Char*)"No Function";
		if (cmdVal == 1)
			return (const UTF8Char*)"Marker Switch Mode";
		return 0;
	case DC_GET_POWER_ON_BUZZER:
	case DC_SET_POWER_ON_BUZZER:
		if (cmdVal == 0)
			return (const UTF8Char*)"Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Not Allow";
		return 0;
	case DC_GET_BUZZER_OFF:
	case DC_SET_BUZZER_OFF:
		if (cmdVal == 0)
			return (const UTF8Char*)"Not Allow";
		if (cmdVal == 1)
			return (const UTF8Char*)"Allow";
		return 0;
	default:
		return 0;
	}
}

Int32 IO::Device::DensoWaveQB30::GetCommand(DeviceCommand dcmd)
{
	switch (dcmd)
	{
	case DC_GET_READ_MODE:
		return this->ReadCommand("RD01\r", 5);
	case DC_GET_BW_MODE:
		return this->ReadCommand("RD02\r", 5);
	case DC_GET_READ_REPEAT_TIME:
		return this->ReadCommand("RD04\r", 5);
	case DC_GET_BRIGHTNESS:
		return this->ReadCommand("RD07\r", 5);
	case DC_GET_SHT_SIGNAL:
		return this->ReadCommand("RD09\r", 5);
	case DC_GET_SCAN_MODE:
		return this->ReadCommand("RD0B\r", 5);
	case DC_GET_SHUTTER_TIME:
		return this->ReadCommand("RD0G\r", 5);
	case DC_GET_GAIN:
		return this->ReadCommand("RD0K\r", 5);
	case DC_GET_LED_LEVEL:
		return this->ReadCommand("RD0L\r", 5);
	case DC_GET_LED_MODE:
		return this->ReadCommand("RD0M\r", 5);
	case DC_GET_OUTPUT_TIMING:
		return this->ReadCommand("RD11\r", 5);
	case DC_GET_UNREAD_DATA_SEND:
		return this->ReadCommand("RD12\r", 5);
	case DC_GET_INDIR_TIME:
		return this->ReadCommand("RD15\r", 5);
	case DC_GET_TRIGGER_DELAY:
		return this->ReadCommand("RD16\r", 5);
	case DC_GET_SIGNAL_ON_DUR:
		return this->ReadCommand("RD17\r", 5);
	case DC_GET_SIGNAL_DELAY:
		return this->ReadCommand("RD18\r", 5);
	case DC_GET_LIGHT_LED:
		return this->ReadCommand("RD1C\r", 5);
	case DC_GET_MARKER_LIGHT:
		return this->ReadCommand("RD1J\r", 5);
	case DC_GET_DECODE_TIME_LIMIT:
		return this->ReadCommand("RD1D\r", 5);
	case DC_GET_OUTPUT1_TYPE:
		return this->ReadCommand("RD1E\r", 5);
	case DC_GET_OUTPUT2_TYPE:
		return this->ReadCommand("RD1G\r", 5);
	case DC_GET_AUTO_SENSE_MODE:
		return this->ReadCommand("RD1H\r", 5);
	case DC_GET_CONT_READ_MODE_B:
		return this->ReadCommand("RD1I\r", 5);
	case DC_GET_QRCODE:
		return this->ReadCommand("RD41\r", 5);
	case DC_GET_MICRO_QRCODE:
		return this->ReadCommand("RD42\r", 5);
	case DC_GET_PDF417:
		return this->ReadCommand("RD43\r", 5);
	case DC_GET_DATAMATRIX:
		return this->ReadCommand("RD44\r", 5);
	case DC_GET_BARCODE:
		return this->ReadCommand("RD45\r", 5);
	case DC_GET_INTERLEAVED_2OF5:
		return this->ReadCommand("RD46\r", 5);
	case DC_GET_CODABAR:
		return this->ReadCommand("RD47\r", 5);
	case DC_GET_CODABAR_START_STOP:
		return this->ReadCommand("RD48\r", 5);
	case DC_GET_CODE39:
		return this->ReadCommand("RD49\r", 5);
	case DC_GET_CODE128:
		return this->ReadCommand("RD4A\r", 5);
	case DC_GET_QRCODE_REVERSE:
		return this->ReadCommand("RD4B\r", 5);
	case DC_GET_QRLINK_CODE:
		return this->ReadCommand("RD4F\r", 5);
	case DC_GET_GS1_DATABAR:
		return this->ReadCommand("RD4I\r", 5);
	case DC_GET_GS1_COMPOSITE:
		return this->ReadCommand("RD4J\r", 5);
	case DC_GET_MICRO_PDF417:
		return this->ReadCommand("RD4K\r", 5);
	case DC_GET_BARCODE_READ_MODE:
		return this->ReadCommand("RD4L\r", 5);
	case DC_GET_SQRC:
		return this->ReadCommand("RD4M\r", 5);
	case DC_GET_SQRC_KEY_UNMATCH:
		return this->ReadCommand("RD4P\r", 5);
	case DC_GET_SQRC_KEY_MATCH:
		return this->ReadCommand("RD4O\r", 5);
	case DC_GET_IQRCODE_SQUARE:
		return this->ReadCommand("RD4Q\r", 5);
	case DC_GET_IQRCODE_RECT:
		return this->ReadCommand("RD4R\r", 5);
	case DC_GET_AZTEC_FULL:
		return this->ReadCommand("RD4V\r", 5);
	case DC_GET_AZTEC_COMPACT:
		return this->ReadCommand("RD4W\r", 5);
	case DC_GET_MENU_READ:
		return this->ReadCommand("RD4Z\r", 5);
	case DC_GET_COMM_SEQ:
		return this->ReadCommand("RD21\r", 5);
	case DC_GET_BAUD_RATE:
		return this->ReadCommand("RD22\r", 5);
	case DC_GET_CODE_MARK:
		return this->ReadCommand("RD25\r", 5);
	case DC_GET_LINE_NUM:
		return this->ReadCommand("RD26\r", 5);
	case DC_GET_BCC:
		return this->ReadCommand("RD27\r", 5);
	case DC_GET_CTS_SIGNAL:
		return this->ReadCommand("RD28\r", 5);
	case DC_GET_CTS_TIME:
		return this->ReadCommand("RD29\r", 5);
	case DC_GET_ACK_NAK_TIME:
		return this->ReadCommand("RD2A\r", 5);
	case DC_GET_RECV_HDR:
		return this->ReadCommand("RD2C\r", 5);
	case DC_GET_RECV_TERMINATOR:
		return this->ReadCommand("RD2D\r", 5);
	case DC_GET_SEND_TERMINATOR:
		return this->ReadCommand("RD2F\r", 5);
	case DC_GET_BUZZER:
		return this->ReadCommand("RD33\r", 5);
	case DC_GET_READ_ERR_BUZZER:
		return this->ReadCommand("RD35\r", 5);
	case DC_GET_MAGIC_KEY:
		return this->ReadCommand("RD37\r", 5);
	case DC_GET_POWER_ON_BUZZER:
		return this->ReadCommand("RD38\r", 5);
	case DC_GET_BUZZER_OFF:
		return this->ReadCommand("RD39\r", 5);
	case DC_SET_READ_MODE:
	case DC_SET_BW_MODE:
	case DC_SET_READ_REPEAT_TIME:
	case DC_SET_BRIGHTNESS:
	case DC_SET_SHT_SIGNAL:
	case DC_SET_SCAN_MODE:
	case DC_SET_SHUTTER_TIME:
	case DC_SET_GAIN:
	case DC_SET_LED_LEVEL:
	case DC_SET_LED_MODE:
	case DC_SET_OUTPUT_TIMING:
	case DC_SET_UNREAD_DATA_SEND:
	case DC_SET_INDIR_TIME:
	case DC_SET_TRIGGER_DELAY:
	case DC_SET_SIGNAL_ON_DUR:
	case DC_SET_SIGNAL_DELAY:
	case DC_SET_LIGHT_LED:
	case DC_SET_MARKER_LIGHT:
	case DC_SET_DECODE_TIME_LIMIT:
	case DC_SET_OUTPUT1_TYPE:
	case DC_SET_OUTPUT2_TYPE:
	case DC_SET_AUTO_SENSE_MODE:
	case DC_SET_CONT_READ_MODE_B:
	case DC_SET_QRCODE:
	case DC_SET_MICRO_QRCODE:
	case DC_SET_PDF417:
	case DC_SET_DATAMATRIX:
	case DC_SET_BARCODE:
	case DC_SET_INTERLEAVED_2OF5:
	case DC_SET_CODABAR:
	case DC_SET_CODEBAR_START_STOP:
	case DC_SET_CODE39:
	case DC_SET_CODE128:
	case DC_SET_QRCODE_REVERSE:
	case DC_SET_QRLINK_CODE:
	case DC_SET_GS1_DATABAR:
	case DC_SET_GS1_COMPOSITE:
	case DC_SET_MICRO_PDF417:
	case DC_SET_BARCODE_READ_MODE:
	case DC_SET_SQRC:
	case DC_SET_SQRC_KEY_UNMATCH:
	case DC_SET_SQRC_KEY_MATCH:
	case DC_SET_IQRCODE_SQUARE:
	case DC_SET_IQRCODE_RECT:
	case DC_SET_AZTEC_FULL:
	case DC_SET_AZTEC_COMPACT:
	case DC_SET_MENU_READ:
	case DC_SET_COMM_SEQ:
	case DC_SET_BAUD_RATE:
	case DC_SET_CODE_MARK:
	case DC_SET_LINE_NUM:
	case DC_SET_BCC:
	case DC_SET_CTS_SIGNAL:
	case DC_SET_CTS_TIME:
	case DC_SET_ACK_NAK_TIME:
	case DC_SET_RECV_HDR:
	case DC_SET_BUZZER:
	case DC_SET_READ_ERR_BUZZER:
	case DC_SET_MAGIC_KEY:
	case DC_SET_POWER_ON_BUZZER:
	case DC_SET_BUZZER_OFF:
	default:
		return -1;
	}
	return -1;
}

Bool IO::Device::DensoWaveQB30::SetCommand(DeviceCommand dcmd, Int32 val)
{
	Char cbuff[12];
	Char *csptr;
	switch (dcmd)
	{
	case DC_SET_READ_MODE:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR01#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_BW_MODE:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR02#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_READ_REPEAT_TIME:
		if (val < 10)
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR04#0"), val), "\r");
		else
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR04#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_BRIGHTNESS:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR07#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_SHT_SIGNAL:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR09#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_SCAN_MODE:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR0B#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_SHUTTER_TIME:
		if (val < 10)
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR0G#0"), val), "\r");
		else
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR0G#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_GAIN:
		if (val < 10)
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR0K#0"), val), "\r");
		else
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR0K#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_LED_LEVEL:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR0L#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_LED_MODE:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR0M#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_OUTPUT_TIMING:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR11#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_UNREAD_DATA_SEND:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR12#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_INDIR_TIME:
		if (val < 10)
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR15#00"), val), "\r");
		else if (val < 100)
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR15#0"), val), "\r");
		else
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR15#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_TRIGGER_DELAY:
		if (val < 10)
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR16#00"), val), "\r");
		else if (val < 100)
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR16#0"), val), "\r");
		else
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR16#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_SIGNAL_ON_DUR:
		if (val < 10)
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR17#00"), val), "\r");
		else if (val < 100)
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR17#0"), val), "\r");
		else
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR17#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_SIGNAL_DELAY:
		if (val < 10)
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR18#0"), val), "\r");
		else
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR18#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_LIGHT_LED:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR1C#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_MARKER_LIGHT:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR1J#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_DECODE_TIME_LIMIT:
		if (val < 10)
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR1D#00"), val), "\r");
		else if (val < 100)
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR1D#0"), val), "\r");
		else
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR1D#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_OUTPUT1_TYPE:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR1E#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_OUTPUT2_TYPE:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR1G#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_AUTO_SENSE_MODE:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR1H#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_CONT_READ_MODE_B:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR1I#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_QRCODE:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR41#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_MICRO_QRCODE:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR42#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_PDF417:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR43#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_DATAMATRIX:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR44#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_BARCODE:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR45#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_INTERLEAVED_2OF5:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR46#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_CODABAR:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR47#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_CODEBAR_START_STOP:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR48#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_CODE39:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR49#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_CODE128:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR4A#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_QRCODE_REVERSE:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR4B#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_QRLINK_CODE:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR4F#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_GS1_DATABAR:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR4I#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_GS1_COMPOSITE:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR4J#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_MICRO_PDF417:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR4K#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_BARCODE_READ_MODE:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR4L#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_SQRC:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR4M#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_SQRC_KEY_UNMATCH:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR4P#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_SQRC_KEY_MATCH:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR4O#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_IQRCODE_SQUARE:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR4Q#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_IQRCODE_RECT:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR4R#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_AZTEC_FULL:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR4V#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_AZTEC_COMPACT:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR4W#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_MENU_READ:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR4Z#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_COMM_SEQ:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR21#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_BAUD_RATE:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR22#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_CODE_MARK:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR25#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_LINE_NUM:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR26#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_BCC:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR27#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_CTS_SIGNAL:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR28#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_CTS_TIME:
		if (val < 10)
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR29#0"), val), "\r");
		else
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR29#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_ACK_NAK_TIME:
		if (val < 10)
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR2A#0"), val), "\r");
		else
			csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR2A#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_RECV_HDR:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR2C#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_BUZZER:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR33#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_READ_ERR_BUZZER:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR35#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_MAGIC_KEY:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR37#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_POWER_ON_BUZZER:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR38#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_SET_BUZZER_OFF:
		csptr = Text::StrConcat(Text::StrInt32(Text::StrConcat(cbuff, "WR39#"), val), "\r");
		return this->WriteCommand(cbuff, (UOSInt)(csptr - cbuff));
	case DC_GET_READ_MODE:
	case DC_GET_BW_MODE:
	case DC_GET_READ_REPEAT_TIME:
	case DC_GET_BRIGHTNESS:
	case DC_GET_SHT_SIGNAL:
	case DC_GET_SCAN_MODE:
	case DC_GET_SHUTTER_TIME:
	case DC_GET_GAIN:
	case DC_GET_LED_LEVEL:
	case DC_GET_LED_MODE:
	case DC_GET_OUTPUT_TIMING:
	case DC_GET_UNREAD_DATA_SEND:
	case DC_GET_INDIR_TIME:
	case DC_GET_TRIGGER_DELAY:
	case DC_GET_SIGNAL_ON_DUR:
	case DC_GET_SIGNAL_DELAY:
	case DC_GET_LIGHT_LED:
	case DC_GET_MARKER_LIGHT:
	case DC_GET_DECODE_TIME_LIMIT:
	case DC_GET_OUTPUT1_TYPE:
	case DC_GET_OUTPUT2_TYPE:
	case DC_GET_AUTO_SENSE_MODE:
	case DC_GET_CONT_READ_MODE_B:
	case DC_GET_QRCODE:
	case DC_GET_MICRO_QRCODE:
	case DC_GET_PDF417:
	case DC_GET_DATAMATRIX:
	case DC_GET_BARCODE:
	case DC_GET_INTERLEAVED_2OF5:
	case DC_GET_CODABAR:
	case DC_GET_CODABAR_START_STOP:
	case DC_GET_CODE39:
	case DC_GET_CODE128:
	case DC_GET_QRCODE_REVERSE:
	case DC_GET_QRLINK_CODE:
	case DC_GET_GS1_DATABAR:
	case DC_GET_GS1_COMPOSITE:
	case DC_GET_MICRO_PDF417:
	case DC_GET_BARCODE_READ_MODE:
	case DC_GET_SQRC:
	case DC_GET_SQRC_KEY_UNMATCH:
	case DC_GET_SQRC_KEY_MATCH:
	case DC_GET_IQRCODE_SQUARE:
	case DC_GET_IQRCODE_RECT:
	case DC_GET_AZTEC_FULL:
	case DC_GET_AZTEC_COMPACT:
	case DC_GET_MENU_READ:
	case DC_GET_COMM_SEQ:
	case DC_GET_BAUD_RATE:
	case DC_GET_CODE_MARK:
	case DC_GET_LINE_NUM:
	case DC_GET_BCC:
	case DC_GET_CTS_SIGNAL:
	case DC_GET_CTS_TIME:
	case DC_GET_ACK_NAK_TIME:
	case DC_GET_RECV_HDR:
	case DC_GET_RECV_TERMINATOR:
	case DC_GET_SEND_TERMINATOR:
	case DC_GET_BUZZER:
	case DC_GET_READ_ERR_BUZZER:
	case DC_GET_MAGIC_KEY:
	case DC_GET_POWER_ON_BUZZER:
	case DC_GET_BUZZER_OFF:
	default:
		return false;
	}
}
