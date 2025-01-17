#include "Stdafx.h"
#include "MyMemory.h"
#include "IO/ProtoHdlr/ProtoMapSvrHandler.h"

UInt16 IO::ProtoHdlr::ProtoMapSvrHandler::CalCheck(UInt8 *buff, Int32 sz)
{
#ifndef HAS_ASM32
	UInt8 ChkDigit1;
	UInt8 ChkDigit2;
	Int32 i;
	Int32 j;
	ChkDigit1 = 0;
	ChkDigit2 = 0;
	i = 0;
	while (i < sz)
	{
		j = ChkDigit1 + (Int32)buff[i];
		ChkDigit1 = j & 255;
		ChkDigit2 = (((((Int32)ChkDigit2) << 1) | (ChkDigit2 >> 7)) + (j >> 8)) ^ buff[i];
		i++;
	}
	return ChkDigit1 | (((Int32)ChkDigit2) << 8);
#else
	_asm
	{
		mov esi,buff
		mov edx,0
		mov ecx,sz
mcclop:
		lodsb
		rol dh,1
		add dl,al
		adc dh,0
		xor dh,al
		dec ecx
		jnz mcclop
		mov ax,dx
	}
#endif
}

IO::ProtoHdlr::ProtoMapSvrHandler::ProtoMapSvrHandler(IO::IProtocolHandler::DataListener *listener)
{
	this->listener = listener;
	NEW_CLASS(this->crcMut, Sync::Mutex());
	NEW_CLASS(this->crc, Crypto::Hash::CRC32R());
}

IO::ProtoHdlr::ProtoMapSvrHandler::~ProtoMapSvrHandler()
{
	DEL_CLASS(this->crc);
	DEL_CLASS(this->crcMut);
}

void *IO::ProtoHdlr::ProtoMapSvrHandler::CreateStreamData(IO::Stream *stm)
{
	return 0;
}

void IO::ProtoHdlr::ProtoMapSvrHandler::DeleteStreamData(IO::Stream *stm, void *stmData)
{
}

UOSInt IO::ProtoHdlr::ProtoMapSvrHandler::ParseProtocol(IO::Stream *stm, void *stmObj, void *stmData, UInt8 *buff, UOSInt buffSize)
{
	Bool found;
	UInt32 crcVal;
	while (buffSize >= 8)
	{
		found = false;
		if (*(Int16*)buff == *(Int16*)"Ma")
		{
			UInt32 packetSize = *(UInt16*)&buff[2];
			if (packetSize <= 2048)
			{
				if (packetSize > buffSize)
					return buffSize;

				crcVal = CalCheck(buff, packetSize - 2);
				if ((crcVal & 0xffff) == *(UInt16*)&buff[packetSize - 2])
				{
					this->listener->DataParsed(stm, stmObj, *(UInt16*)&buff[4], 0, &buff[6], packetSize - 8);

					found = true;
					buff += packetSize;
					buffSize -= packetSize;
				}
			}
		}
		else if (*(Int16*)buff == *(Int16*)"ma")
		{
			UInt32 packetSize = *(UInt16*)&buff[2];
			if (packetSize <= 2048)
			{
				if (packetSize > buffSize)
					return buffSize;

				this->crcMut->Lock();
				this->crc->Clear();
				this->crc->Calc(buff, packetSize - 2);
				this->crc->GetValue((UInt8*)&crcVal);
				this->crcMut->Unlock();
				if ((crcVal & 0xffff) == *(UInt16*)&buff[packetSize - 2])
				{
					this->listener->DataParsed(stm, stmObj, 0x10000 | *(UInt16*)&buff[4], 0, &buff[6], packetSize - 8);

					found = true;
					buff += packetSize;
					buffSize -= packetSize;
				}
			}
		}

		if (!found)
		{
			buff++;
			buffSize--;
		}
	}
	return buffSize;
}

UOSInt IO::ProtoHdlr::ProtoMapSvrHandler::BuildPacket(UInt8 *buff, Int32 cmdType, Int32 seqId, UInt8 *cmd, UOSInt cmdSize, void *stmData)
{
	*(Int16*)&buff[2] = (Int16)(cmdSize + 8);
	*(Int16*)&buff[4] = cmdType;
	if (cmdSize > 0)
	{
		MemCopyNO(&buff[6], cmd, cmdSize);
	}

	if (cmdType & 0x10000)
	{
		UInt32 crcVal;
		*(Int16*)buff = *(Int16*)"ma";

		this->crcMut->Lock();
		this->crc->Clear();
		this->crc->Calc(buff, cmdSize + 6);
		this->crc->GetValue((UInt8*)&crcVal);
		this->crcMut->Unlock();
		*(UInt16*)&buff[cmdSize + 6] = (UInt16)crcVal;
	}
	else
	{
		*(Int16*)buff = *(Int16*)"Ma";
		*(UInt16*)&buff[cmdSize + 6] = CalCheck(buff, (Int32)cmdSize + 6);
	}
	return cmdSize + 8;
}
