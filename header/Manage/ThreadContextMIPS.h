#ifndef _SM_MANAGE_THREADCONTEXTMIPS
#define _SM_MANAGE_THREADCONTEXTMIPS
#include "Manage/ThreadContext.h"

namespace Manage
{
	class ThreadContextMIPS : public ThreadContext
	{
	private:
		void *context;
		UOSInt procId;
		UOSInt threadId;
	public:
		ThreadContextMIPS(UOSInt procId, UOSInt threadId, void *context);
		virtual ~ThreadContextMIPS();

		virtual OSInt GetRegisterCnt();
		virtual UTF8Char *GetRegister(OSInt index, UTF8Char *buff, UInt8 *regVal, Int32 *regBitCount);
		virtual void ToString(Text::StringBuilderUTF *sb);
		virtual ContextType GetType();
		virtual UOSInt GetThreadId();
		virtual UOSInt GetProcessId();
		virtual OSInt GetInstAddr();
		virtual OSInt GetStackAddr();
		virtual OSInt GetFrameAddr();
		virtual void SetInstAddr(OSInt instAddr);
		virtual void SetStackAddr(OSInt stackAddr);
		virtual void SetFrameAddr(OSInt frameAddr);
		virtual ThreadContext *Clone();
		virtual Bool GetRegs(Manage::Dasm::Dasm_Regs *regs);
		virtual Manage::Dasm *CreateDasm();
		void *GetContext();
	};
};
#endif