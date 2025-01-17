#ifndef _SM_IO_CYCLICLOGBUFFER
#define _SM_IO_CYCLICLOGBUFFER
#include "Data/DateTime.h"
#include "IO/LogTool.h"
#include "Sync/Mutex.h"
#include "Text/StringBuilderUTF.h"

namespace IO
{
	class CyclicLogBuffer : public IO::ILogHandler
	{
	private:
		Sync::Mutex *logMut;
		UTF8Char **logBuff;
		UOSInt logInd;
		UOSInt buffSize;

	public:
		CyclicLogBuffer(UOSInt buffSize);
		virtual ~CyclicLogBuffer();

		virtual void LogAdded(Data::DateTime *logTime, const UTF8Char *logMsg, LogLevel logLev);
		virtual void LogClosed();

		void GetLogs(Text::StringBuilderUTF *sb, const UTF8Char* seperator);
	};
}
#endif
