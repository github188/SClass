#ifndef _SM_EXPORTER_MAPCSVEXPORTER
#define _SM_EXPORTER_MAPCSVEXPORTER
#include "IO/FileExporter.h"

namespace Exporter
{
	class MapCSVExporter : public IO::FileExporter
	{
	private:
		Int32 codePage;
	public:
		MapCSVExporter();
		virtual ~MapCSVExporter();

		virtual Int32 GetName();
		virtual SupportType IsObjectSupported(IO::ParsedObject *pobj);
		virtual Bool GetOutputName(OSInt index, UTF8Char *nameBuff, UTF8Char *fileNameBuff);
		virtual void SetCodePage(Int32 codePage);
		virtual Bool ExportFile(IO::SeekableStream *stm, const UTF8Char *fileName, IO::ParsedObject *pobj, void *param);
	};
};
#endif