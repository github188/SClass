#include "Stdafx.h"
#include "Exporter/GIFExporter.h"
#include "Exporter/GUIGIFExporter.h"
#include "Text/MyString.h"

Exporter::GUIGIFExporter::GUIGIFExporter() : Exporter::GUIExporter()
{
}

Exporter::GUIGIFExporter::~GUIGIFExporter()
{
}

Int32 Exporter::GUIGIFExporter::GetName()
{
	return *(Int32*)"GPGI";
}

Bool Exporter::GUIGIFExporter::GetOutputName(UOSInt index, UTF8Char *nameBuff, UTF8Char *fileNameBuff)
{
	if (index == 0)
	{
		Text::StrConcat(nameBuff, (const UTF8Char*)"GIF file");
		Text::StrConcat(fileNameBuff, (const UTF8Char*)"*.gif");
		return true;
	}
	return false;
}

Bool Exporter::GUIGIFExporter::ExportFile(IO::SeekableStream *stm, const UTF8Char *fileName, IO::ParsedObject *pobj, void *param)
{
	Exporter::GIFExporter exporter;
	return exporter.ExportFile(stm, fileName, pobj, 0);
}
