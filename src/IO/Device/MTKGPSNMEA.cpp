#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/DateTime.h"
#include "IO/FileStream.h"
#include "IO/SerialPort.h"
#include "IO/Device/MTKGPSNMEA.h"
#include "Sync/Thread.h"
#include "Text/MyString.h"

void IO::Device::MTKGPSNMEA::ParseUnknownCmd(const UTF8Char *cmd)
{
	if (Text::StrStartsWith(cmd, (const UTF8Char*)"$PMTK"))
	{
		if (Text::StrStartsWith(cmd, (const UTF8Char*)"$PMTK011,"))
		{
		}
		else if (Text::StrStartsWith(cmd, (const UTF8Char*)"$PMTK010,"))
		{
		}
		else
		{
			this->cmdResultMut->Lock();
			this->cmdWResults->Add(Text::StrCopyNew(cmd));
			this->cmdResultMut->Unlock();
			this->cmdEvt->Set();
		}
	}
	else
	{
		this->cmdResultMut->Lock();
		this->cmdWResults->Add(Text::StrCopyNew(cmd));
		this->cmdResultMut->Unlock();
		this->cmdEvt->Set();
	}
}

IO::Device::MTKGPSNMEA::MTKGPSNMEA(IO::Stream *stm, Bool relStm) : IO::GPSNMEA(stm, relStm)
{
	NEW_CLASS(this->cmdMut, Sync::Mutex());
	NEW_CLASS(this->cmdEvt, Sync::Event(true, (const UTF8Char*)"IO.Device.MTKTracker.cmdEvt"));
	NEW_CLASS(this->cmdResultMut, Sync::Mutex());
//	NEW_CLASS(this->cmdResults, Data::ArrayList<const Char *>());
	NEW_CLASS(this->cmdWResults, Data::ArrayList<const UTF8Char *>());
	this->firmwareBuild = 0;
	this->firmwareRel = 0;
	this->productMode = 0;
	this->sdkVer = 0;
}

IO::Device::MTKGPSNMEA::~MTKGPSNMEA()
{
	OSInt i;
	this->stm->Close();
	i = this->cmdWResults->GetCount();
	while (i-- > 0)
	{
		Text::StrDelNew(this->cmdWResults->RemoveAt(i));
	}
	DEL_CLASS(this->cmdWResults);
	DEL_CLASS(this->cmdResultMut);
	DEL_CLASS(this->cmdEvt);
	DEL_CLASS(this->cmdMut);
	SDEL_TEXT(this->firmwareBuild);
	SDEL_TEXT(this->firmwareRel);
	SDEL_TEXT(this->productMode);
	SDEL_TEXT(this->sdkVer);
}

Map::ILocationService::ServiceType IO::Device::MTKGPSNMEA::GetServiceType()
{
	return Map::ILocationService::ST_MTK;
}

void IO::Device::MTKGPSNMEA::HotStart()
{
	UInt8 buff[64];
	OSInt cmdSize = GenNMEACommand((const UTF8Char*)"$PMTK101", buff);
	this->stm->Write(buff, cmdSize);
}

void IO::Device::MTKGPSNMEA::WarmStart()
{
	UInt8 buff[64];
	OSInt cmdSize = GenNMEACommand((const UTF8Char*)"$PMTK102", buff);
	this->stm->Write(buff, cmdSize);
}

void IO::Device::MTKGPSNMEA::ColdStart()
{
	UInt8 buff[64];
	OSInt cmdSize = GenNMEACommand((const UTF8Char*)"$PMTK103", buff);
	this->stm->Write(buff, cmdSize);
}

void IO::Device::MTKGPSNMEA::FactoryReset()
{
	UInt8 buff[64];
	OSInt cmdSize = GenNMEACommand((const UTF8Char*)"$PMTK104", buff);
	this->stm->Write(buff, cmdSize);
}

Bool IO::Device::MTKGPSNMEA::IsMTKDevice()
{
	UInt8 buff[64];
	Bool succ = false;
	OSInt cmdSize = GenNMEACommand((const UTF8Char*)"$PMTK000", buff);
	const UTF8Char *result = SendMTKCommand(buff, cmdSize, (const UTF8Char*)"$PMTK001", 2000);
	if (result == 0)
		return false;
	succ = Text::StrStartsWith(result, (const UTF8Char*)"$PMTK001,0,3*");
	Text::StrDelNew(result);
	return succ;
}

