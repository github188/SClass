#ifndef _SM_IO_FILEANALYSE_MPEGFILEANALYSE
#define _SM_IO_FILEANALYSE_MPEGFILEANALYSE
#include "Data/ArrayList.h"
#include "IO/IStreamData.h"
#include "IO/FileAnalyse/IFileAnalyse.h"
#include "Sync/Mutex.h"
#include "Text/StringBuilder.h"

namespace IO
{
	namespace FileAnalyse
	{
		class MPEGFileAnalyse : public IO::FileAnalyse::IFileAnalyse
		{
		private:
			typedef struct
			{
				Int64 fileOfst;
				OSInt packSize;
				UInt8 packType;
			} PackInfo;
		private:
			IO::IStreamData *fd;
			Data::ArrayList<PackInfo*> *packs;
			Sync::Mutex *packMut;
			Int32 mpgVer;

			Bool pauseParsing;
			Bool threadRunning;
			Bool threadToStop;
			Bool threadStarted;
			static UInt32 __stdcall ParseThread(void *userObj);
		public:
			MPEGFileAnalyse(IO::IStreamData *fd);
			virtual ~MPEGFileAnalyse();

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