#include "Stdafx.h"
#include "MyMemory.h"
#include "IO/ProtoHdlr/ProtoCortexHandler.h"

IO::ProtoHdlr::ProtoCortexHandler::ProtoCortexHandler(IO::IProtocolHandler::DataListener *listener)
{
	this->listener = listener;
}

IO::ProtoHdlr::ProtoCortexHandler::~ProtoCortexHandler()
{
}

void *IO::ProtoHdlr::ProtoCortexHandler::CreateStreamData(IO::Stream *stm)
{
	return 0;
}

void IO::ProtoHdlr::ProtoCortexHandler::DeleteStreamData(IO::Stream *stm, void *stmData)
{
}

OSInt IO::ProtoHdlr::ProtoCortexHandler::ParseProtocol(IO::Stream *stm, void *stmObj, void *stmData, UInt8 *buff, OSInt buffSize)
{
	OSInt i;
	OSInt j;
	OSInt k;
	UInt8 cmdSize;
	UInt8 chk;
	i = 0;
	j = 0;
	while (i < buffSize)
	{
		if (buff[i] == 0xff)
		{
			if (i != j)
			{
				this->listener->DataSkipped(stm, stmObj, &buff[j], i - j);
				j = i;
			}
			if (i + 4 > buffSize)
				return buffSize - i;
			cmdSize = buff[i + 2];
			if (i + 4 + cmdSize > buffSize)
				return buffSize - i;
			k = i + 3 + cmdSize;
			chk = 0;
			while (k-- > i)
			{
				chk = (UInt8)((chk + buff[k]) & 0xff);
			}
			if (chk == buff[i + cmdSize + 3])
			{
				this->listener->DataParsed(stm, stmObj, buff[i + 1], 0, &buff[i + 3], cmdSize);

				i += cmdSize + 4;
				j = i;
			}
			else
			{
			}
		}
		i++;
	}
	if (i != j)
	{
		this->listener->DataSkipped(stm, stmObj, &buff[j], i - j);
	}
	return 0;
}

OSInt IO::ProtoHdlr::ProtoCortexHandler::BuildPacket(UInt8 *buff, Int32 cmdType, Int32 seqId, UInt8 *cmd, OSInt cmdSize, void *stmData)
{
	UInt8 chk;
	OSInt i;
	buff[0] = 0xff;
	buff[1] = (UInt8)(cmdType & 0xff);
	buff[2] = (UInt8)(cmdSize & 0xff);
	if (cmdSize > 0)
	{
		MemCopyNO(&buff[3], cmd, cmdSize);
	}
	i = 3 + cmdSize;
	chk = 0;
	while (i-- > 0)
	{
		chk = (UInt8)((chk + buff[i]) & 0xff);
	}
	buff[cmdSize + 3] = chk;
	return cmdSize + 4;
}
