#ifndef _SM_IO_PROTOHDLR_PROTOCORTEXHANDLER
#define _SM_IO_PROTOHDLR_PROTOCORTEXHANDLER
#include "IO/IProtocolHandler.h"

namespace IO
{
	namespace ProtoHdlr
	{
		class ProtoCortexHandler : public IO::IProtocolHandler
		{
		private:
			IO::IProtocolHandler::DataListener *listener;

		public:
			ProtoCortexHandler(IO::IProtocolHandler::DataListener *listener);
			virtual ~ProtoCortexHandler();

			virtual void *CreateStreamData(IO::Stream *stm);
			virtual void DeleteStreamData(IO::Stream *stm, void *stmData);
			virtual UOSInt ParseProtocol(IO::Stream *stm, void *stmObj, void *stmData, UInt8 *buff, UOSInt buffSize); // return unprocessed size
			virtual UOSInt BuildPacket(UInt8 *buff, Int32 cmdType, Int32 seqId, UInt8 *cmd, UOSInt cmdSize, void *stmData);
		};
	}
}
#endif