Bool IO::Device::MTKGPSNMEA::QueryFirmware()
{
	UTF8Char sbuff[256];
	UTF8Char *sarr2[2];
	UTF8Char *sarr[5];
	OSInt i;
	UInt8 buff[64];
	OSInt cmdSize = GenNMEACommand((const UTF8Char*)"$PMTK605", buff);
	const UTF8Char *result = SendMTKCommand(buff, cmdSize, (const UTF8Char*)"$PMTK705", 2000);
	if (result == 0)
		return false;
	
	Text::StrConcat(sbuff, result);
	Text::StrDelNew(result);
	Text::StrSplit(sarr2, 2, sbuff, '*');
	i = Text::StrSplit(sarr, 5, sarr2[0], ',');
	if (i == 5)
	{
		SDEL_TEXT(this->firmwareBuild);
		SDEL_TEXT(this->firmwareRel);
		SDEL_TEXT(this->productMode);
		SDEL_TEXT(this->sdkVer);
		this->firmwareRel = Text::StrCopyNew(sarr[1]);
		this->firmwareBuild = Text::StrCopyNew(sarr[2]);
		this->productMode = Text::StrCopyNew(sarr[3]);
		this->sdkVer = Text::StrCopyNew(sarr[4]);
		return true;
	}
	else
	{
		return false;
	}
}

Bool IO::Device::MTKGPSNMEA::IsLogEnabled()
{
	UInt8 buff[64];
	OSInt cmdSize = GenNMEACommand((const UTF8Char*)"$PMTK182,2,7", buff);
	UTF8Char sbuff[128];
	const UTF8Char *result = SendMTKCommand(buff, cmdSize, (const UTF8Char*)"$PMTK182,3,7,", 2000);
	if (result == 0)
		return false;
	Text::StrConcat(sbuff, result);
	OSInt i = Text::StrIndexOf(sbuff, '*');
	sbuff[i] = 0;
	i = Text::StrLastIndexOf(sbuff, ',');
	i = Text::StrToInt32(&sbuff[i + 1]);
	Text::StrDelNew(result);
	return (i & 2) != 0;
}

Bool IO::Device::MTKGPSNMEA::DisableLog()
{
	UInt8 buff[64];
	OSInt cmdSize = GenNMEACommand((const UTF8Char*)"$PMTK182,5", buff);
	const UTF8Char *result = SendMTKCommand(buff, cmdSize, (const UTF8Char*)"$PMTK001,182,5,3", 2000);
	if (result == 0)
		return false;
	Text::StrDelNew(result);
	return true;
}

Bool IO::Device::MTKGPSNMEA::EnableLog()
{
	UInt8 buff[64];
	OSInt cmdSize = GenNMEACommand((const UTF8Char*)"$PMTK182,4", buff);
	const UTF8Char *result = SendMTKCommand(buff, cmdSize, (const UTF8Char*)"$PMTK001,182,4,3", 2000);
	if (result == 0)
		return false;
	Text::StrDelNew(result);
	return true;
}

OSInt IO::Device::MTKGPSNMEA::CalLogBlockCount(OSInt logSize)
{
	if (logSize & 65535)
	{
		return (logSize >> 16) + 1;
	}
	else
	{
		return (logSize >> 16);
	}
}

