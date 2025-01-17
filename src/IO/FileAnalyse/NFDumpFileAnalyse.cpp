#include "Stdafx.h"
#include "Data/ByteTool.h"
#include "Data/Compress/LZODecompressor.h"
#include "IO/FileStream.h"
#include "IO/Path.h"
#include "IO/FileAnalyse/NFDumpFileAnalyse.h"
#include "Manage/Process.h"
#include "Net/SocketFactory.h"
#include "Sync/Thread.h"

#define LZOPROGRAM L"LZOBlockDecomp.exe"

UInt32 __stdcall IO::FileAnalyse::NFDumpFileAnalyse::ParseThread(void *userObj)
{
	IO::FileAnalyse::NFDumpFileAnalyse *me = (IO::FileAnalyse::NFDumpFileAnalyse*)userObj;
	UInt64 endOfst;
	UInt64 ofst;
	UInt8 buff[12];
	UInt32 sz;
	IO::FileAnalyse::NFDumpFileAnalyse::PackInfo *pack;
	me->threadRunning = true;
	me->threadStarted = true;

	pack = MemAlloc(IO::FileAnalyse::NFDumpFileAnalyse::PackInfo, 1);
	pack->fileOfst = 0;
	pack->packSize = 140;
	pack->packType = 0;
	me->packs->Add(pack);
	pack = MemAlloc(IO::FileAnalyse::NFDumpFileAnalyse::PackInfo, 1);
	pack->fileOfst = 140;
	pack->packSize = 136;
	pack->packType = 1;
	me->packs->Add(pack);

	endOfst = me->fd->GetDataSize();
	ofst = 276;
	while (ofst <= (endOfst - 12) && !me->threadToStop)
	{
		if (me->pauseParsing)
		{
			Sync::Thread::Sleep(100);
		}
		else
		{
			me->fd->GetRealData(ofst, 12, buff);
			sz = ReadUInt32(&buff[4]);
			if (ofst + sz + 12 > endOfst)
			{
				break;
			}

			pack = MemAlloc(IO::FileAnalyse::NFDumpFileAnalyse::PackInfo, 1);
			pack->fileOfst = ofst;
			pack->packSize = 12;
			pack->packType = 2;
			me->packs->Add(pack);
			
			pack = MemAlloc(IO::FileAnalyse::NFDumpFileAnalyse::PackInfo, 1);
			pack->fileOfst = ofst + 12;
			pack->packSize = sz;
			pack->packType = 3;
			me->packs->Add(pack);

			ofst += sz + 12;
		}
	}
	me->threadRunning = false;
	return 0;
}

UOSInt IO::FileAnalyse::NFDumpFileAnalyse::LZODecompBlock(UInt8 *srcBlock, UOSInt srcSize, UInt8 *outBlock, UOSInt maxOutSize)
{
	Data::Compress::LZODecompressor dec;
	UOSInt destSize;
	if (dec.Decompress(outBlock, &destSize, srcBlock, srcSize))
	{
		return destSize;
	}
	else
	{
		return 0;
	}
}

IO::FileAnalyse::NFDumpFileAnalyse::NFDumpFileAnalyse(IO::IStreamData *fd)
{
	UInt8 buff[256];
	this->fd = 0;
	this->threadRunning = false;
	this->pauseParsing = false;
	this->threadToStop = false;
	this->threadStarted = false;
	NEW_CLASS(this->extMap, Data::Int32Map<UInt8*>());

	this->hasLZODecomp = true;

	NEW_CLASS(this->packs, Data::SyncArrayList<IO::FileAnalyse::NFDumpFileAnalyse::PackInfo*>());
	fd->GetRealData(0, 256, buff);
	if (buff[0] != 0x0c || buff[1] != 0xa5 || buff[2] != 1 || buff[3] != 0)
	{
		return;
	}
	this->fd = fd->GetPartialData(0, fd->GetDataSize());
	Sync::Thread::Create(ParseThread, this);
	while (!this->threadStarted)
	{
		Sync::Thread::Sleep(10);
	}
}

