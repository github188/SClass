#include "Stdafx.h"
#include "Core/Core.h"
#include "IO/ConsoleWriter.h"
#include "Net/OSSocketFactory.h"
#include "Net/WebServer/HTTPDirectoryHandler.h"
#include "Net/WebServer/HTTPFormParser.h"
#include "Net/WebServer/WebListener.h"
#include "Net/WebServer/WebStandardHandler.h"
#include "Text/StringBuilderUTF8.h"
#include "Text/XML.h"

IO::ConsoleWriter *console;

Int32 MyMain(Core::IProgControl *progCtrl)
{
	Net::WebServer::WebListener *svr;
	Net::SocketFactory *sockf;
	Net::WebServer::WebStandardHandler *hdlr;
	Text::StringBuilderUTF8 sb;
	IO::ConsoleWriter *console;
	Int32 port = 80;
	NEW_CLASS(console, IO::ConsoleWriter());

	OSInt argc;
	UTF8Char **argv = progCtrl->GetCommandLines(progCtrl, &argc);
	if (argc >= 2)
	{
		port = Text::StrToInt32(argv[1]);
		if (port <= 0)
		{
			port = 0;
		}
	}

	sb.ClearStr();
	sb.Append((const UTF8Char*)"Listening to port ");
	sb.AppendI32(port);
	console->WriteLine(sb.ToString());
	NEW_CLASS(sockf, Net::OSSocketFactory(true));
	NEW_CLASS(hdlr, Net::WebServer::HTTPDirectoryHandler((const UTF8Char*)".", true, 0, true));
//	NEW_CLASS(svr, Net::WebServer::WebListener(sockf, hdlr, port, 120, 8, (const UTF8Char*)"sswr", false, true));
	NEW_CLASS(svr, Net::WebServer::WebListener(sockf, hdlr, port, 120, 1, (const UTF8Char*)"sswr", false, true));
	if (!svr->IsError())
	{
		progCtrl->WaitForExit(progCtrl);
	}
	else
	{
		console->WriteLine((const UTF8Char*)"Error in listening port");
	}
	DEL_CLASS(svr);
	hdlr->Release();
	DEL_CLASS(sockf);
	DEL_CLASS(console);
	return 0;
}