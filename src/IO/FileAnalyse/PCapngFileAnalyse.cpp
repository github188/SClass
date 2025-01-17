#include "Stdafx.h"
#include "Data/ByteTool.h"
#include "IO/FileAnalyse/PCapngFileAnalyse.h"
#include "Net/EthernetAnalyzer.h"
#include "Sync/Thread.h"

UInt32 __stdcall IO::FileAnalyse::PCapngFileAnalyse::ParseThread(void *userObj)
{
	IO::FileAnalyse::PCapngFileAnalyse *me = (IO::FileAnalyse::PCapngFileAnalyse*)userObj;
	UInt64 ofst;
	UInt64 dataSize;
	UInt32 thisSize;
	IO::FileAnalyse::PCapngFileAnalyse::BlockInfo *block;
	UInt8 packetHdr[16];
	Data::ArrayList<UInt16> *linkTypeList;
	Data::ArrayList<Int8> *resList;
	me->threadRunning = true;
	me->threadStarted = true;
	NEW_CLASS(resList, Data::ArrayList<Int8>());
	NEW_CLASS(linkTypeList, Data::ArrayList<UInt16>());
	ofst = 0;
	dataSize = me->fd->GetDataSize();
	while (ofst < dataSize - 16 && !me->threadToStop)
	{
		if (me->fd->GetRealData(ofst, 12, packetHdr) != 12)
			break;
		
		if (me->isBE)
		{
			thisSize = ReadMUInt32(&packetHdr[4]);
		}
		else
		{
			thisSize = ReadUInt32(&packetHdr[4]);
		}
		if (thisSize < 12 || thisSize + ofst > dataSize)
		{
			break;
		}
		block = MemAlloc(IO::FileAnalyse::PCapngFileAnalyse::BlockInfo, 1);
		block->ofst = ofst;
		if (me->isBE)
		{
			block->blockType = ReadMUInt32(&packetHdr[0]);
		}
		else
		{
			block->blockType = ReadUInt32(&packetHdr[0]);
		}
		block->blockLength = thisSize;
		block->timeResol = 0;
		if (block->blockType == 0x0a0d0d0a)
		{
			resList->Clear();
			linkTypeList->Clear();
		}
		else if (block->blockType == 1)
		{
			UInt8 *packetBuff = MemAlloc(UInt8, thisSize);
			me->fd->GetRealData(ofst, thisSize, packetBuff);
			UInt16 linkType;
			Int8 timeResol = 0;
			UInt16 optCode;
			UInt16 optLeng;
			if (me->isBE)
			{
				linkType = ReadMUInt16(&packetBuff[8]);
			}
			else
			{
				linkType = ReadUInt16(&packetBuff[8]);
			}
			
			UOSInt i = 16;
			while (i < block->blockLength - 4)
			{
				if (me->isBE)
				{
					optCode = ReadMUInt16(&packetBuff[i]);
					optLeng = ReadMUInt16(&packetBuff[i + 2]);
				}
				else
				{
					optCode = ReadUInt16(&packetBuff[i]);
					optLeng = ReadUInt16(&packetBuff[i + 2]);
				}
				if (i + 4 + optLeng > block->blockLength)
				{
					break;
				}
				if (optCode == 0)
				{
					break;
				}
				else if (optCode == 9)
				{
					timeResol = (Int8)packetBuff[i + 4];
					break;
				}
				i += 4 + (UOSInt)optLeng;
				if (i & 3)
				{
					i += 4 - (i & 3);
				}
			}
			resList->Add(timeResol);
			linkTypeList->Add(linkType);
			MemFree(packetBuff);
		}
		else if (block->blockType == 6 || block->blockType == 5)
		{
			UInt32 ifId;
			if (me->isBE)
			{
				ifId = ReadMUInt32(&packetHdr[8]);
			}
			else
			{
				ifId = ReadUInt32(&packetHdr[8]);
			}
			block->timeResol = resList->GetItem(ifId);
			block->linkType = linkTypeList->GetItem(ifId);
		}

		me->blockList->Add(block);
		ofst += thisSize;
	}
	DEL_CLASS(linkTypeList);
	DEL_CLASS(resList);
	me->threadRunning = false;
	return 0;
}