IO::FileAnalyse::NFDumpFileAnalyse::~NFDumpFileAnalyse()
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
	LIST_FREE_FUNC(this->packs, MemFree);
	Data::ArrayList<UInt8*> *extList = this->extMap->GetValues();
	LIST_FREE_FUNC(extList, MemFree);
	DEL_CLASS(this->extMap);
	DEL_CLASS(this->packs);
}

UOSInt IO::FileAnalyse::NFDumpFileAnalyse::GetFrameCount()
{
	return this->packs->GetCount();
}

Bool IO::FileAnalyse::NFDumpFileAnalyse::GetFrameName(UOSInt index, Text::StringBuilderUTF *sb)
{
	IO::FileAnalyse::NFDumpFileAnalyse::PackInfo *pack;
	pack = this->packs->GetItem(index);
	if (pack == 0)
		return false;
	sb->AppendU64(pack->fileOfst);
	sb->Append((const UTF8Char*)": Type=");
	if (pack->packType == 0)
	{
		sb->Append((const UTF8Char*)"File header");
	}
	else if (pack->packType == 1)
	{
		sb->Append((const UTF8Char*)"Stat Record");
	}
	else if (pack->packType == 2)
	{
		sb->Append((const UTF8Char*)"Data Block Header");
	}
	else if (pack->packType == 3)
	{
		sb->Append((const UTF8Char*)"Data Block");
	}
	sb->Append((const UTF8Char*)", size=");
	sb->AppendI32((Int32)pack->packSize);
	return true;
}

