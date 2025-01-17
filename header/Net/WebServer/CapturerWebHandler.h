#ifndef _SM_NET_WEBSERVER_CAPTURERWEBHANDLER
#define _SM_NET_WEBSERVER_CAPTURERWEBHANDLER
#include "IO/BTCapturer.h"
#include "IO/RadioSignalLogger.h"
#include "Net/WiFiCapturer.h"
#include "Net/WebServer/WebServiceHandler.h"
#include "Text/StringBuilderUTF.h"

namespace Net
{
	namespace WebServer
	{
		class CapturerWebHandler : public Net::WebServer::WebServiceHandler
		{
		private:
			IO::BTCapturer *btCapture;
			Net::WiFiCapturer *wifiCapture;
			IO::RadioSignalLogger *radioLogger;

			static Bool __stdcall IndexFunc(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, WebServiceHandler *svc);
			static Bool __stdcall BTCurrentFunc(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, WebServiceHandler *svc);
			static Bool __stdcall BTDetailFunc(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, WebServiceHandler *svc);
			static Bool __stdcall WiFiCurrentFunc(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, WebServiceHandler *svc);
			static Bool __stdcall WiFiDetailFunc(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, WebServiceHandler *svc);
			static Bool __stdcall WiFiDownloadFunc(Net::WebServer::IWebRequest *req, Net::WebServer::IWebResponse *resp, const UTF8Char *subReq, WebServiceHandler *svc);

			static void AppendWiFiTable(Text::StringBuilderUTF *sb, Net::WebServer::IWebRequest *req, Data::ArrayList<Net::WiFiLogFile::LogFileEntry*> *entryList, Int64 scanTime);
			static void AppendBTTable(Text::StringBuilderUTF *sb, Net::WebServer::IWebRequest *req, Data::ArrayList<IO::BTScanner::ScanRecord2*> *entryList, Bool inRangeOnly);
			static OSInt __stdcall WiFiLogRSSICompare(void *obj1, void *obj2);
			static OSInt __stdcall BTLogRSSICompare(void *obj1, void *obj2);
		public:
			CapturerWebHandler(Net::WiFiCapturer *wifiCapture, IO::BTCapturer *btCapture, IO::RadioSignalLogger *radioLogger);
			virtual ~CapturerWebHandler();
		};
	}
}
#endif