IO::FileAnalyse::PCapngFileAnalyse::PCapngFileAnalyse(IO::IStreamData *fd)
{
	UInt8 buff[16];
	this->fd = 0;
	this->threadRunning = false;
	this->pauseParsing = false;
	this->threadToStop = false;
	this->threadStarted = false;
	this->isBE = false;
	NEW_CLASS(this->blockList, Data::SyncArrayList<IO::FileAnalyse::PCapngFileAnalyse::BlockInfo*>());
	this->packetBuff = MemAlloc(UInt8, 65536);
	if (fd->GetRealData(0, 16, buff) != 16)
	{
		return;
	}
	if (ReadNInt32(buff) != 0x0a0d0d0a)
	{
		return;
	}
	if (ReadInt32(&buff[8]) == 0x1a2b3c4d)
	{
		this->fd = fd->GetPartialData(0, fd->GetDataSize());
		this->isBE = false;
	}
	else if (ReadMInt32(&buff[8]) == 0x1a2b3c4d)
	{
		this->fd = fd->GetPartialData(0, fd->GetDataSize());
		this->isBE = true;
	}
	else
	{
		return;
	}
	
	Sync::Thread::Create(ParseThread, this);
	while (!this->threadStarted)
	{
		Sync::Thread::Sleep(10);
	}
}

IO::FileAnalyse::PCapngFileAnalyse::~PCapngFileAnalyse()
{
	if (this->threadRunning)
	{
		this->threadToStop = true;
		while (this->threadRunning)
		{
			Sync::Thread::Sleep(10);
		}
	}

	SDEL_CLASS(this->fd);
	LIST_FREE_FUNC(this->blockList, MemFree);
	DEL_CLASS(this->blockList);
	MemFree(this->packetBuff);
}

UOSInt IO::FileAnalyse::PCapngFileAnalyse::GetFrameCount()
{
	return this->blockList->GetCount();
}

Bool IO::FileAnalyse::PCapngFileAnalyse::GetFrameName(UOSInt index, Text::StringBuilderUTF *sb)
{
	IO::FileAnalyse::PCapngFileAnalyse::BlockInfo *block;
	if (index >= this->blockList->GetCount())
	{
		return false;
	}
	block = this->blockList->GetItem(index);
	fd->GetRealData(block->ofst, block->blockLength, this->packetBuff);
	sb->AppendU64(block->ofst);
	sb->Append((const UTF8Char*)", size=");
	sb->AppendU32(block->blockLength);
	if (block->blockType == 0x0a0d0d0a)
	{
		sb->Append((const UTF8Char*)", SHB");
	}
	else if (block->blockType == 1)
	{
		sb->Append((const UTF8Char*)", IDB");
	}
	else if (block->blockType == 6)
	{
		UInt32 psize;
		sb->Append((const UTF8Char*)", EPB");
		if (this->isBE)
		{
			psize = ReadMUInt32(&this->packetBuff[20]);
		}
		else
		{
			psize = ReadUInt32(&this->packetBuff[20]);
		}
		if (psize + 32 <= block->blockLength)
		{
			sb->Append((const UTF8Char*)", ");
			if (!Net::EthernetAnalyzer::PacketDataGetName(block->linkType, &this->packetBuff[28], psize, sb))
			{
				sb->Append((const UTF8Char*)"Unknown");
			}
		}
	}
	else if (block->blockType == 3)
	{
		UInt32 psize;
		sb->Append((const UTF8Char*)", SPB");
		if (this->isBE)
		{
			psize = ReadMUInt32(&this->packetBuff[8]);
		}
		else
		{
			psize = ReadUInt32(&this->packetBuff[8]);
		}
		if (psize + 32 <= block->blockLength)
		{
			sb->Append((const UTF8Char*)", ");
			if (!Net::EthernetAnalyzer::PacketDataGetName(block->linkType, &this->packetBuff[28], psize, sb))
			{
				sb->Append((const UTF8Char*)"Unknown");
			}
		}
	}
	else if (block->blockType == 4)
	{
		sb->Append((const UTF8Char*)", NRB");
	}
	else if (block->blockType == 5)
	{
		sb->Append((const UTF8Char*)", ISB");
	}
	else if (block->blockType == 9)
	{
		sb->Append((const UTF8Char*)", JEB");
	}
	else if (block->blockType == 10)
	{
		sb->Append((const UTF8Char*)", DSB");
	}
	else if (block->blockType == 0xBAD)
	{
		sb->Append((const UTF8Char*)", CB");
	}
	else if (block->blockType == 0x40000BAD)
	{
		sb->Append((const UTF8Char*)", CBNC");
	}
	return true;
}

