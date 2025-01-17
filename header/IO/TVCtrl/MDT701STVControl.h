#ifndef _SM_IO_TVCTRL_MDT701STVCONTROL
#define _SM_IO_TVCTRL_MDT701STVCONTROL
#include "Data/DateTime.h"
#include "IO/Stream.h"
#include "IO/TVControl.h"
#include "Sync/Mutex.h"

namespace IO
{
	namespace TVCtrl
	{
		class MDT701STVControl : public IO::TVControl
		{
		private:
			IO::Stream *stm;
			Int32 monId;
			Data::DateTime *nextTime;

			Sync::Mutex *mut;
			UInt8 *recvBuff;
			UOSInt recvSize;
			Bool recvRunning;
			Bool recvToStop;
			Sync::Event *recvEvt;

			static UInt32 __stdcall RecvThread(void *userObj);
			Bool SendBasicCommand(const Char *buff, UOSInt buffSize, const Char *cmdReply, UOSInt replySize, UInt32 cmdTimeout, UInt32 cmdInterval);
		public:
			MDT701STVControl(IO::Stream *stm, Int32 monId);
			virtual ~MDT701STVControl();

			virtual Bool SendInstruction(CommandType ct);
			virtual Bool SendGetCommand(CommandType ct, Int32 *val, UTF8Char *sbuff);
			virtual Bool SendSetCommand(CommandType ct, Int32 val);
			virtual void GetSupportedCmd(Data::ArrayList<CommandType> *cmdList, Data::ArrayList<CommandFormat> *cmdFormats);
			static Bool GetInfo(IO::TVControl::TVInfo *info);
		};
	}
}
#endif
