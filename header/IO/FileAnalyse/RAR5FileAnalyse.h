#ifndef _SM_IO_FILEANALYSE_RAR5FILEANALYSE
#define _SM_IO_FILEANALYSE_RAR5FILEANALYSE
#include "Data/ArrayList.h"
#include "IO/IStreamData.h"
#include "IO/FileAnalyse/IFileAnalyse.h"
#include "Sync/Mutex.h"
#include "Text/StringBuilderUTF.h"

namespace IO
{
	namespace FileAnalyse
	{
		class RAR5FileAnalyse : public IO::FileAnalyse::IFileAnalyse
		{
		private:
			typedef struct
			{
				Int64 fileOfst;
				UInt32 headerSize;
				UInt32 headerType;
				Int64 dataSize;
			} BlockInfo;
		private:
			IO::IStreamData *fd;
			Data::ArrayList<BlockInfo*> *packs;
			Sync::Mutex *packMut;

			Bool pauseParsing;
			Bool threadRunning;
			Bool threadToStop;
			Bool threadStarted;

			static const UInt8 *ReadVInt(const UInt8 *buffPtr, UInt64 *val);
			static UInt32 __stdcall ParseThread(void *userObj);
		public:
			RAR5FileAnalyse(IO::IStreamData *fd);
			virtual ~RAR5FileAnalyse();

			virtual UOSInt GetFrameCount();
			virtual Bool GetFrameName(UOSInt index, Text::StringBuilderUTF *sb);
			virtual Bool GetFrameDetail(UOSInt index, Text::StringBuilderUTF *sb);

			virtual Bool IsError();
			virtual Bool IsParsing();
			virtual Bool TrimPadding(const UTF8Char *outputFile);
		};
	}
}
#endif