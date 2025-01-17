#ifndef _SM_IO_FILEANALYSE_PCAPNGFILEANALYSE
#define _SM_IO_FILEANALYSE_PCAPNGFILEANALYSE
#include "Data/SyncArrayList.h"
#include "IO/IStreamData.h"
#include "IO/FileAnalyse/IFileAnalyse.h"
#include "Text/StringBuilderUTF.h"

namespace IO
{
	namespace FileAnalyse
	{
		class PCapngFileAnalyse : public IO::FileAnalyse::IFileAnalyse
		{
		private:
			typedef struct
			{
				UInt64 ofst;
				UInt32 blockType;
				UInt32 blockLength;
				UInt16 linkType;
				Int8 timeResol;
			} BlockInfo;
		private:
			IO::IStreamData *fd;
			Bool isBE;
			Data::SyncArrayList<BlockInfo *> *blockList;
			UInt8 *packetBuff;

			Bool pauseParsing;
			Bool threadRunning;
			Bool threadToStop;
			Bool threadStarted;

			static UInt32 __stdcall ParseThread(void *userObj);
		public:
			PCapngFileAnalyse(IO::IStreamData *fd);
			virtual ~PCapngFileAnalyse();

			virtual UOSInt GetFrameCount();
			virtual Bool GetFrameName(UOSInt index, Text::StringBuilderUTF *sb);
			virtual Bool GetFrameDetail(UOSInt index, Text::StringBuilderUTF *sb);

			virtual Bool IsError();
			virtual Bool IsParsing();
			virtual Bool TrimPadding(const UTF8Char *outputFile);

			static void SetTime(Data::DateTime *dt, Int64 ts, Int8 timeResol);
		};
	}
}
#endif
