#ifndef _SM_NET_TCPSERVERSTREAM
#define _SM_NET_TCPSERVERSTREAM
#include "IO/LogTool.h"
#include "IO/Stream.h"
#include "Net/TCPClient.h"
#include "Net/TCPServer.h"

namespace Net
{
	class TCPServerStream : public IO::Stream
	{
	public:
		Net::SocketFactory *sockf;
		Net::TCPServer *svr;
		IO::LogTool *log;
		Sync::Mutex *readMut;
		Sync::Mutex *connMut;
		Sync::Event *readEvt;
		Net::TCPClient *currCli;

		static void __stdcall ConnHandler(UInt32 *s, void *userObj);
	public:
		TCPServerStream(Net::SocketFactory *sockf, UInt16 port, IO::LogTool *log);
		~TCPServerStream();

		virtual UOSInt Read(UInt8 *buff, UOSInt size);
		virtual UOSInt Write(const UInt8 *buff, UOSInt size);

		virtual Int32 Flush();
		virtual void Close();
		virtual Bool Recover();

		Bool IsError();
	};
}
#endif
