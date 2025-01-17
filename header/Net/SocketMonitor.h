#ifndef _SM_NET_SOCKETMONITOR
#define _SM_NET_SOCKETMONITOR
#include "Data/DateTime.h"
#include "Sync/Event.h"
#include "Net/SocketFactory.h"

namespace Net
{
	class SocketMonitor
	{
	public:
		typedef void (__stdcall *RAWDataHdlr)(void *userData, const UInt8 *packetData, UOSInt packetSize);

		typedef struct
		{
			SocketMonitor *me;
			Bool threadRunning;
			Sync::Event *evt;
			Bool toStop;
		} ThreadStat;

	private:
		Net::SocketFactory *sockf;
		UInt32 *soc;
		RAWDataHdlr hdlr;
		void *userData;

		ThreadStat *threadStats;
		UOSInt threadCnt;
		UInt16 port;
		Sync::Event *ctrlEvt;

	private:
		static UInt32 __stdcall DataThread(void *obj);

	public:
		SocketMonitor(Net::SocketFactory *sockf, UInt32 *soc, RAWDataHdlr hdlr, void *userData, UOSInt workerCnt);
		~SocketMonitor();
	};
};
#endif
