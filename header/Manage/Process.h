// link with Psapi.lib
#ifndef _SM_MANAGE_PROCESS
#define _SM_MANAGE_PROCESS
#include "Data/ArrayList.h"
#include "Manage/IMemoryReader.h"
#include "Manage/ModuleInfo.h"
#include "Manage/ThreadInfo.h"
#include "Text/StringBuilderUTF.h"

namespace Manage
{
	class Process : public Manage::IMemoryReader
	{
	public:
		typedef enum
		{
			PP_REALTIME = 0x00000100,
			PP_HIGH = 0x00000080,
			PP_ABOVE_NORMAL = 0x00008000,
			PP_NORMAL = 0x00000020,
			PP_BELOW_NORMAL = 0x00004000,
			PP_IDLE = 0x00000040,
			PP_UNKNOWN = 0
		} ProcessPriority;

		typedef enum
		{
			HT_UNKNOWN,
			HT_FIXED,
			HT_MOVABLE,
			HT_FREE
		} HeapType;

		typedef struct
		{
			OSInt startAddr;
			OSInt size;
			HeapType heapType;
		} HeapInfo;

	private:
		UOSInt procId;
		void *handle;
		Bool needRelease;

	public:
		Process(UOSInt procId, Bool controlRight);
		Process();
		Process(const UTF8Char *cmdLine);
		Process(const WChar *cmdLine);
		virtual ~Process();

		static UOSInt GetCurrProcId();

		UOSInt GetProcId();
		Bool IsRunning();
		Bool Kill();
		WChar *GetFilename(WChar *buff);
		Bool GetFilename(Text::StringBuilderUTF *sb);
		OSInt GetMemorySize();
		Bool SetMemorySize(OSInt minSize, OSInt maxSize);
		OSInt GetThreadIds(Data::ArrayList<UInt32> *threadList);
		void *GetHandle();
		OSInt GetModules(Data::ArrayList<Manage::ModuleInfo *> *modList);
		OSInt GetThreads(Data::ArrayList<Manage::ThreadInfo *> *threadList);
		OSInt GetHeapLists(Data::ArrayList<Int32> *heapList);
		OSInt GetHeaps(Data::ArrayList<HeapInfo*> *heapList, Int32 heapListId, OSInt maxCount);
		void FreeHeaps(Data::ArrayList<HeapInfo*> *heapList);

		Bool GetWorkingSetSize(UOSInt *minSize, UOSInt *maxSize);
		Bool GetMemoryInfo(OSInt *pageFault, OSInt *workingSetSize, OSInt *pagedPoolUsage, OSInt *nonPagedPoolUsage, OSInt *pageFileUsage);
		Bool GetTimeInfo(Data::DateTime *createTime, Data::DateTime *kernelTime, Data::DateTime *userTime);
		UInt32 GetGDIObjCount();
		UInt32 GetUserObjCount();
		UInt32 GetHandleCount();
		ProcessPriority GetPriority();
		Manage::ThreadContext::ContextType GetContextType();

		virtual UInt8 ReadMemUInt8(UInt64 addr);
		virtual UInt16 ReadMemUInt16(UInt64 addr);
		virtual UInt32 ReadMemUInt32(UInt64 addr);
		virtual UInt64 ReadMemUInt64(UInt64 addr);
		virtual OSInt ReadMemory(UInt64 addr, UInt8 *buff, OSInt reqSize);
	public:
		typedef struct
		{
			UInt32 processId;
			UInt32 threadCnt;
			UInt32 parentId;
		} ProcessInfo;

		static void *FindProcess(const UTF8Char *processName);
		static void *FindProcessW(const WChar *processName);
		static UTF8Char *FindProcessNext(UTF8Char *processNameBuff, void *sess, ProcessInfo *info);
		static WChar *FindProcessNextW(WChar *processNameBuff, void *sess, ProcessInfo *info);
		static void FindProcessClose(void *sess);
		static Int32 ExecuteProcess(const UTF8Char *cmdLine, Text::StringBuilderUTF *result);
		static Int32 ExecuteProcessW(const WChar *cmdLine, Text::StringBuilderUTF *result);
		static Bool IsAlreadyStarted();
		static Bool OpenPath(const UTF8Char *path);
		static Bool OpenPathW(const WChar *path);
		static const UTF8Char *GetPriorityName(ProcessPriority priority);
	};
};
#endif