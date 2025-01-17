#ifndef _SM_IO_IPROTOCOLHANDLER
#define _SM_IO_IPROTOCOLHANDLER
#include "IO/IProtocolParser.h"

namespace IO
{
	class IProtocolHandler : public IProtocolParser
	{
	public:
		class DataListener
		{
		public:
			virtual void DataParsed(IO::Stream *stm, void *stmObj, Int32 cmdType, Int32 seqId, const UInt8 *cmd, UOSInt cmdSize) = 0;
			virtual void DataSkipped(IO::Stream *stm, void *stmObj, const UInt8 *buff, UOSInt buffSize) = 0;
		};

	public:
		virtual ~IProtocolHandler(){};
		virtual UOSInt BuildPacket(UInt8 *buff, Int32 cmdType, Int32 seqId, const UInt8 *cmd, UOSInt cmdSize, void *stmData) = 0;
	};
}
#endif
