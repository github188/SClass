#ifndef _SM_EXPORTER_SHA1EXPORTER
#define _SM_EXPORTER_SHA1EXPORTER
#include "IO/FileExporter.h"

namespace Exporter
{
	class SHA1Exporter : public IO::FileExporter
	{
	private:
		Int32 codePage;
	public:
		SHA1Exporter();
		virtual ~SHA1Exporter();

		virtual Int32 GetName();
		virtual SupportType IsObjectSupported(IO::ParsedObject *pobj);
		virtual Bool GetOutputName(OSInt index, UTF8Char *nameBuff, UTF8Char *fileNameBuff);
		virtual void SetCodePage(Int32 codePage);
		virtual Bool ExportFile(IO::SeekableStream *stm, const UTF8Char *fileName, IO::ParsedObject *pobj, void *param);
	};
};
#endif