Bool IO::FileAnalyse::NFDumpFileAnalyse::GetFrameDetail(UOSInt index, Text::StringBuilderUTF *sb)
{
	UTF8Char sbuff[64];
	UInt8 *extBuff;
	IO::FileAnalyse::NFDumpFileAnalyse::PackInfo *pack;
	UInt8 *packBuff;
	UOSInt i;
	UOSInt j;
	UOSInt k;
	UOSInt l;
	pack = this->packs->GetItem(index);
	if (pack == 0)
		return false;

	sb->AppendU64(pack->fileOfst);
	sb->Append((const UTF8Char*)": Type=");
	if (pack->packType == 0)
	{
		sb->Append((const UTF8Char*)"File header");
	}
	else if (pack->packType == 1)
	{
		sb->Append((const UTF8Char*)"Stat Record");
	}
	else if (pack->packType == 2)
	{
		sb->Append((const UTF8Char*)"Data Block Header");
	}
	else if (pack->packType == 3)
	{
		sb->Append((const UTF8Char*)"Data Block");
	}
	sb->Append((const UTF8Char*)", size=");
	sb->AppendI32((Int32)pack->packSize);

	if (pack->packType == 0)
	{
		packBuff = MemAlloc(UInt8, pack->packSize);
		this->fd->GetRealData(pack->fileOfst, pack->packSize, packBuff);

		sb->Append((const UTF8Char*)"\r\nVersion = ");
		sb->AppendU16(ReadUInt16(&packBuff[2]));
		sb->Append((const UTF8Char*)"\r\nFlags = 0x");
		sb->AppendHex32V(ReadUInt32(&packBuff[4]));
		if (packBuff[4] & 1)
		{
			sb->Append((const UTF8Char*)" Compressed");
		}
		if (packBuff[4] & 2)
		{
			sb->Append((const UTF8Char*)" Anonimized");
		}
		if (packBuff[4] & 4)
		{
			sb->Append((const UTF8Char*)" Catalog");
		}
		sb->Append((const UTF8Char*)"\r\nNumber of Blocks = ");
		sb->AppendU32(ReadUInt32(&packBuff[8]));
		sb->Append((const UTF8Char*)"\r\nIdentifier = ");
		sb->Append((UTF8Char*)&packBuff[12]);

		MemFree(packBuff);
	}
	else if (pack->packType == 1)
	{
		packBuff = MemAlloc(UInt8, pack->packSize);
		this->fd->GetRealData(pack->fileOfst, pack->packSize, packBuff);

		sb->Append((const UTF8Char*)"\r\nNumber of flows = ");
		sb->AppendI64(ReadInt64(&packBuff[0]));
		sb->Append((const UTF8Char*)"\r\nNumber of bytes = ");
		sb->AppendI64(ReadInt64(&packBuff[8]));
		sb->Append((const UTF8Char*)"\r\nNumber of packets = ");
		sb->AppendI64(ReadInt64(&packBuff[16]));
		sb->Append((const UTF8Char*)"\r\nNumber of flows (TCP) = ");
		sb->AppendI64(ReadInt64(&packBuff[24]));
		sb->Append((const UTF8Char*)"\r\nNumber of flows (UDP) = ");
		sb->AppendI64(ReadInt64(&packBuff[32]));
		sb->Append((const UTF8Char*)"\r\nNumber of flows (ICMP) = ");
		sb->AppendI64(ReadInt64(&packBuff[40]));
		sb->Append((const UTF8Char*)"\r\nNumber of flows (Other) = ");
		sb->AppendI64(ReadInt64(&packBuff[48]));
		sb->Append((const UTF8Char*)"\r\nNumber of bytes (TCP) = ");
		sb->AppendI64(ReadInt64(&packBuff[56]));
		sb->Append((const UTF8Char*)"\r\nNumber of bytes (UDP) = ");
		sb->AppendI64(ReadInt64(&packBuff[64]));
		sb->Append((const UTF8Char*)"\r\nNumber of bytes (ICMP) = ");
		sb->AppendI64(ReadInt64(&packBuff[72]));
		sb->Append((const UTF8Char*)"\r\nNumber of bytes (Other) = ");
		sb->AppendI64(ReadInt64(&packBuff[80]));
		sb->Append((const UTF8Char*)"\r\nNumber of packets (TCP) = ");
		sb->AppendI64(ReadInt64(&packBuff[88]));
		sb->Append((const UTF8Char*)"\r\nNumber of packets (UDP) = ");
		sb->AppendI64(ReadInt64(&packBuff[96]));
		sb->Append((const UTF8Char*)"\r\nNumber of packets (ICMP) = ");
		sb->AppendI64(ReadInt64(&packBuff[104]));
		sb->Append((const UTF8Char*)"\r\nNumber of packets (Other) = ");
		sb->AppendI64(ReadInt64(&packBuff[112]));
		sb->Append((const UTF8Char*)"\r\nFirst seen = ");
		sb->AppendU32(ReadUInt32(&packBuff[120]));
		sb->Append((const UTF8Char*)"\r\nLast seen = ");
		sb->AppendU32(ReadUInt32(&packBuff[124]));
		sb->Append((const UTF8Char*)"\r\nmSec First = ");
		sb->AppendU16(ReadUInt16(&packBuff[128]));
		sb->Append((const UTF8Char*)"\r\nmSec Last = ");
		sb->AppendU16(ReadUInt16(&packBuff[130]));
		sb->Append((const UTF8Char*)"\r\nSequence Failure = ");
		sb->AppendU32(ReadUInt32(&packBuff[132]));

		MemFree(packBuff);
	}
	else if (pack->packType == 2)
	{
		packBuff = MemAlloc(UInt8, pack->packSize);
		this->fd->GetRealData(pack->fileOfst, pack->packSize, packBuff);

		sb->Append((const UTF8Char*)"\r\nNumber of records = ");
		sb->AppendU32(ReadUInt32(&packBuff[0]));
		sb->Append((const UTF8Char*)"\r\nBlock size = ");
		sb->AppendU32(ReadUInt32(&packBuff[4]));
		sb->Append((const UTF8Char*)"\r\nBlock ID = ");
		sb->AppendU16(ReadUInt16(&packBuff[8]));
		sb->Append((const UTF8Char*)"\r\nFlags = ");
		sb->AppendU16(ReadUInt16(&packBuff[10]));
		if (packBuff[10] == 0)
		{
			sb->Append((const UTF8Char*)" kompatibility");
		}
		else if (packBuff[10] == 1)
		{
			sb->Append((const UTF8Char*)" block uncompressed");
		}
		else if (packBuff[10] == 2)
		{
			sb->Append((const UTF8Char*)" block compressed");
		}
		MemFree(packBuff);
	}
	else if (pack->packType == 3)
	{
		UOSInt size = pack->packSize;
		UOSInt dispSize = size;
		if (dispSize > 256)
			dispSize = 256;
		packBuff = MemAlloc(UInt8, size);
		this->fd->GetRealData(pack->fileOfst, size, packBuff);

		sb->Append((const UTF8Char*)"\r\n\r\n");
		sb->AppendHexBuff(packBuff, dispSize, ' ', Text::LBT_CRLF);
		if (this->hasLZODecomp)
		{
			Data::DateTime dt;
			UOSInt decBuffSize = 1048576 * 5;
			UInt8 *decBuff = MemAlloc(UInt8, decBuffSize);
			decBuffSize = this->LZODecompBlock(packBuff, size, decBuff, decBuffSize);
			if (decBuffSize > 0)
			{
				UInt32 recType;
				UInt32 recSize;
				sb->Append((const UTF8Char*)"\r\n");
				i = 0;
				while (i < decBuffSize)
				{
					recType = ReadUInt16(&decBuff[i]);
					recSize = ReadUInt16(&decBuff[i + 2]);
					sb->Append((const UTF8Char*)"\r\nRec Size = ");
					sb->AppendU32(recSize);
					sb->Append((const UTF8Char*)", Rec Type = ");
					sb->AppendU32(recType);
					if (recSize < 4)
						break;
					if (recType == 1)
					{
						sb->Append((const UTF8Char*)" (CommonRecordV0)");
						sb->Append((const UTF8Char*)", Flags = 0x");
						sb->AppendHex8(decBuff[i + 4]);
						sb->Append((const UTF8Char*)", Exporter Sys ID = ");
						sb->AppendU16(decBuff[i + 5]);
						sb->Append((const UTF8Char*)", Ext Map ID = ");
						sb->AppendU16(ReadUInt16(&decBuff[i + 6]));
						dt.SetUnixTimestamp(ReadUInt32(&decBuff[i + 12]));
						dt.AddMS(ReadUInt16(&decBuff[i + 8]));
						dt.ToString(sbuff, "yyyy-MM-dd HH:mm:ss.fffzz");
						sb->Append((const UTF8Char*)", first = ");
						sb->Append(sbuff);
						dt.SetUnixTimestamp(ReadUInt32(&decBuff[i + 16]));
						dt.AddMS(ReadUInt16(&decBuff[i + 10]));
						dt.ToString(sbuff, "yyyy-MM-dd HH:mm:ss.fffzz");
						sb->Append((const UTF8Char*)", last = ");
						sb->Append(sbuff);

						sb->Append((const UTF8Char*)", Fwd Status = ");
						sb->AppendU16(decBuff[i + 20]);
						sb->Append((const UTF8Char*)", TCP Flags = ");
						sb->AppendU16(decBuff[i + 21]);
						sb->Append((const UTF8Char*)", Prot = ");
						sb->AppendU16(decBuff[i + 22]);
						sb->Append((const UTF8Char*)", Tos = ");
						sb->AppendU16(decBuff[i + 23]);
						sb->Append((const UTF8Char*)", src Port = ");
						sb->AppendU16(ReadUInt16(&decBuff[i + 24]));
						sb->Append((const UTF8Char*)", dest port = ");
						sb->AppendU16(ReadUInt16(&decBuff[i + 26]));
						j = 28;
						if (decBuff[i + 4] & 1) //IPv6
						{
							sb->Append((const UTF8Char*)", src IP = ");
//							Net::SocketUtil::GetIPv6Name(sbuff, ReadUInt32(&decBuff[i + j]));
//							sb->Append(sbuff);
							sb->Append((const UTF8Char*)", dest IP = ");
//							Net::SocketUtil::GetIPv6Name(sbuff, ReadUInt32(&decBuff[i + j + 16]));
//							sb->Append(sbuff);
							j += 32;
						}
						else //IPv4
						{
							sb->Append((const UTF8Char*)", src IP = ");
							Net::SocketUtil::GetIPv4Name(sbuff, ReadMUInt32(&decBuff[i + j]));
							sb->Append(sbuff);
							sb->Append((const UTF8Char*)", dest IP = ");
							Net::SocketUtil::GetIPv4Name(sbuff, ReadMUInt32(&decBuff[i + j + 4]));
							sb->Append(sbuff);
							j += 8;
						}
						sb->Append((const UTF8Char*)", In Pkts = ");
						if (decBuff[i + 4] & 2) //64 bit
						{
							sb->AppendU64(ReadUInt64(&decBuff[i + j]));
							j += 8;
						}
						else //32 bit
						{
							sb->AppendU32(ReadUInt32(&decBuff[i + j]));
							j += 4;
						}
						sb->Append((const UTF8Char*)", In Bytes = ");
						if (decBuff[i + 4] & 4) //64 bit
						{
							sb->AppendU64(ReadUInt64(&decBuff[i + j]));
							j += 8;
						}
						else //32 bit
						{
							sb->AppendU32(ReadUInt32(&decBuff[i + j]));
							j += 4;
						}
						extBuff = this->extMap->Get(ReadUInt16(&decBuff[i + 6]));
						if (extBuff)
						{
							OSInt extId;
							k = ReadUInt16(&extBuff[2]);
							l = 4;
							while (l < k)
							{
								extId = ReadUInt16(&extBuff[l]);
								if (extId == 4) //EX_IO_SNMP_2
								{
									sb->Append((const UTF8Char*)", input = ");
									sb->AppendU16(ReadUInt16(&decBuff[i + j]));
									sb->Append((const UTF8Char*)", output = ");
									sb->AppendU16(ReadUInt16(&decBuff[i + j + 2]));
									j += 4;
								}
								else if (extId == 5) //EX_IO_SNMP_4
								{
									sb->Append((const UTF8Char*)", input = ");
									sb->AppendU32(ReadUInt32(&decBuff[i + j]));
									sb->Append((const UTF8Char*)", output = ");
									sb->AppendU32(ReadUInt32(&decBuff[i + j + 4]));
									j += 8;
								}
								else if (extId == 6) //EX_AS_2
								{
									sb->Append((const UTF8Char*)", Src AS = ");
									sb->AppendU16(ReadUInt16(&decBuff[i + j]));
									sb->Append((const UTF8Char*)", Dest AS = ");
									sb->AppendU16(ReadUInt16(&decBuff[i + j + 2]));
									j += 4;
								}
								else if (extId == 7) //EX_AS_4
								{
									sb->Append((const UTF8Char*)", Src AS = ");
									sb->AppendU32(ReadUInt32(&decBuff[i + j]));
									sb->Append((const UTF8Char*)", dest AS = ");
									sb->AppendU32(ReadUInt32(&decBuff[i + j + 4]));
									j += 8;
								}
								else if (extId == 8) //EX_MULTIPLE
								{
									sb->Append((const UTF8Char*)", Dst Tos = ");
									sb->AppendU16(decBuff[i + j]);
									sb->Append((const UTF8Char*)", Dir = ");
									sb->AppendU16(decBuff[i + j + 1]);
									sb->Append((const UTF8Char*)", Src Mask = ");
									sb->AppendU16(decBuff[i + j + 2]);
									sb->Append((const UTF8Char*)", Dst Mask = ");
									sb->AppendU16(decBuff[i + j + 3]);
									j += 4;
								}
								else if (extId == 9) //EX_NEXT_HOP_v4
								{
									sb->Append((const UTF8Char*)", Next HOP IP = ");
									Net::SocketUtil::GetIPv4Name(sbuff, ReadMUInt32(&decBuff[i + j]));
									sb->Append(sbuff);
									j += 4;
								}
								else if (extId == 10) //EX_NEXT_HOP_v6
								{
									sb->Append((const UTF8Char*)", Next HOP IP = ");
//									Net::SocketUtil::GetIPv4Name(sbuff, ReadMUInt32(&decBuff[i + j]));
//									sb->Append(sbuff);
									j += 16;
								}
								else if (extId == 11) //EX_NEXT_HOP_BGP_v4
								{
									sb->Append((const UTF8Char*)", BGP Next IP = ");
									Net::SocketUtil::GetIPv4Name(sbuff, ReadMUInt32(&decBuff[i + j]));
									sb->Append(sbuff);
									j += 4;
								}
								else if (extId == 12) //EX_NEXT_HOP_BGP_v6
								{
									sb->Append((const UTF8Char*)", BGP Next IP = ");
//									Net::SocketUtil::GetIPv4Name(sbuff, ReadMUInt32(&decBuff[i + j]));
//									sb->Append(sbuff);
									j += 16;
								}
								else if (extId == 13) //EX_VLAN
								{
									sb->Append((const UTF8Char*)", Src VLAN = ");
									sb->AppendU16(ReadUInt16(&decBuff[i + j]));
									sb->Append((const UTF8Char*)", Dst VLAN = ");
									sb->AppendU16(ReadUInt16(&decBuff[i + j + 2]));
									j += 4;
								}
								else if (extId == 14) //EX_OUT_PKG_4
								{
									sb->Append((const UTF8Char*)", Out Pkts = ");
									sb->AppendU32(ReadUInt32(&decBuff[i + j]));
									j += 4;
								}
								else if (extId == 15) //EX_OUT_PKG_8
								{
									sb->Append((const UTF8Char*)", Out Pkts = ");
									sb->AppendU64(ReadUInt64(&decBuff[i + j]));
									j += 8;
								}
								else if (extId == 16) //EX_OUT_BYTES_4
								{
									sb->Append((const UTF8Char*)", Out Bytes = ");
									sb->AppendU32(ReadUInt32(&decBuff[i + j]));
									j += 4;
								}
								else if (extId == 17) //EX_OUT_BYTES_8
								{
									sb->Append((const UTF8Char*)", Out Bytes = ");
									sb->AppendU64(ReadUInt64(&decBuff[i + j]));
									j += 8;
								}
								else if (extId == 18) //EX_AGGR_FLOWS_4
								{
									sb->Append((const UTF8Char*)", Aggr Flows = ");
									sb->AppendU32(ReadUInt32(&decBuff[i + j]));
									j += 4;
								}
								else if (extId == 19) //EX_AGGR_FLOWS_8
								{
									sb->Append((const UTF8Char*)", Aggr Flows = ");
									sb->AppendU64(ReadUInt64(&decBuff[i + j]));
									j += 8;
								}
								else if (extId == 20) //EX_MAC_1
								{
									sb->Append((const UTF8Char*)", In Src MAC = ");
									sb->AppendHexBuff(&decBuff[i + j + 2], 6, 0, Text::LBT_NONE);
									sb->Append((const UTF8Char*)", Out Dst MAC = ");
									sb->AppendHexBuff(&decBuff[i + j + 10], 6, 0, Text::LBT_NONE);
									j += 16;
								}
								else if (extId == 21) //EX_MAC_2
								{
									sb->Append((const UTF8Char*)", In Dst MAC = ");
									sb->AppendHexBuff(&decBuff[i + j + 2], 6, 0, Text::LBT_NONE);
									sb->Append((const UTF8Char*)", Out Src MAC = ");
									sb->AppendHexBuff(&decBuff[i + j + 10], 6, 0, Text::LBT_NONE);
									j += 16;
								}
								else if (extId == 22) //EX_MPLS
								{
									sb->Append((const UTF8Char*)", MPLS LABEL 1 = ");
									sb->Append((const UTF8Char*)", MPLS LABEL 2 = ");
									sb->Append((const UTF8Char*)", MPLS LABEL 3 = ");
									sb->Append((const UTF8Char*)", MPLS LABEL 4 = ");
									sb->Append((const UTF8Char*)", MPLS LABEL 5 = ");
									sb->Append((const UTF8Char*)", MPLS LABEL 6 = ");
									sb->Append((const UTF8Char*)", MPLS LABEL 7 = ");
									sb->Append((const UTF8Char*)", MPLS LABEL 8 = ");
									sb->Append((const UTF8Char*)", MPLS LABEL 9 = ");
									sb->Append((const UTF8Char*)", MPLS LABEL 10 = ");
									j += 40;
								}
								else if (extId == 23) //EX_ROUTER_IP_v4
								{
									sb->Append((const UTF8Char*)", Router IP = ");
									Net::SocketUtil::GetIPv4Name(sbuff, ReadMUInt32(&decBuff[i + j]));
									sb->Append(sbuff);
									j += 4;
								}
								else if (extId == 24) //EX_ROUTER_IP_v6
								{
									sb->Append((const UTF8Char*)", Router IP = ");
//									Net::SocketUtil::GetIPv4Name(sbuff, ReadMUInt32(&decBuff[i + j]));
//									sb->Append(sbuff);
									j += 16;
								}
								else if (extId == 25) //EX_ROUTER_ID
								{
									sb->Append((const UTF8Char*)", Engine Type = ");
									sb->AppendU16(decBuff[i + j + 2]);
									sb->Append((const UTF8Char*)", Engine ID = ");
									sb->AppendU16(decBuff[i + j + 3]);
									j += 4;
								}
								else if (extId == 26) //EX_BGPADJ
								{
									sb->Append((const UTF8Char*)", BGP Next Adjacent AS Number = ");
									sb->AppendU32(ReadUInt32(&decBuff[i + j]));
									sb->Append((const UTF8Char*)", BGP Prev Adjacent AS Number = ");
									sb->AppendU32(ReadUInt32(&decBuff[i + j + 4]));
									j += 8;
								}
								else if (extId == 27) //EX_BGPADJ
								{
									sb->Append((const UTF8Char*)", T Received = ");
									dt.SetTicks(ReadInt64(&decBuff[i + j]));
									dt.ToString(sbuff, "yyyy-MM-dd HH:mm:ss.fffzz");
									sb->Append(sbuff);
									j += 8;
								}
								if (j >= recSize)
									break;
								l += 2;
							}
						}
					}
					else if (recType == 2)
					{
						sb->Append((const UTF8Char*)" (ExtensionMap)");
						sb->Append((const UTF8Char*)", Map ID = ");
						sb->AppendU16(ReadUInt16(&decBuff[i + 4]));
						sb->Append((const UTF8Char*)", Extension Size = ");
						sb->AppendU16(ReadUInt16(&decBuff[i + 6]));
						j = 1;
						k = 8;
						while (k < recSize)
						{
							l = ReadUInt16(&decBuff[i + k]);
							if (l == 0)
								break;
							sb->Append((const UTF8Char*)", Extension ID");
							sb->AppendUOSInt(j);
							sb->Append((const UTF8Char*)" = ");
							sb->AppendUOSInt(l);
							j++;
							k += 2;
						}
						extBuff = this->extMap->Get(ReadUInt16(&decBuff[i + 4]));
						if (extBuff == 0)
						{
							extBuff = MemAlloc(UInt8, recSize - 4);
							MemCopyNO(extBuff, &decBuff[i + 4], recSize - 4);
							WriteInt16(&extBuff[2], recSize - 4);
							this->extMap->Put(ReadUInt16(&decBuff[i + 4]), extBuff);
						}
					}
					else if (recType == 3)
					{
						sb->Append((const UTF8Char*)" (PortHistogram)");
					}
					else if (recType == 4)
					{
						sb->Append((const UTF8Char*)" (BppHistogram)");
					}
					else if (recType == 5)
					{
						sb->Append((const UTF8Char*)" (ExporterRecord)");
					}
					else if (recType == 6)
					{
						sb->Append((const UTF8Char*)" (SamplerRecord)");
					}
					else if (recType == 7)
					{
						sb->Append((const UTF8Char*)" (ExporterInfoRecord)");
						sb->Append((const UTF8Char*)", version = ");
						sb->AppendU32(ReadUInt32(&decBuff[i + 4]));
						sb->Append((const UTF8Char*)", SA Family = ");
						sb->AppendUOSInt(j = ReadUInt16(&decBuff[i + 24]));
						sb->Append((const UTF8Char*)", IP = ");
						if (j == 2) //AF_INET
						{
							Net::SocketUtil::GetIPv4Name(sbuff, ReadMUInt32(&decBuff[i + 16]));
							sb->Append(sbuff);
						}
						else if (j == 23) //AF_INET6
						{
//							Net::SocketUtil::GetIPv4Name(sbuff, ReadUInt32(&decBuff[i + 16]));
//							sb->Append(sbuff);
						}
						else
						{
							sb->AppendHexBuff(&decBuff[i + 8], 16, 0, Text::LBT_NONE);
						}
						sb->Append((const UTF8Char*)", Sys ID = ");
						sb->AppendU16(ReadUInt16(&decBuff[i + 26]));
						sb->Append((const UTF8Char*)", SA Family = ");
						sb->AppendU32(ReadUInt32(&decBuff[i + 28]));
					}
					else if (recType == 8)
					{
						sb->Append((const UTF8Char*)" (ExporterStatRecord)");
						sb->Append((const UTF8Char*)", Stat Cnt = ");
						j = 8;
						l = ReadUInt32(&decBuff[i + 4]);
						k = 0;
						sb->AppendU32((UInt32)l);
						while (k < l)
						{
							sb->Append((const UTF8Char*)", Sys ID");
							sb->AppendUOSInt(k);
							sb->Append((const UTF8Char*)" = ");
							sb->AppendU32(ReadUInt32(&decBuff[i + j]));
							sb->Append((const UTF8Char*)", Seq Failure");
							sb->AppendUOSInt(k);
							sb->Append((const UTF8Char*)" = ");
							sb->AppendU32(ReadUInt32(&decBuff[i + j + 4]));
							sb->Append((const UTF8Char*)", Packets");
							sb->AppendUOSInt(k);
							sb->Append((const UTF8Char*)" = ");
							sb->AppendU64(ReadUInt64(&decBuff[i + j + 8]));
							sb->Append((const UTF8Char*)", Flows");
							sb->AppendUOSInt(k);
							sb->Append((const UTF8Char*)" = ");
							sb->AppendU64(ReadUInt64(&decBuff[i + j + 16]));
							j += 24;
							k++;
						}
					}
					else if (recType == 9)
					{
						sb->Append((const UTF8Char*)" (SamplerInfoRecord)");
						sb->Append((const UTF8Char*)", ID = ");
						sb->AppendI32(ReadInt32(&decBuff[i + 4]));
						sb->Append((const UTF8Char*)", Interval = ");
						sb->AppendU32(ReadUInt32(&decBuff[i + 8]));
						sb->Append((const UTF8Char*)", Mode = ");
						sb->AppendU16(ReadUInt16(&decBuff[i + 12]));
						sb->Append((const UTF8Char*)", Exporter Sys ID = ");
						sb->AppendU16(ReadUInt16(&decBuff[i + 14]));
					}
					else if (recType == 10)
					{
						sb->Append((const UTF8Char*)" (CommonRecord)");
					}
					i += recSize;
				}
			}
			MemFree(decBuff);
		}
		MemFree(packBuff);
	}
	return true;
}

Bool IO::FileAnalyse::NFDumpFileAnalyse::IsError()
{
	return this->fd == 0;
}

Bool IO::FileAnalyse::NFDumpFileAnalyse::IsParsing()
{
	return this->threadRunning;
}

Bool IO::FileAnalyse::NFDumpFileAnalyse::TrimPadding(const UTF8Char *outputFile)
{
	return false;
}
