#ifndef _SM_IO_FILELOG
#define _SM_IO_FILELOG
#include "IO/FileStream.h"
#include "IO/LogTool.h"
#include "Sync/Mutex.h"
#include "Text/UTF8Writer.h"

namespace IO
{
	class FileLog : public ILogHandler
	{
	private:
		Sync::Mutex *mut;
		IO::ILogHandler::LogType logStyle;
		IO::ILogHandler::LogGroup groupStyle;
		Int32 lastVal;
		Text::UTF8Writer *log;
		IO::FileStream *fileStm;
		const Char *dateFormat;
		const UTF8Char *fileName;
		const UTF8Char *extName;
		Bool closed;

		UTF8Char *GetNewName(UTF8Char *buff, Data::DateTime *logTime);
	public:
		FileLog(const UTF8Char *fileName, LogType style, LogGroup groupStyle, const Char *dateFormat);
		virtual ~FileLog();
		virtual void LogClosed();
		virtual void LogAdded(Data::DateTime *logTime, const UTF8Char *logMsg, LogLevel logLev);
	};
}
#endif
