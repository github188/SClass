#ifndef _SM_IO_MINIZZIP
#define _SM_IO_MINIZZIP
#include "Data/ArrayList.h"
#include "Text/Encoding.h"

namespace IO
{
	class MinizZIP
	{
	private:
		void *hand;
		Text::Encoding *enc;

	private:
		Bool AddDir(UTF8Char *zipPath, UTF8Char *dirPath);

	public:
		MinizZIP(const UTF8Char *zipFile);
		~MinizZIP();

		Bool AddFile(const UTF8Char *sourceFile);
		Bool AddFiles(Data::ArrayList<const UTF8Char *> *files);
		Bool AddContent(const UInt8 *content, UOSInt contLeng, const UTF8Char *fileName);
	};
}
#endif
