#ifndef _SM_EXPORTER_EXCELXMLEXPORTER
#define _SM_EXPORTER_EXCELXMLEXPORTER
#include "IO/FileExporter.h"
#include "IO/IWriter.h"
#include "Text/SpreadSheet/CellStyle.h"

namespace Exporter
{
	class ExcelXMLExporter : public IO::FileExporter
	{
	private:
		Int32 codePage;
	public:
		ExcelXMLExporter();
		virtual ~ExcelXMLExporter();

		virtual Int32 GetName();
		virtual SupportType IsObjectSupported(IO::ParsedObject *pobj);
		virtual Bool GetOutputName(OSInt index, UTF8Char *nameBuff, UTF8Char *fileNameBuff);
		virtual void SetCodePage(Int32 codePage);
		virtual Bool ExportFile(IO::SeekableStream *stm, const UTF8Char *fileName, IO::ParsedObject *pobj, void *param);

	private:
		static void WriteBorderStyle(IO::IWriter *writer, const UTF8Char *position, Text::SpreadSheet::CellStyle::BorderStyle *border);
	};
};
#endif