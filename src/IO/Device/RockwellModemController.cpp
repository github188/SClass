#include "Stdafx.h"
#include "MyMemory.h"
#include "IO/Device/RockwellModemController.h"
#include "Text/MyString.h"

IO::Device::RockwellModemController::RockwellModemController(IO::ATCommandChannel *channel, Bool needRelease) : IO::ModemController(channel, needRelease)
{
}

IO::Device::RockwellModemController::~RockwellModemController()
{
}

UTF8Char *IO::Device::RockwellModemController::VoiceGetManufacturer(UTF8Char *manu)
{
	return this->SendStringCommand(manu, "AT#MFR?", 3000);
}

UTF8Char *IO::Device::RockwellModemController::VoiceGetModel(UTF8Char *model)
{
	return this->SendStringCommand(model, "AT#MDL?", 3000);
}

UTF8Char *IO::Device::RockwellModemController::VoiceGetRevision(UTF8Char *ver)
{
	return this->SendStringCommand(ver, "AT#REV?", 3000);
}

Bool IO::Device::RockwellModemController::VoiceGetBaudRate(Int32 *baudRate)
{
	UTF8Char sbuff[128];
	if (this->SendStringCommand(sbuff, "AT#BDR?", 3000) == 0)
		return false;
	Int32 v;
	if (!Text::StrToInt32(sbuff, &v))
		return false;
	*baudRate = v * 2400;
	return true;
}

Bool IO::Device::RockwellModemController::VoiceSetBaudRate(Int32 baudRate)
{
	Char sbuff[32];
	Text::StrInt32(Text::StrConcat(sbuff, "AT#BDR="), baudRate / 2400);
	return this->SendBoolCommand(sbuff);
}

Bool IO::Device::RockwellModemController::VoiceGetCallerIDType(CallerIDType *callerIDType)
{
	UTF8Char sbuff[128];
	if (this->SendStringCommand(sbuff, "AT#CID?", 3000) == 0)
		return false;
	Int32 v;
	if (!Text::StrToInt32(sbuff, &v))
		return false;
	*callerIDType = (CallerIDType)v;
	return true;
}

Bool IO::Device::RockwellModemController::VoiceSetCallerIDType(CallerIDType callerIDType)
{
	switch (callerIDType)
	{
	case CIDT_DISABLE:
		return this->SendBoolCommand("AT#CID=0");
	case CIDT_FORMATED:
		return this->SendBoolCommand("AT#CID=1");
	case CIDT_UNFORMATED:
		return this->SendBoolCommand("AT#CID=2");
	default:
		return false;
	}
}

Bool IO::Device::RockwellModemController::VoiceGetType(VoiceType *voiceType)
{
	UTF8Char sbuff[128];
	if (this->SendStringCommand(sbuff, "AT#CLS?", 3000) == 0)
		return false;
	Int32 v;
	if (!Text::StrToInt32(sbuff, &v))
		return false;
	*voiceType = (VoiceType)v;
	return true;
}

Bool IO::Device::RockwellModemController::VoiceSetType(VoiceType voiceType)
{
	switch (voiceType)
	{
	case VT_DATA:
		return this->SendBoolCommand("AT#CLS=0");
	case VT_CLASS1FAX:
		return this->SendBoolCommand("AT#CLS=1");
	case VT_CLASS2FAX:
		return this->SendBoolCommand("AT#CLS=2");
	case VT_VOICE:
		return this->SendBoolCommand("AT#CLS=8");
	default:
		return false;
	}
}

Bool IO::Device::RockwellModemController::VoiceGetBufferSize(Int32 *buffSize)
{
	UTF8Char sbuff[128];
	if (this->SendStringCommand(sbuff, "AT#VBQ?", 3000) == 0)
		return false;
	return Text::StrToInt32(sbuff, buffSize);
}

Bool IO::Device::RockwellModemController::VoiceGetBitsPerSample(Int32 *bps)
{
	UTF8Char sbuff[128];
	if (this->SendStringCommand(sbuff, "AT#VBS?", 3000) == 0)
		return false;
	return Text::StrToInt32(sbuff, bps);
}

Bool IO::Device::RockwellModemController::VoiceSetBitsPerSample(Int32 bps)
{
	Char sbuff[32];
	Text::StrInt32(Text::StrConcat(sbuff, "AT#VBS="), bps);
	return this->SendBoolCommand(sbuff);
}

Bool IO::Device::RockwellModemController::VoiceGetToneDur(Int32 *durMS)
{
	UTF8Char sbuff[128];
	if (this->SendStringCommand(sbuff, "AT#VBS?", 3000) == 0)
		return false;
	Int32 v;
	if (!Text::StrToInt32(sbuff, &v))
		return false;
	*durMS = v * 100;
	return true;
}