Bool IO::Device::MTKGPSNMEA::ReadLogPart(OSInt addr, UInt8 *buff)
{
	UTF8Char sbuff[64];
	UTF8Char *sptr;
	OSInt i;
	OSInt j;
	UInt8 cbuff[64];
	sptr = Text::StrConcat(sbuff, (const UTF8Char*)"$PMTK182,7,");
	sptr = Text::StrHexVal32(sptr, (Int32)addr);
	sptr = Text::StrConcat(sptr, (const UTF8Char*)",");
	sptr = Text::StrHexVal32(sptr, 0x400);
	Data::DateTime dt;
	Data::DateTime dt2;
	const UTF8Char *data = 0;
	const UTF8Char *resp = 0;
	const UTF8Char *cmdRes;
	i = GenNMEACommand(sbuff, cbuff);
	this->cmdMut->Lock();
	this->stm->Write(cbuff, i);
	
	dt.SetCurrTimeUTC();
	while (true)
	{
		this->cmdEvt->Wait(1000);
		while (this->cmdWResults->GetCount() > 0)
		{
			dt.SetCurrTimeUTC();
			this->cmdResultMut->Lock();
			cmdRes = this->cmdWResults->RemoveAt(0);
			this->cmdResultMut->Unlock();
			if (Text::StrStartsWith(cmdRes, (const UTF8Char*)"$PMTK182,8"))
			{
				SDEL_TEXT(data);
				data = cmdRes;
			}
			else if (Text::StrStartsWith(cmdRes, (const UTF8Char*)"$PMTK001,182,7,"))
			{
				resp = cmdRes;
				break;
			}
			else
			{
				Text::StrDelNew(cmdRes);
			}
		}
		if (resp)
			break;
		dt2.SetCurrTimeUTC();
		if (dt2.DiffMS(&dt) >= 2000)
			break;
	}

	this->cmdMut->Unlock();
	if (resp && data)
	{
		Bool succ = false;
		Text::StrConcatC(sbuff, &data[11], 8);
		i = Text::StrHex2Int32(sbuff);
		j = Text::StrCharCnt(data);
		if (i == addr && j == 2071)
		{
			j = Text::StrHex2Bytes(&data[20], buff);
			if (j == 1024)
			{
				succ = true;
			}
		}
		Text::StrDelNew(data);
		Text::StrDelNew(resp);
		return succ;
	}
	else
	{
		if (data)
		{
			Text::StrDelNew(data);
		}
		if (resp)
		{
			Text::StrDelNew(resp);
		}
		return false;
	}
}

Bool IO::Device::MTKGPSNMEA::ReadLogBlock(OSInt addr, UInt8 *buff)
{
	OSInt retryCnt = 0;
	OSInt ofst = 0;
	Bool succ;
	while (ofst < 65536)
	{
		succ = this->ReadLogPart(addr + ofst, &buff[ofst]);
		if (succ)
		{
			ofst += 1024;
			retryCnt = 0;
		}
		else
		{
			if (++retryCnt >= 3)
				return false;
		}
	}
	return true;
}

Bool IO::Device::MTKGPSNMEA::ParseLog(Map::GPSTrack *gps)
{
	Bool logEnabled = this->IsLogEnabled();
	this->DisableLog();
	OSInt logSize = this->GetLogSize();
	OSInt endAddr = this->CalLogBlockCount(logSize) << 16;
	OSInt addr = 0;
	Bool succ = true;
	UInt8 *block;
	block = MemAlloc(UInt8, 65536);
	while (addr < endAddr)
	{
		succ = this->ReadLogBlock(addr, block);
		if (!succ)
			break;
		succ = ParseBlock(block, gps);
		if (!succ)
			break;
		addr += 65536;
	}

	MemFree(block);
	if (logEnabled)
	{
		this->EnableLog();
	}
	return succ;
}

Bool IO::Device::MTKGPSNMEA::DelLogData()
{
	UInt8 buff[64];
	OSInt cmdSize = GenNMEACommand((const UTF8Char*)"$PMTK182,6,1", buff);
	const UTF8Char *result = SendMTKCommand(buff, cmdSize, (const UTF8Char*)"$PMTK001,182,6,3", 30000);
	if (result == 0)
		return false;
	Text::StrDelNew(result);
	return true;
}

Bool IO::Device::MTKGPSNMEA::SetLogFormat(LogFormat lf)
{
	UTF8Char sbuff[32];
	UInt8 buff[64];
	Text::StrHexVal32(Text::StrConcat(sbuff, (const UTF8Char*)"$PMTK182,1,2,"), (Int32)lf);
	OSInt cmdSize = GenNMEACommand(sbuff, buff);
	const UTF8Char *result = SendMTKCommand(buff, cmdSize, (const UTF8Char*)"$PMTK001,182,1,3", 2000);
	if (result == 0)
		return false;
	Text::StrDelNew(result);
	return true;
}

Bool IO::Device::MTKGPSNMEA::SetLogInterval(Int32 sec)
{
	UTF8Char sbuff[32];
	UInt8 buff[64];
	Text::StrInt32(Text::StrConcat(sbuff, (const UTF8Char*)"$PMTK182,1,3,"), sec * 10);
	OSInt cmdSize = GenNMEACommand(sbuff, buff);
	const UTF8Char *result = SendMTKCommand(buff, cmdSize, (const UTF8Char*)"$PMTK001,182,1,3", 2000);
	if (result == 0)
		return false;
	Text::StrDelNew(result);
	return true;
}

