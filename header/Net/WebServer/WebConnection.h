#ifndef _SM_NET_WEBSERVER_WEBCONNECTION
#define _SM_NET_WEBSERVER_WEBCONNECTION
#include "IO/FileStream.h"
#include "Net/SSLEngine.h"
#include "Net/WebServer/IWebHandler.h"
#include "Net/WebServer/WebListener.h"
#include "Net/WebServer/WebRequest.h"
#include "Text/StringBuilderUTF8.h"

namespace Net
{
	namespace WebServer
	{
		class WebConnection : public IWebResponse
		{
		public:
			typedef void (__stdcall *SendLogger)(void *userObj, UOSInt buffSize);
		private:
			Net::SocketFactory *sockf;
			Net::SSLEngine *ssl;
			Net::TCPClient *cli;
			Net::WebServer::WebListener *svr;
			Net::WebServer::IWebHandler *hdlr;
			UInt8 *dataBuff;
			UOSInt dataBuffSize;
			UOSInt buffSize;
			Net::WebServer::WebRequest *currReq;
			Bool allowKA;

			Bool allowProxy;
			Bool proxyMode;
			Net::TCPClient *proxyCli;

			Bool respHeaderSent;
			Bool respDataEnd;
			Net::WebStatus::StatusCode respStatus;
			Text::StringBuilderUTF8 *respHeaders;
			UInt64 respLeng;

			SendLogger logger;
			void *loggerObj;

		public:
			WebConnection(Net::SocketFactory *sockf, Net::SSLEngine *ssl, Net::TCPClient *cli, WebListener *svr, IWebHandler *hdlr, Bool allowProxy, Bool allowKA);
			virtual ~WebConnection();

			void ReceivedData(const UInt8 *buff, UOSInt size);
			void ProxyData(const UInt8 *buff, UOSInt size);
			void EndProxyConn();
			void ProxyShutdown();
			void ProcessTimeout();
			const UTF8Char *GetRequestURL();

		private:
			void SendHeaders(IWebRequest::RequestProtocol protocol);
			void ProcessResponse();

		public:
			virtual Bool SetStatusCode(Net::WebStatus::StatusCode code);
			virtual Int32 GetStatusCode();
			virtual Bool AddHeader(const UTF8Char *name, const UTF8Char *value);
			virtual Bool AddDefHeaders(Net::WebServer::IWebRequest *req);
			virtual UInt64 GetRespLength();
			virtual void ShutdownSend();

			virtual UOSInt Read(UInt8 *buff, UOSInt size);
			virtual UOSInt Write(const UInt8 *buff, UOSInt size);
			virtual Int32 Flush();
			virtual void Close();
			virtual Bool Recover();

			void SetSendLogger(SendLogger logger, void *userObj);
		};
	}
}
#endif
