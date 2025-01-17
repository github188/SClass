#ifndef _SM_MANAGE_MONCONN
#define _SM_MANAGE_MONCONN
#include "Net/SocketFactory.h"
#include "Net/TCPClient.h"
#include "Data/SyncArrayList.h"
#include "IO/ConsoleWriter.h"
#include "Data/DateTime.h"

namespace Manage
{
	typedef enum
	{
		MON_EVT_STOP_PROCESS,
		MON_EVT_PROCESS_START_ERR
	} MonEvent;
	typedef void (__stdcall *EventHandler)(MonEvent evt, Int32 eventStatus, void *userObj);
	class MonConn
	{
	public:
		static UOSInt BuildPacket(UInt8 *outbuff, UInt8 *data, UOSInt dataSize, UInt16 cmdType, UInt16 cmdSeq);
		static UInt16 CalCheck(UInt8 *packet);
		static UInt8 *FindPacket(UInt8 *buff, UOSInt buffSize);
		static Bool IsCompletePacket(UInt8 *buff, UOSInt buffSize);
		static void ParsePacket(UInt8 *buff, UInt16 *cmdSize, UInt16 *cmdType, UInt16 *cmdSeq, UInt8** cmdData);

	private:
		Data::SyncArrayList<UInt8*> *cmdList;
		EventHandler hdlr;
		Bool ConnRRunning;
		Bool ConnTRunning;
		Bool ToStop;
		Sync::Mutex *cmdSeqMut;
		UInt16 cmdSeq;
		UInt16 port;
		Int32 svrMonConn;

		Net::SocketFactory *sockf;
		Net::TCPClient *cli;
		Bool cliErr;
		Sync::Event *connREvt;
		Sync::Event *connTEvt;
		Bool requesting;
		Data::DateTime *lastReqTime;
		Data::DateTime *lastKATime;
		void *userObj;
		IO::Writer *msgWriter;

		static UInt32 __stdcall ConnTThread(void *conn);
		static UInt32 __stdcall ConnRThread(void *conn);
		void AddCommand(UInt8 *data, UOSInt dataSize, UInt16 cmdType);
	public:
		MonConn(EventHandler hdlr, void *userObj, Net::SocketFactory *sockf, IO::Writer *msgWriter);
		~MonConn();

		Bool IsError();

		void StartProcess(Int32 name);
		void EndProcess();
		void StartTCPPort(UInt16 portNum);
		void StartUDPPort(UInt16 portNum);
		void AddLogMessage(Int32 name, Int32 name2, UInt16 logLevel, const UTF8Char *msg);
	};
}
#endif
