#ifndef _SM_SYNC_EVENT
#define _SM_SYNC_EVENT

namespace Sync
{
	class Event
	{
	private:
		Bool isSet;
		Bool isAuto;
	public:
		void *hand;

		Event(const UTF8Char *name);
		Event(Bool isAuto, const UTF8Char *name);
		~Event();
		void Wait();
		Bool Wait(UOSInt timeout); //true if timed out
		void Set();
		void Clear();
		Bool IsSet();
		void *GetHandle();
	};
}
#endif
