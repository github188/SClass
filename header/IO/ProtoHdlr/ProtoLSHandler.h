#ifndef _SM_IO_PROTOHDLR_PROTOLSHANDLER
#define _SM_IO_PROTOHDLR_PROTOLSHANDLER
#include "IO/IProtocolHandler.h"
#include "Sync/Mutex.h"

namespace IO
{
	namespace ProtoHdlr
	{
		class ProtoLSHandler : public IO::IProtocolHandler
		{
		private:
			IO::IProtocolHandler::DataListener *listener;

		public:
			ProtoLSHandler(IO::IProtocolHandler::DataListener *listener);
			virtual ~ProtoLSHandler();

			virtual void *CreateStreamData(IO::Stream *stm);
			virtual void DeleteStreamData(IO::Stream *stm, void *stmData);
			virtual UOSInt ParseProtocol(IO::Stream *stm, void *stmObj, void *stmData, UInt8 *buff, UOSInt buffSize); // return unprocessed size
			virtual UOSInt BuildPacket(UInt8 *buff, Int32 cmdType, Int32 seqId, UInt8 *cmd, UOSInt cmdSize, void *stmData);

			UInt16 CalCheck(UInt8 *buff, UOSInt buffSize);
		};
	}
}
#endif