Bool IO::Device::MTKGPSNMEA::SetLogDistance(Int32 meter)
{
	UTF8Char sbuff[32];
	UInt8 buff[64];
	Text::StrInt32(Text::StrConcat(sbuff, (const UTF8Char*)"$PMTK182,1,4,"), meter * 10);
	OSInt cmdSize = GenNMEACommand(sbuff, buff);
	const UTF8Char *result = SendMTKCommand(buff, cmdSize, (const UTF8Char*)"$PMTK001,182,1,3", 2000);
	if (result == 0)
		return false;
	Text::StrDelNew(result);
	return true;
}

Bool IO::Device::MTKGPSNMEA::SetLogMode(LogMode lm)
{
	const UTF8Char *cmd;
	UInt8 buff[64];
	if (lm == LM_OVERWRITE)
	{
		cmd = (const UTF8Char*)"$PMTK182,1,6,1";
	}
	else if (lm == LM_STOP)
	{
		cmd = (const UTF8Char*)"$PMTK182,1,6,2";
	}
	else
	{
		return false;
	}
	OSInt cmdSize = GenNMEACommand(cmd, buff);
	const UTF8Char *result = SendMTKCommand(buff, cmdSize, (const UTF8Char*)"$PMTK001,182,1,3", 2000);
	if (result == 0)
		return false;
	Text::StrDelNew(result);
	return true;
}

IO::Device::MTKGPSNMEA::LogFormat IO::Device::MTKGPSNMEA::GetLogFormat()
{
	UTF8Char sbuff[128];
	UInt8 buff[64];
	OSInt cmdSize = GenNMEACommand((const UTF8Char*)"$PMTK182,2,2", buff);
	const UTF8Char *result = SendMTKCommand(buff, cmdSize, (const UTF8Char*)"$PMTK182,3,2", 2000);
	if (result == 0)
		return LF_UNKNOWN;
	Text::StrConcat(sbuff, result);
	OSInt i = Text::StrIndexOf(sbuff, '*');
	sbuff[i] = 0;
	i = Text::StrLastIndexOf(sbuff, ',');
	i = Text::StrHex2Int32(&sbuff[i + 1]);
	Text::StrDelNew(result);
	return (LogFormat)i;
}

Int32 IO::Device::MTKGPSNMEA::GetLogInterval()
{
	UTF8Char sbuff[128];
	UInt8 buff[64];
	OSInt cmdSize = GenNMEACommand((const UTF8Char*)"$PMTK182,2,3", buff);
	const UTF8Char *result = SendMTKCommand(buff, cmdSize, (const UTF8Char*)"$PMTK182,3,3", 2000);
	if (result == 0)
		return 0;
	Text::StrConcat(sbuff, result);
	OSInt i = Text::StrIndexOf(sbuff, '*');
	sbuff[i] = 0;
	i = Text::StrLastIndexOf(sbuff, ',');
	i = Text::StrToInt32(&sbuff[i + 1]);
	Text::StrDelNew(result);
	return (Int32)i / 10;
}

Int32 IO::Device::MTKGPSNMEA::GetLogDistance()
{
	UTF8Char sbuff[128];
	UInt8 buff[64];
	OSInt cmdSize = GenNMEACommand((const UTF8Char*)"$PMTK182,2,4", buff);
	const UTF8Char *result = SendMTKCommand(buff, cmdSize, (const UTF8Char*)"$PMTK182,3,4", 2000);
	if (result == 0)
		return 0;
	Text::StrConcat(sbuff, result);
	OSInt i = Text::StrIndexOf(sbuff, '*');
	sbuff[i] = 0;
	i = Text::StrLastIndexOf(sbuff, ',');
	i = Text::StrToInt32(&sbuff[i + 1]);
	Text::StrDelNew(result);
	return (Int32)i / 10;
}

