#ifndef _SM_IO_BTLOG
#define _SM_IO_BTLOG
#include "Data/ArrayListUInt32.h"
#include "Data/UInt64Map.h"
#include "IO/ProgCtrl/BluetoothCtlProgCtrl.h"

namespace IO
{
	class BTLog
	{
	public:
		typedef struct
		{
			UInt8 mac[6];
			UInt64 macInt;
			const UTF8Char *name;
			Int32 txPower;
			Data::ArrayListUInt32 *keys;
		} LogEntry;		
	private:
		Data::UInt64Map<LogEntry*> *logs;

		static Bool IsDefaultName(const UTF8Char *name);
	public:
		BTLog();
		~BTLog();

		LogEntry *AddEntry(UInt64 macInt, const UTF8Char *name, Int32 txPower);
		void AppendList(Data::UInt64Map<IO::ProgCtrl::BluetoothCtlProgCtrl::DeviceInfo*> *devMap);
		void ClearList();
		Bool LoadFile(const UTF8Char *fileName);
		Bool StoreFile(const UTF8Char *fileName);

		Data::ArrayList<IO::BTLog::LogEntry*> *GetLogList();
	};
}
#endif