Bool IO::FileAnalyse::PCapngFileAnalyse::GetFrameDetail(UOSInt index, Text::StringBuilderUTF *sb)
{
	UTF8Char sbuff[64];
	IO::FileAnalyse::PCapngFileAnalyse::BlockInfo *block;
	if (index >= this->blockList->GetCount())
	{
		return false;
	}
	block = this->blockList->GetItem(index);
	fd->GetRealData(block->ofst, block->blockLength, this->packetBuff);
	sb->Append((const UTF8Char*)"Offset=");
	sb->AppendU64(block->ofst);
	sb->Append((const UTF8Char*)"\r\nType=0x");
	sb->AppendHex32(block->blockType);
	sb->Append((const UTF8Char*)"\r\nTotalSize=");
	sb->AppendU32(block->blockLength);
	sb->Append((const UTF8Char*)"\r\n");
	if (block->blockType == 0x0a0d0d0a)
	{
		sb->Append((const UTF8Char*)"\r\nSection Header Block:");
		sb->Append((const UTF8Char*)"\r\nByte Order=");
		UInt16 majorVer;
		UInt16 minorVer;
		Int64 sectionLength;
		if (this->isBE)
		{
			sb->Append((const UTF8Char*)"Big-endian");
			majorVer = ReadMUInt16(&this->packetBuff[12]);
			minorVer = ReadMUInt16(&this->packetBuff[14]);
			sectionLength = ReadMInt64(&this->packetBuff[16]);
		}
		else
		{
			sb->Append((const UTF8Char*)"Little-endian");
			majorVer = ReadUInt16(&this->packetBuff[12]);
			minorVer = ReadUInt16(&this->packetBuff[14]);
			sectionLength = ReadInt64(&this->packetBuff[16]);
		}
		sb->Append((const UTF8Char*)"\r\nMajor Version=");
		sb->AppendU16(majorVer);
		sb->Append((const UTF8Char*)"\r\nMinor Version=");
		sb->AppendU16(minorVer);
		sb->Append((const UTF8Char*)"\r\nSection Length=");
		sb->AppendI64(sectionLength);
		UInt16 optCode;
		UInt16 optLeng;
		UOSInt i = 24;
		while (i < block->blockLength - 4)
		{
			if (this->isBE)
			{
				optCode = ReadMUInt16(&this->packetBuff[i]);
				optLeng = ReadMUInt16(&this->packetBuff[i + 2]);
			}
			else
			{
				optCode = ReadUInt16(&this->packetBuff[i]);
				optLeng = ReadUInt16(&this->packetBuff[i + 2]);
			}
			sb->Append((const UTF8Char*)"\r\nOption Code=");
			sb->AppendU16(optCode);
			sb->Append((const UTF8Char*)"\r\nOption Length=");
			sb->AppendU16(optLeng);
			if (i + 4 + optLeng > block->blockLength)
			{
				break;
			}
			if (optCode == 0)
			{
				break;
			}
			else if (optCode == 1)
			{
				sb->Append((const UTF8Char*)"\r\nComment=");
				sb->AppendC(&this->packetBuff[i + 4], optLeng);
			}
			else if (optCode == 2)
			{
				sb->Append((const UTF8Char*)"\r\nHardware=");
				sb->AppendC(&this->packetBuff[i + 4], optLeng);
			}
			else if (optCode == 3)
			{
				sb->Append((const UTF8Char*)"\r\nOS=");
				sb->AppendC(&this->packetBuff[i + 4], optLeng);
			}
			else if (optCode == 4)
			{
				sb->Append((const UTF8Char*)"\r\nUser Application=");
				sb->AppendC(&this->packetBuff[i + 4], optLeng);
			}

			i += 4 + (UOSInt)optLeng;
			if (i & 3)
			{
				i += 4 - (i & 3);
			}
		}
	}
	else if (block->blockType == 1)
	{
		sb->Append((const UTF8Char*)"\r\nInterface Description Block:");
		UInt16 linkType;
		UInt16 reserved;
		UInt32 snapLen;
		if (this->isBE)
		{
			linkType = ReadMUInt16(&this->packetBuff[8]);
			reserved = ReadMUInt16(&this->packetBuff[10]);
			snapLen = ReadMUInt32(&this->packetBuff[12]);
		}
		else
		{
			linkType = ReadUInt16(&this->packetBuff[8]);
			reserved = ReadUInt16(&this->packetBuff[10]);
			snapLen = ReadUInt32(&this->packetBuff[12]);
		}
		sb->Append((const UTF8Char*)"\r\nLinkType=");
		sb->AppendU16(linkType);
		const UTF8Char *csptr = Net::EthernetAnalyzer::LinkTypeGetName(linkType);
		if (csptr)
		{
			sb->Append((const UTF8Char*)" (");
			sb->Append(csptr);
			sb->Append((const UTF8Char*)")");
		}
		sb->Append((const UTF8Char*)"\r\nReserved=0x");
		sb->AppendU16(reserved);
		sb->Append((const UTF8Char*)"\r\nSnap Length=");
		sb->AppendU32(snapLen);
		UInt16 optCode;
		UInt16 optLeng;
		UOSInt i = 16;
		while (i < block->blockLength - 4)
		{
			if (this->isBE)
			{
				optCode = ReadMUInt16(&this->packetBuff[i]);
				optLeng = ReadMUInt16(&this->packetBuff[i + 2]);
			}
			else
			{
				optCode = ReadUInt16(&this->packetBuff[i]);
				optLeng = ReadUInt16(&this->packetBuff[i + 2]);
			}
			sb->Append((const UTF8Char*)"\r\nOption Code=");
			sb->AppendU16(optCode);
			sb->Append((const UTF8Char*)"\r\nOption Length=");
			sb->AppendU16(optLeng);
			if (i + 4 + optLeng > block->blockLength)
			{
				break;
			}
			if (optCode == 0)
			{
				break;
			}
			else if (optCode == 1)
			{
				sb->Append((const UTF8Char*)"\r\nComment=");
				sb->AppendC(&this->packetBuff[i + 4], optLeng);
			}
			else if (optCode == 2)
			{
				sb->Append((const UTF8Char*)"\r\nName=");
				sb->AppendC(&this->packetBuff[i + 4], optLeng);
			}
			else if (optCode == 3)
			{
				sb->Append((const UTF8Char*)"\r\nDescription=");
				sb->AppendC(&this->packetBuff[i + 4], optLeng);
			}
			else if (optCode == 4)
			{
				sb->Append((const UTF8Char*)"\r\nIPv4 Address=");
				Net::SocketUtil::GetIPv4Name(sbuff, ReadNUInt32(&this->packetBuff[i + 4]));
				sb->Append(sbuff);
				sb->Append((const UTF8Char*)"\r\nNetmask=");
				Net::SocketUtil::GetIPv4Name(sbuff, ReadNUInt32(&this->packetBuff[i + 8]));
				sb->Append(sbuff);
			}
			else if (optCode == 5)
			{
				Net::SocketUtil::AddressInfo addr;
				Net::SocketUtil::SetAddrInfoV6(&addr, &this->packetBuff[i + 4], 0);
				sb->Append((const UTF8Char*)"\r\nIPv6 Address=");
				Net::SocketUtil::GetAddrName(sbuff, &addr);
				sb->Append(sbuff);
				sb->Append((const UTF8Char*)"/");
				sb->AppendU16(this->packetBuff[i + 20]);
			}
			else if (optCode == 6)
			{
				sb->Append((const UTF8Char*)"\r\nMAC Address=");
				sb->AppendHexBuff(&this->packetBuff[i + 4], 6, ':', Text::LBT_NONE);
			}
			else if (optCode == 7)
			{
				sb->Append((const UTF8Char*)"\r\nEUI Address=");
				sb->AppendHexBuff(&this->packetBuff[i + 4], 8, ' ', Text::LBT_NONE);
			}
			else if (optCode == 8)
			{
				Int64 speed;
				if (this->isBE)
				{
					speed = ReadMInt64(&this->packetBuff[i + 4]);
				}
				else
				{
					speed = ReadInt64(&this->packetBuff[i + 4]);
				}
				sb->Append((const UTF8Char*)"\r\nSpeed=");
				sb->AppendI64(speed);
				sb->Append((const UTF8Char*)"bps");
			}
			else if (optCode == 9)
			{
				sb->Append((const UTF8Char*)"\r\nTimestamps resolution=");
				sb->AppendI16((Int8)this->packetBuff[i + 4]);
			}
			else if (optCode == 10)
			{
				Int32 tzone;
				if (this->isBE)
				{
					tzone = ReadMInt32(&this->packetBuff[i + 4]);
				}
				else
				{
					tzone = ReadInt32(&this->packetBuff[i + 4]);
				}
				sb->Append((const UTF8Char*)"\r\nTime Zone=");
				sb->AppendI32(tzone);
			}
			else if (optCode == 11)
			{
				sb->Append((const UTF8Char*)"\r\nFilter=");
				sb->AppendU16(this->packetBuff[i + 4]);
				if (optLeng > 1)
				{
					sb->Append((const UTF8Char*)" (");
					sb->AppendC(&this->packetBuff[i + 5], (UOSInt)optLeng - 1);
					sb->Append((const UTF8Char*)")");
				}
			}
			else if (optCode == 12)
			{
				sb->Append((const UTF8Char*)"\r\nOS=");
				sb->AppendC(&this->packetBuff[i + 4], (UOSInt)optLeng);
			}
			else if (optCode == 13)
			{
				sb->Append((const UTF8Char*)"\r\nFrame Check Sequence Length=");
				sb->AppendU16(this->packetBuff[i + 4]);
			}
			else if (optCode == 14)
			{
				Int64 tsOffset;
				if (this->isBE)
				{
					tsOffset = ReadMInt64(&this->packetBuff[i + 4]);
				}
				else
				{
					tsOffset = ReadInt64(&this->packetBuff[i + 4]);
				}
				sb->Append((const UTF8Char*)"\r\nTS Offset=");
				sb->AppendI64(tsOffset);
				sb->Append((const UTF8Char*)"sec.");
			}
			else if (optCode == 15)
			{
				sb->Append((const UTF8Char*)"\r\nHardware=");
				sb->AppendHexBuff(&this->packetBuff[i + 4], 8, ' ', Text::LBT_NONE);
			}

			i += 4 + (UOSInt)optLeng;
			if (i & 3)
			{
				i += 4 - (i & 3);
			}
		}
	}
	else if (block->blockType == 6)
	{
		sb->Append((const UTF8Char*)"\r\nEnhanced Packet Block:");
		UInt32 ifId;
		Int64 ts;
		UInt32 capPSize;
		UInt32 oriPSize;
		Data::DateTime dt;
		if (this->isBE)
		{
			ifId = ReadMUInt32(&this->packetBuff[8]);
			ts = ReadMInt64(&this->packetBuff[12]);
			capPSize = ReadMUInt32(&this->packetBuff[20]);
			oriPSize = ReadMUInt32(&this->packetBuff[24]);
		}
		else
		{
			ifId = ReadUInt32(&this->packetBuff[8]);
			ts = (((Int64)ReadInt32(&this->packetBuff[12])) << 32) | ReadUInt32(&this->packetBuff[16]);
			capPSize = ReadUInt32(&this->packetBuff[20]);
			oriPSize = ReadUInt32(&this->packetBuff[24]);
		}
		sb->Append((const UTF8Char*)"\r\nInterface ID=");
		sb->AppendU32(ifId);
		SetTime(&dt, ts, block->timeResol);
		dt.ToLocalTime();
		dt.ToString(sbuff, "yyyy-MM-dd HH:mm:ss.fff");
		sb->Append((const UTF8Char*)"\r\nTime=");
		sb->Append(sbuff);
		sb->Append((const UTF8Char*)"\r\nCaptured Packet Length=");
		sb->AppendU32(capPSize);
		sb->Append((const UTF8Char*)"\r\nOriginal Packet Length=");
		sb->AppendU32(oriPSize);
		if (capPSize + 32 <= block->blockLength)
		{
			Net::EthernetAnalyzer::PacketDataGetDetail(block->linkType, &this->packetBuff[28], capPSize, sb);

			UInt16 optCode;
			UInt16 optLeng;
			UOSInt i = 28 + capPSize;
			if (i & 3)
			{
				i += 4 - (i & 3);
			}
			while (i < block->blockLength - 4)
			{
				if (this->isBE)
				{
					optCode = ReadMUInt16(&this->packetBuff[i]);
					optLeng = ReadMUInt16(&this->packetBuff[i + 2]);
				}
				else
				{
					optCode = ReadUInt16(&this->packetBuff[i]);
					optLeng = ReadUInt16(&this->packetBuff[i + 2]);
				}
				sb->Append((const UTF8Char*)"\r\nOption Code=");
				sb->AppendU16(optCode);
				sb->Append((const UTF8Char*)"\r\nOption Length=");
				sb->AppendU16(optLeng);
				if (i + 4 + optLeng > block->blockLength)
				{
					break;
				}
				if (optCode == 0)
				{
					break;
				}
				else if (optCode == 1)
				{
					sb->Append((const UTF8Char*)"\r\nComment=");
					sb->AppendC(&this->packetBuff[i + 4], optLeng);
				}
				else if (optCode == 2)
				{
					sb->Append((const UTF8Char*)"\r\nHardware=");
					sb->AppendC(&this->packetBuff[i + 4], optLeng);
				}
				else if (optCode == 3)
				{
					sb->Append((const UTF8Char*)"\r\nOS=");
					sb->AppendC(&this->packetBuff[i + 4], optLeng);
				}
				else if (optCode == 4)
				{
					sb->Append((const UTF8Char*)"\r\nUser Application=");
					sb->AppendC(&this->packetBuff[i + 4], optLeng);
				}

				i += 4 + (UOSInt)optLeng;
				if (i & 3)
				{
					i += 4 - (i & 3);
				}
			}
		}
		else
		{
			sb->Append((const UTF8Char*)"\r\n");
			sb->Append((const UTF8Char*)"\r\n");
			sb->AppendHexBuff(&this->packetBuff[28], block->blockLength - 32, ' ', Text::LBT_CRLF);
		}
	}
	else if (block->blockType == 5)
	{
		sb->Append((const UTF8Char*)"\r\nInterface Statistics Block:");
		UInt32 ifId;
		Int64 ts;
		Data::DateTime dt;
		if (this->isBE)
		{
			ifId = ReadMUInt32(&this->packetBuff[8]);
			ts = ReadMInt64(&this->packetBuff[12]);
		}
		else
		{
			ifId = ReadUInt32(&this->packetBuff[8]);
			ts = (((Int64)ReadInt32(&this->packetBuff[12])) << 32) | ReadUInt32(&this->packetBuff[16]);
		}
		sb->Append((const UTF8Char*)"\r\nInterface ID=");
		sb->AppendU32(ifId);
		SetTime(&dt, ts, block->timeResol);
		dt.ToLocalTime();
		dt.ToString(sbuff, "yyyy-MM-dd HH:mm:ss.fff");
		sb->Append((const UTF8Char*)"\r\nTime=");
		sb->Append(sbuff);
		UInt16 optCode;
		UInt16 optLeng;
		UOSInt i = 20;
		while (i < block->blockLength - 4)
		{
			if (this->isBE)
			{
				optCode = ReadMUInt16(&this->packetBuff[i]);
				optLeng = ReadMUInt16(&this->packetBuff[i + 2]);
			}
			else
			{
				optCode = ReadUInt16(&this->packetBuff[i]);
				optLeng = ReadUInt16(&this->packetBuff[i + 2]);
			}
			sb->Append((const UTF8Char*)"\r\nOption Code=");
			sb->AppendU16(optCode);
			sb->Append((const UTF8Char*)"\r\nOption Length=");
			sb->AppendU16(optLeng);
			if (i + 4 + optLeng > block->blockLength)
			{
				break;
			}
			if (optCode == 0)
			{
				break;
			}
			else if (optCode == 1)
			{
				sb->Append((const UTF8Char*)"\r\nComment=");
				sb->AppendC(&this->packetBuff[i + 4], optLeng);
			}
			else if (optCode == 2)
			{
				sb->Append((const UTF8Char*)"\r\nStart Time=");
				if (this->isBE)
				{
					ts = ReadMInt64(&this->packetBuff[i + 4]);
				}
				else
				{
					ts = (((Int64)ReadInt32(&this->packetBuff[i + 4])) << 32) | ReadUInt32(&this->packetBuff[i + 8]);
				}
				SetTime(&dt, ts, block->timeResol);
				dt.ToLocalTime();
				dt.ToString(sbuff, "yyyy-MM-dd HH:mm:ss.fff");
				sb->Append(sbuff);
			}
			else if (optCode == 3)
			{
				sb->Append((const UTF8Char*)"\r\nEnd Time=");
				if (this->isBE)
				{
					ts = ReadMInt64(&this->packetBuff[i + 4]);
				}
				else
				{
					ts = (((Int64)ReadInt32(&this->packetBuff[i + 4])) << 32) | ReadUInt32(&this->packetBuff[i + 8]);
				}
				SetTime(&dt, ts, block->timeResol);
				dt.ToLocalTime();
				dt.ToString(sbuff, "yyyy-MM-dd HH:mm:ss.fff");
				sb->Append(sbuff);
			}
			else if (optCode == 4)
			{
				sb->Append((const UTF8Char*)"\r\nReceived Packets=");
				if (this->isBE)
				{
					ts = ReadMInt64(&this->packetBuff[i + 4]);
				}
				else
				{
					ts = ReadInt64(&this->packetBuff[i + 4]);
				}
				sb->AppendI64(ts);
			}
			else if (optCode == 5)
			{
				sb->Append((const UTF8Char*)"\r\nDropped Packets=");
				if (this->isBE)
				{
					ts = ReadMInt64(&this->packetBuff[i + 4]);
				}
				else
				{
					ts = ReadInt64(&this->packetBuff[i + 4]);
				}
				sb->AppendI64(ts);
			}
			else if (optCode == 6)
			{
				sb->Append((const UTF8Char*)"\r\nPackets Accepted by Filter=");
				if (this->isBE)
				{
					ts = ReadMInt64(&this->packetBuff[i + 4]);
				}
				else
				{
					ts = ReadInt64(&this->packetBuff[i + 4]);
				}
				sb->AppendI64(ts);
			}
			else if (optCode == 7)
			{
				sb->Append((const UTF8Char*)"\r\nPackets Dropped by OS=");
				if (this->isBE)
				{
					ts = ReadMInt64(&this->packetBuff[i + 4]);
				}
				else
				{
					ts = ReadInt64(&this->packetBuff[i + 4]);
				}
				sb->AppendI64(ts);
			}
			else if (optCode == 8)
			{
				sb->Append((const UTF8Char*)"\r\nPackets Delivered to the user=");
				if (this->isBE)
				{
					ts = ReadMInt64(&this->packetBuff[i + 4]);
				}
				else
				{
					ts = ReadInt64(&this->packetBuff[i + 4]);
				}
				sb->AppendI64(ts);
			}

			i += 4 + (UOSInt)optLeng;
			if (i & 3)
			{
				i += 4 - (i & 3);
			}
		}
	}
	return true;
}

Bool IO::FileAnalyse::PCapngFileAnalyse::IsError()
{
	return this->fd == 0;
}

Bool IO::FileAnalyse::PCapngFileAnalyse::IsParsing()
{
	return this->threadRunning;
}

Bool IO::FileAnalyse::PCapngFileAnalyse::TrimPadding(const UTF8Char *outputFile)
{
	return false;
}

void IO::FileAnalyse::PCapngFileAnalyse::SetTime(Data::DateTime *dt, Int64 ts, Int8 timeResol)
{
	switch (timeResol)
	{
	case 6:
		dt->SetTicks(ts / 1000LL);
		break;
	case 9:
		dt->SetTicks(ts / 1000000LL);
		break;
	default:
		dt->SetTicks(ts);
		break;
	}
}
