#ifndef _SM_MANAGE_EXCEPTIONLOGGER
#define _SM_MANAGE_EXCEPTIONLOGGER
#include "Manage/AddressResolver.h"
#include "IO/IWriter.h"
#include "IO/Stream.h"
#include "Manage/StackTracer.h"

namespace Manage
{
	class ExceptionLogger
	{
	private:
		static void WriteContext(IO::IWriter *writer, IO::Stream *stm, Manage::ThreadContext *context, Manage::AddressResolver *addrResol);
		static void WriteStackTrace(IO::IWriter *writer, Manage::StackTracer *tracer, Manage::AddressResolver *addrResol);
	public:
		static Bool LogToFile(const UTF8Char *fileName, UInt32 exCode, const UTF8Char *exName, OSInt exAddr, Manage::ThreadContext *context);
	};
}

#endif