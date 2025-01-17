#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ByteTool.h"
#include "Net/IRTPPLHandler.h"
#include "Net/RTPSvrChannel.h"
#include "Sync/Thread.h"
#include "Text/MyString.h"
#include "Text/URLString.h"

#include "IO/Console.h"
#include "Text/StringBuilder.h"

void __stdcall Net::RTPSvrChannel::PacketHdlr(UInt32 ip, UInt16 port, UInt8 *buff, OSInt dataSize, void *userData)
{
	Net::RTPSvrChannel *me = (Net::RTPSvrChannel*)userData;

	if (dataSize < 12)
	{
		me = 0;
		return;
	}
	Int32 v = buff[0] >> 6;
	if (v != 2)
		return;
	if (buff[0] & 0x20) //padding
	{
		dataSize -= buff[dataSize - 1];
		if (dataSize < 12)
			return;
	}
	Bool extension = (buff[0] & 0x10) != 0;
	Int32 csrcCnt = buff[0] & 15;
	Bool marker = (buff[1] & 0x80) != 0;
	Int32 payloadType = buff[1] & 0x7f;
	Int32 seqNum = ReadMUInt16(&buff[2]);
	UInt32 timestamp = ReadMUInt32(&buff[4]);
	me->lastSSRC = ReadMUInt32(&buff[8]);

/*	me->packMut->Lock();
	if (me->packCnt >= me->threadCnt)
	{
		Bool lastExist = false;
		OSInt i;
		OSInt minIndex = 0;
		Int32 minSeq = me->packBuff[0].seqNum;
		if (minSeq == 65535)
			lastExist = true;
		i = me->threadCnt;
		while (i-- > 0)
		{
			if (me->packBuff[i].seqNum == 65535)
				lastExist = true;
			if (me->packBuff[i].seqNum < minSeq)
			{
				minSeq = me->packBuff[i].seqNum;
				minIndex = i;
			}
		}
		if (lastExist)
		{
			minSeq = 65536;
			i = me->threadCnt;
			while (i-- > 0)
			{
				if (me->packBuff[i].seqNum > 32767 && me->packBuff[i].seqNum < minSeq)
				{
					minSeq = me->packBuff[i].seqNum;
					minIndex = i;
				}
			}
		}


		Net::IRTPPLHandler *plHdlr = me->payloadMap->Get(me->packBuff[minIndex].payloadType);
		if (plHdlr)
		{
			plHdlr->MediaDataReceived(me->packBuff[minIndex].buff, me->packBuff[minIndex].dataSize, me->packBuff[minIndex].seqNum, me->packBuff[minIndex].ts);
		}
		me->packBuff[minIndex].payloadType = payloadType;
		me->packBuff[minIndex].seqNum = seqNum;
		me->packBuff[minIndex].dataSize  = dataSize - 12;
		me->packBuff[minIndex].ts = timestamp;
		MemCopy(me->packBuff[minIndex].buff, &buff[12], dataSize - 12);
	}
	else
	{
		me->packBuff[me->packCnt].payloadType = payloadType;
		me->packBuff[me->packCnt].seqNum = seqNum;
		me->packBuff[me->packCnt].dataSize  = dataSize - 12;
		me->packBuff[me->packCnt].ts = timestamp;
		MemCopy(me->packBuff[me->packCnt].buff, &buff[12], dataSize - 12);
		me->packCnt++;
	}

	me->packMut->Unlock();*/
}