IO::Device::MTKGPSNMEA::LogMode IO::Device::MTKGPSNMEA::GetLogMode()
{
	UTF8Char sbuff[128];
	LogMode lm = LM_UNKNOWN;
	UInt8 buff[64];
	OSInt cmdSize = GenNMEACommand((const UTF8Char*)"$PMTK182,2,6", buff);
	const UTF8Char *result = SendMTKCommand(buff, cmdSize, (const UTF8Char*)"$PMTK182,3,6", 2000);
	if (result == 0)
		return LM_UNKNOWN;
	Text::StrConcat(sbuff, result);
	OSInt i = Text::StrIndexOf(sbuff, '*');
	sbuff[i] = 0;
	i = Text::StrLastIndexOf(sbuff, ',');
	i = Text::StrToInt32(&sbuff[i + 1]);
	Text::StrDelNew(result);
	if (i == 1)
	{
		lm = LM_OVERWRITE;
	}
	else if (i == 2)
	{
		lm = LM_STOP;
	}
	return lm;
}

OSInt IO::Device::MTKGPSNMEA::GetLogSize()
{
	UTF8Char sbuff[128];
	UInt8 buff[64];
	OSInt cmdSize = GenNMEACommand((const UTF8Char*)"$PMTK182,2,8", buff);
	const UTF8Char *result = SendMTKCommand(buff, cmdSize, (const UTF8Char*)"$PMTK182,3,8", 2000);
	if (result == 0)
		return 0;
	Text::StrConcat(sbuff, result);
	OSInt i = Text::StrIndexOf(sbuff, '*');
	sbuff[i] = 0;
	i = Text::StrLastIndexOf(sbuff, ',');
	i = Text::StrHex2Int32(&sbuff[i + 1]);
	Text::StrDelNew(result);
	return i;
}

const UTF8Char *IO::Device::MTKGPSNMEA::SendMTKCommand(const UInt8 *cmdBuff, OSInt cmdSize, const UTF8Char *resultStart, Int32 timeoutMS)
{
	Data::DateTime dt;
	Data::DateTime dt2;
	const UTF8Char *cmdRes;

	this->cmdMut->Lock();
	this->stm->Write(cmdBuff, cmdSize);
	const UTF8Char *resultStr = 0;
	
	dt.SetCurrTimeUTC();
	while (true)
	{
		this->cmdEvt->Wait(1000);
		while (this->cmdWResults->GetCount() > 0)
		{
			dt.SetCurrTimeUTC();
			this->cmdResultMut->Lock();
			cmdRes = this->cmdWResults->RemoveAt(0);
			this->cmdResultMut->Unlock();
			if (Text::StrStartsWith(cmdRes, resultStart))
			{
				resultStr = cmdRes;
				break;
			}
			else
			{
				Text::StrDelNew(cmdRes);
			}
		}
		if (resultStr)
			break;
		dt2.SetCurrTimeUTC();
		if (dt2.DiffMS(&dt) >= timeoutMS)
			break;
	}

	this->cmdMut->Unlock();
	return resultStr;
}

const UTF8Char *IO::Device::MTKGPSNMEA::GetFirmwareRel()
{
	return this->firmwareRel;
}

const UTF8Char *IO::Device::MTKGPSNMEA::GetFirmwareBuild()
{
	return this->firmwareBuild;
}

const UTF8Char *IO::Device::MTKGPSNMEA::GetProductMode()
{
	return this->productMode;
}

const UTF8Char *IO::Device::MTKGPSNMEA::GetSDKVer()
{
	return this->sdkVer;
}

UOSInt IO::Device::MTKGPSNMEA::GetMTKSerialPort()
{
	UOSInt port = IO::SerialPort::GetUSBPort();
	if (port)
		return port;
	port = IO::SerialPort::GetBTPort();
	return port;
}

