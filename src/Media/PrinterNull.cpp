#include "Stdafx.h"
#include "MyMemory.h"
#include "Media/Printer.h"

OSInt Media::Printer::GetPrinterCount()
{
	return 0;
}

UTF8Char *Media::Printer::GetPrinterName(UTF8Char *sbuff, OSInt index)
{
	return 0;
}

Media::Printer *Media::Printer::SelectPrinter(void *hWnd)
{
	return 0;
}

Media::Printer::Printer(const UTF8Char *printerName)
{
}

Media::Printer::~Printer()
{
}

Bool Media::Printer::IsError()
{
	return true;
}

Bool Media::Printer::ShowPrintSettings(void *hWnd)
{
	return false;
}

Media::IPrintDocument *Media::Printer::StartPrint(IPrintHandler *hdlr, Media::DrawEngine *eng)
{
	return 0;
}

void Media::Printer::EndPrint(IPrintDocument *doc)
{
}