void __stdcall Net::RTPSvrChannel::PacketCtrlHdlr(UInt32 ip, UInt16 port, UInt8 *buff, OSInt dataSize, void *userData)
{
	Net::RTPSvrChannel *me = (Net::RTPSvrChannel*)userData;
	Data::DateTime *dt;
	Text::StringBuilder *sb;
	Int32 size = 0;
	Int32 ofst = 0;
	OSInt i;
	while (ofst < dataSize)
	{
		size = (ReadMInt16(&buff[ofst + 2]) + 1) << 2;
		if (size + ofst > dataSize)
		{
			break;
		}
		else if ((buff[ofst + 0] & 0xc0) == 0x80)
		{
			switch (buff[ofst + 1])
			{
			case 200: //SR - Sender Report
/*				NEW_CLASS(sb, Text::StringBuilder());
				sb->Append(L"SR: Len=");
				sb->Append(size);
				sb->Append(L",RC=");
				sb->Append(buff[ofst + 0] & 0x1f);
				sb->Append(L",SSRC=");
				sb->Append(ReadMInt32(&buff[ofst + 4]));
				if (size >= 28)
				{
					NEW_CLASS(dt, Data::DateTime());
					sb->Append(L",NTP ts=");
					dt->SetNTPTime(ReadMInt32(&buff[ofst + 8]), ReadMInt32(&buff[ofst + 12]));
					sb->Append(dt);
					sb->Append(L",RTP ts=");
					sb->Append(ReadMInt32(&buff[ofst + 16]));
					sb->Append(L",nPacket=");
					sb->Append(ReadMInt32(&buff[ofst + 20]));
					sb->Append(L",nOctet=");
					sb->Append(ReadMInt32(&buff[ofst + 24]));
					DEL_CLASS(dt);
				}
				sb->Append(L"\r\n");
				IO::Console::PrintStrO(sb->ToString());
				DEL_CLASS(sb);*/

				break;
			case 201: //RR - Receiver Report
/*				NEW_CLASS(sb, Text::StringBuilder());
				sb->Append(L"RR: Len=");
				sb->Append(size);
				sb->Append(L",RC=");
				sb->Append(buff[ofst + 0] & 0x1f);
				sb->Append(L",SSRC=");
				sb->Append(ReadMInt32(&buff[ofst + 4]));
				sb->Append(L"\r\n");
				IO::Console::PrintStrO(sb->ToString());
				DEL_CLASS(sb);*/
				me->sessCtrl->SessionKA(me->ssrc);
				break;
			case 202: //SDES - Source Description RTCP Packet
//				IO::Console::PrintStrO(L"SDES\r\n");
				break;
			case 203: //BYE
				break;
			default:
				i = 1;
				break;
			}
		}
		ofst += size;
	}
}

Net::RTPSvrChannel::RTPSvrChannel(Net::SocketFactory *sockf, UInt16 port, Int32 ssrc, Int32 targetIP, UInt16 targetPort, Net::RTPSessionController *sessCtrl)
{
	this->rtpUDP = 0;
	this->rtcpUDP = 0;
	this->threadCnt = 5;
	this->ssrc = ssrc;
	this->targetIP = targetIP;
	this->targetPort = targetPort;
	this->sessCtrl = sessCtrl;
	this->seqNum = 12442;

	if (port & 1)
	{
		port += 1;
	}
	NEW_CLASS(this->rtpUDP, Net::UDPServer(sockf, port, 0, PacketHdlr, this, 0, 0, this->threadCnt, false));
	if (port == 0)
	{
		port = this->rtpUDP->GetPort();
		if (port & 1)
		{
			port += 1;
			DEL_CLASS(this->rtpUDP);
			NEW_CLASS(this->rtpUDP, Net::UDPServer(sockf, port, 0, PacketHdlr, this, 0, 0, this->threadCnt, false));
		}
	}
	NEW_CLASS(this->rtcpUDP, Net::UDPServer(sockf, port + 1, 0, PacketCtrlHdlr, this, 0, 0, 1, false));
}

Net::RTPSvrChannel::~RTPSvrChannel()
{

	DEL_CLASS(this->rtpUDP);
	SDEL_CLASS(this->rtcpUDP);
}

Int32 Net::RTPSvrChannel::GetTargetIP()
{
	return this->targetIP;
}

UInt16 Net::RTPSvrChannel::GetPort()
{
	return this->rtpUDP->GetPort();
}

Int32 Net::RTPSvrChannel::GetSeqNum()
{
	return seqNum;
}

Bool Net::RTPSvrChannel::SendPacket(Int32 payloadType, Int32 ts, UInt8 *buff, UInt32 dataSize, Bool marker)
{
	OSInt sendSize = dataSize + 12;
	UInt8 sendBuff[1500];
	sendBuff[0] = 0x80;
	if (dataSize & 1)
	{
		sendBuff[0] |= 0x20;
		sendBuff[sendSize] = 0;
		sendSize++;
	}
	sendBuff[1] = payloadType & 0x7f;
	if (marker)
		sendBuff[1] |= 0x80;
	WriteMInt16(&sendBuff[2], this->seqNum);
	this->seqNum = (seqNum + 1) & 65535;
	WriteMInt32(&sendBuff[4], ts);
	WriteMInt32(&sendBuff[8], this->ssrc);
	MemCopy(&sendBuff[12], buff, dataSize);
	this->rtpUDP->SendTo(this->targetIP, this->targetPort, sendBuff, sendSize);
	return true;
}

Bool Net::RTPSvrChannel::SendControl(UInt8 *buff, UInt32 dataSize)
{
	this->rtcpUDP->SendTo(this->targetIP, this->targetPort + 1, buff, dataSize);
	return true;
}
