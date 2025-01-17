#ifndef _SM_IO_ATCOMMANDCHANNEL
#define _SM_IO_ATCOMMANDCHANNEL
#include "Data/ArrayList.h"
#include "IO/LogTool.h"
#include "IO/Stream.h"
#include "Sync/Event.h"
#include "Sync/Mutex.h"
#include "Sync/MutexUsage.h"

namespace IO
{
	class ATCommandChannel
	{
	public:
		typedef void (__stdcall *EventHandler)(void *userObj, UInt8 eventType);
		typedef Bool (__stdcall *CommandHandler)(void *userObj, const Char *cmd);

	private:
		IO::Stream *stm;
		Bool stmRelease;
		Sync::Mutex *cmdMut;
		Sync::Event *cmdEvt;
		EventHandler evtHdlr;
		void *evtHdlrObj;
		CommandHandler cmdHdlr;
		void *cmdHdlrObj;
		IO::ILogger *log;

		Data::ArrayList<const Char *> *cmdResults;
		Sync::Mutex *cmdResultMut;

		Bool threadRunning;
		Bool threadToStop;
	private:
		static UInt32 __stdcall CmdThread(void *userObj);
		void ClearResults();
	public:
		ATCommandChannel(IO::Stream *stm, Bool needRelease);
		~ATCommandChannel();

		IO::Stream *GetStream();

		UOSInt SendATCommand(Data::ArrayList<const Char *> *retArr, const Char *atCmd, Int32 timeoutMS);
		UOSInt SendATCommands(Data::ArrayList<const Char *> *retArr, const Char *atCmd, const Char *atCmdSub, Int32 timeoutMS);
		UOSInt SendDialCommand(Data::ArrayList<const Char *> *retArr, const Char *atCmd, Int32 timeoutMS);

		Bool UseCmd(Sync::MutexUsage *mutUsage);
		UOSInt CmdSend(const UInt8 *data, UOSInt dataSize);
		const Char *CmdGetNextResult(UOSInt timeoutMS);

		void Close();

		void SetEventHandler(EventHandler evtHdlr, void *userObj);
		void SetCommandHandler(CommandHandler cmdHdlr, void *userObj);
		void SetLogger(IO::ILogger *log);
	};
}
#endif