Bool IO::Device::MTKGPSNMEA::ParseBlock(UInt8 *block, Map::GPSTrack *gps)
{
	Int32 bitmask;
	Map::GPSTrack::GPSRecord rec;
	bitmask = *(Int32*)&block[2];
	if ((bitmask & 0x3f) != 0x3f)
		return false;

	Data::DateTime currTime;
	currTime.SetCurrTimeUTC();
	currTime.SetYear(currTime.GetYear() - 5);
	Int64 minTime = currTime.ToTicks();
	currTime.SetCurrTimeUTC();
	currTime.SetYear(currTime.GetYear() + 1);
	Int64 maxTime = currTime.ToTicks();

	OSInt currOfst = 0x200;
	OSInt recStart;
	while (currOfst <= 65532)
	{
		if (*(UInt32*)&block[currOfst] == 0xaaaaaaaa && currOfst <= 65520 && (*(Int32*)&block[currOfst + 4] & 0xffffff) == 0xaaaaaa && *(UInt32*)&block[currOfst + 12] == 0xbbbbbbbb)
		{
			if (block[currOfst + 7] == 2)
			{
				bitmask = *(Int32*)&block[currOfst + 8];
				if ((bitmask & 0x3f) != 0x3f)
					return false;
			}
			currOfst += 16;
		}
		else if (*(UInt32*)&block[currOfst] == 0xffffffff)
		{
			return true;
		}
		else
		{
			recStart = currOfst;
//			if (bitmask & 1) // UTC
//			{
				rec.utcTimeTicks = 1000LL * *(UInt32*)&block[currOfst];
				currOfst += 4;
				while (rec.utcTimeTicks < minTime)
				{
					rec.utcTimeTicks += 619315200000; //7168 days
				}
				while (rec.utcTimeTicks > maxTime)
				{
					rec.utcTimeTicks -= 619315200000; //7168 days
				}
//			}
//			if (bitmask & 2) // VALID
//			{
				rec.valid = (block[currOfst] & 1) != 1;
				currOfst += 2;
//			}
//			if (bitmask & 4) // LATITUDE
//			{
				rec.lat = *(Double*)&block[currOfst];
				currOfst += 8;
//			}
//			if (bitmask & 8) // LONGITUDE
//			{
				rec.lon = *(Double*)&block[currOfst];
				currOfst += 8;
//			}
			if (bitmask & 0x10) // HEIGHT
			{
				rec.altitude = *(Single*)&block[currOfst];
				currOfst += 4;
			}
			else
			{
				rec.altitude = 0;
			}
			if (bitmask & 0x20) // SPEED
			{
				rec.speed = (*(Single*)&block[currOfst]) / 1.852;
				currOfst += 4;
			}
			else
			{
				rec.speed = 0;
			}
			if (bitmask & 0x40) // HEADING
			{
				rec.heading = *(Single*)&block[currOfst];
				currOfst += 4;
			}
			else
			{
				rec.heading = 0;
			}
			if (bitmask & 0x80) // DSTA
			{
				currOfst += 2;
			}
			if (bitmask & 0x100) // DAGE
			{
				currOfst += 4;
			}
			if (bitmask & 0x200) // PDOP
			{
//				Double pdop = *(UInt16*)&block[currOfst] * 0.01;
				currOfst += 2;
			}
			if (bitmask & 0x400) // HDOP
			{
//				Double hdop = *(UInt16*)&block[currOfst] * 0.01;
				currOfst += 2;
			}
			if (bitmask & 0x800) // VDOP
			{
//				Double vdop = *(UInt16*)&block[currOfst] * 0.01;
				currOfst += 2;
			}
			if (bitmask & 0x1000) // NSAT
			{
				rec.nSateUsed = block[currOfst];
				rec.nSateView = block[currOfst + 1];
				currOfst += 2;
			}
			else
			{
				rec.nSateUsed = 0;
				rec.nSateView = 0;
			}
			if (bitmask & 0x2000) //SID-ELEVATION-AZIMUTH-SNR
			{
				Int32 size = 0;
				if (bitmask & 0x2000) //SID
				{
					size += 4;
				}
				if (bitmask & 0x4000) //ELEVATION
				{
					size += 2;
				}
				if (bitmask & 0x8000) //AZIMUTH
				{
					size += 2;
				}
				if (bitmask & 0x10000)
				{
					size += 2;
				}
				currOfst += rec.nSateUsed * size;
			}
			if (bitmask & 0x20000) // RCR
			{
				currOfst += 2;
			}
			if (bitmask & 0x40000) // MILLISECOND
			{
				currOfst += 2;
			}
			if (bitmask & 0x80000) // DISTANCE
			{
				currOfst += 8;
			}
			if (block[currOfst] != '*')
				return false;
			UInt8 chk = 0;
			while (recStart < currOfst)
			{
				chk = chk ^ block[recStart];
				recStart++;
			}
			if (block[currOfst + 1] == chk)
			{
				gps->AddRecord(&rec);
			}
			currOfst += 2;
		}
	}
	return true;
}