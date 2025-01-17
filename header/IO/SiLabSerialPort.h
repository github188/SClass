#ifndef _SM_IO_SILABSERIALPORT
#define _SM_IO_SILABSERIALPORT
#include "Data/ArrayList.h"
#include "Sync/Event.h"
#include "Sync/Mutex.h"
#include "IO/Stream.h"

namespace IO
{
	class SiLabDriver;
	class SiLabSerialPort : public IO::Stream
	{
	private:
		SiLabDriver *driver;
		void *handle;
		Sync::Event *rdEvt;
		Sync::Mutex *rdMut;
		Bool reading;

	public:
		SiLabSerialPort(void *handle, IO::SiLabDriver *driver);
		virtual ~SiLabSerialPort();

		virtual UOSInt Read(UInt8 *buff, UOSInt size);
		virtual UOSInt Write(const UInt8 *buff, UOSInt size);

		virtual void *BeginRead(UInt8 *buff, UOSInt size, Sync::Event *evt);
		virtual UOSInt EndRead(void *reqData);
		virtual void CancelRead(void *reqData);
		virtual void *BeginWrite(const UInt8 *buff, UOSInt size, Sync::Event *evt);
		virtual UOSInt EndWrite(void *reqData);
		virtual void CancelWrite(void *reqData);

		virtual Int32 Flush();
		virtual void Close();
		virtual Bool Recover();
	};
}
#endif
