#ifndef _SM_EXPORTER_SHPEXPORTER
#define _SM_EXPORTER_SHPEXPORTER
#include "IO/FileExporter.h"

namespace Exporter
{
	class SHPExporter : public IO::FileExporter
	{
	private:
		Int32 codePage;
	public:
		SHPExporter();
		virtual ~SHPExporter();

		virtual Int32 GetName();
		virtual SupportType IsObjectSupported(IO::ParsedObject *pobj);
		virtual Bool GetOutputName(OSInt index, UTF8Char *nameBuff, UTF8Char *fileNameBuff);
		virtual void SetCodePage(Int32 codePage);
		virtual Bool ExportFile(IO::SeekableStream *stm, const UTF8Char *fileName, IO::ParsedObject *pobj, void *param);
	};
};
#endif