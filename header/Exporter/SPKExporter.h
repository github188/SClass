#ifndef _SM_EXPORTER_SPKEXPORTER
#define _SM_EXPORTER_SPKEXPORTER
#include "IO/FileExporter.h"
#include "IO/PackageFile.h"
#include "IO/SPackageFile.h"

namespace Exporter
{
	class SPKExporter : public IO::FileExporter
	{
	public:
		SPKExporter();
		virtual ~SPKExporter();

		virtual Int32 GetName();
		virtual SupportType IsObjectSupported(IO::ParsedObject *pobj);
		virtual Bool GetOutputName(OSInt index, UTF8Char *nameBuff, UTF8Char *fileNameBuff);
		virtual void SetCodePage(Int32 codePage);
		virtual Bool ExportFile(IO::SeekableStream *stm, const UTF8Char *fileName, IO::ParsedObject *pobj, void *param);

	private:
		void ExportPackageFile(IO::SPackageFile *spkg, IO::PackageFile *pkgFile, UTF8Char *buff, UTF8Char *buffEnd);
	};
};
#endif