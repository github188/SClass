#ifndef _SM_NET_RTSPCLIENT
#define _SM_NET_RTSPCLIENT
#include "Net/IRTPController.h"
#include "Net/RTPCliChannel.h"
#include "Net/SDPFile.h"
#include "Net/TCPClient.h"
#include "Sync/Mutex.h"

namespace Net
{
	class RTSPClient : public IRTPController
	{
	private:
		typedef struct
		{
			Int32 useCnt;

			Net::SocketFactory *sockf;
			Net::TCPClient *cli;
			Sync::Mutex *cliMut;
			const UTF8Char *host;
			UInt16 port;
			Int32 nextSeq;

			Sync::Event *reqEvt;
			Sync::Mutex *reqMut;
			UInt8 *reqReply;
			UOSInt reqReplySize;
			Int32 reqReplyStatus;
			Bool reqSuccess;
			const UTF8Char *reqStrs;

			Bool threadRunning;
			Bool threadToStop;
		} ClientData;
	private:
		ClientData *cliData;

		static UInt32 __stdcall ControlThread(void *userObj);
		Int32 NextRequest();
		Bool WaitForReply();
		Bool SendData(UInt8 *buff, UOSInt buffSize);

		RTSPClient(RTSPClient *cli);
	public:
		RTSPClient(Net::SocketFactory *sockf, const UTF8Char *host, UInt16 port);
		~RTSPClient();


		Bool GetOptions(const UTF8Char *url, Data::ArrayList<const UTF8Char *> *options);
		Net::SDPFile *GetMediaInfo(const UTF8Char *url);

		static IO::ParsedObject *ParseURL(Net::SocketFactory *sockf, const UTF8Char *url);

	private:
		UTF8Char *SetupRTP(UTF8Char *sessIdOut, const UTF8Char *url, Net::RTPCliChannel *rtpChannel);

		Bool Play(const UTF8Char *url, const UTF8Char *sessId);
		Bool Close(const UTF8Char *url, const UTF8Char *sessId);

	public:
		virtual Bool Init(Net::RTPCliChannel *rtpChannel);
		virtual Bool Play(Net::RTPCliChannel *rtpChannel);
		virtual Bool KeepAlive(Net::RTPCliChannel *rtpChannel);
		virtual Bool StopPlay(Net::RTPCliChannel *rtpChannel);
		virtual Bool Deinit(Net::RTPCliChannel *rtpChannel);

		virtual Net::IRTPController *Clone();
	};
};
#endif