Bool IO::Device::RockwellModemController::VoiceSetToneDur(Int32 durMS)
{
	Char sbuff[32];
	Text::StrInt32(Text::StrConcat(sbuff, "AT#VBT="), durMS / 100);
	return this->SendBoolCommand(sbuff);
}

UTF8Char *IO::Device::RockwellModemController::VoiceGetCompression(UTF8Char *comp)
{
	return this->SendStringCommand(comp, "AT#VCI?", 3000);
}

Bool IO::Device::RockwellModemController::VoiceGetVoiceLineType(VoiceLineType *voiceLineType)
{
	UTF8Char sbuff[128];
	if (this->SendStringCommand(sbuff, "AT#VLS?", 3000) == 0)
		return false;
	Int32 v;
	if (!Text::StrToInt32(sbuff, &v))
		return false;
	*voiceLineType = (VoiceLineType)v;
	return true;
}

Bool IO::Device::RockwellModemController::VoiceSetVoiceLineType(VoiceLineType voiceLineType)
{
	Char sbuff[32];
	Text::StrInt32(Text::StrConcat(sbuff, "AT#VLS="), (Int32)voiceLineType);
	return this->SendBoolCommand(sbuff);
}

IO::ModemController::DialResult IO::Device::RockwellModemController::VoiceToneDial(Char *phoneNum)
{
	Char sbuff[32];
	Text::StrConcat(Text::StrConcat(sbuff, "ATDT"), phoneNum);
	return this->SendDialCommand(sbuff);
}

IO::ModemController::DialResult IO::Device::RockwellModemController::VoicePulseDial(Char *phoneNum)
{
	Char sbuff[32];
	Text::StrConcat(Text::StrConcat(sbuff, "ATDP"), phoneNum);
	return this->SendDialCommand(sbuff);
}

UTF8Char *IO::Device::RockwellModemController::GetVoiceTypeString(UTF8Char *buff, VoiceType voiceType)
{
	switch (voiceType)
	{
	case VT_DATA:
		return Text::StrConcat(buff, (const UTF8Char*)"Data Mode");
	case VT_CLASS1FAX:
		return Text::StrConcat(buff, (const UTF8Char*)"Class 1 fax");
	case VT_CLASS2FAX:
		return Text::StrConcat(buff, (const UTF8Char*)"Class 2 fax");
	case VT_VOICE:
		return Text::StrConcat(buff, (const UTF8Char*)"Voice/Audio Mode");
	default:
		return Text::StrConcat(buff, (const UTF8Char*)"Unknown");
	}
}

UTF8Char *IO::Device::RockwellModemController::GetCallerIDTypeString(UTF8Char *buff, CallerIDType callerIDType)
{
	switch (callerIDType)
	{
	case CIDT_DISABLE:
		return Text::StrConcat(buff, (const UTF8Char*)"Disable Caller ID");
	case CIDT_FORMATED:
		return Text::StrConcat(buff, (const UTF8Char*)"Enable Caller ID with formatted data");
	case CIDT_UNFORMATED:
		return Text::StrConcat(buff, (const UTF8Char*)"Enable Caller ID with unformatted data");
	default:
		return Text::StrConcat(buff, (const UTF8Char*)"Unknown");
	}
}

UTF8Char *IO::Device::RockwellModemController::GetVoiceLineTypeString(UTF8Char *buff, VoiceLineType voiceLineType)
{
	switch (voiceLineType)
	{
	case VLT_PHONE_LINE:
		return Text::StrConcat(buff, (const UTF8Char*)"Phone Line");
	case VLT_HANDSET:
		return Text::StrConcat(buff, (const UTF8Char*)"Handset");
	case VLT_SPEAKER:
		return Text::StrConcat(buff, (const UTF8Char*)"Speaker");
	case VLT_MICROPHONE:
		return Text::StrConcat(buff, (const UTF8Char*)"Auxiliary Input device (Microphone)");
	case VLT_INT_SPK:
		return Text::StrConcat(buff, (const UTF8Char*)"Phone Line with internal speaker");
	case VLT_EMULATION:
		return Text::StrConcat(buff, (const UTF8Char*)"Telephone Emulation Mode");
	case VLT_SPEAKERPHONE:
		return Text::StrConcat(buff, (const UTF8Char*)"Speakerphone Mode");
	case VLT_MUTE_HANDSET:
		return Text::StrConcat(buff, (const UTF8Char*)"Mutes local handset");
	case VLT_CALLER_ID_RELAY:
		return Text::StrConcat(buff, (const UTF8Char*)"Caller ID relay");
	case VLT_CODEC:
		return Text::StrConcat(buff, (const UTF8Char*)"Sound Codec");
	default:
		return Text::StrConcat(buff, (const UTF8Char*)"Unknown");
	}
}
