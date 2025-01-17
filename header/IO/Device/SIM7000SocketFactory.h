#ifndef _SM_IO_DEVICE_SIM7000SOCKETFACTORY
#define _SM_IO_DEVICE_SIM7000SOCKETFACTORY
#include "Data/LinkedList.h"
#include "IO/Device/SIM7000.h"
#include "Net/SocketFactory.h"
#include "Net/SocketUtil.h"

namespace IO
{
	namespace Device
	{
		class SIM7000SocketFactory : public Net::SocketFactory
		{
		private:
			typedef enum
			{
				SS_EMPTY,
				SS_TCP_UNOPENED,
				SS_TCP_OPENED,
				SS_UDP_UNOPENED,
				SS_UDP_OPENED
			} SocketState;

			typedef struct
			{
				UInt32 remoteIP;
				UInt16 remotePort;
				UOSInt dataSize;
				UInt8 data[1];
			} DataPacket;
			

			typedef struct
			{
				SocketState state;
				UInt32 udpRIP;
				UInt16 udpRPort;
				Sync::Mutex *dataMut;
				Data::LinkedList *dataList;
				Sync::Event *dataEvt;
			} SocketStatus;
			
		private:
			IO::Device::SIM7000 *modem;
			Bool needRelease;
			const UTF8Char *apn;
			SocketStatus status[8];

			static void __stdcall OnReceiveData(void *userObj, UOSInt index, UInt32 remoteIP, UInt16 remotePort, const UInt8 *buff, UOSInt buffSize);
			void CloseAllSockets();
		public:
			SIM7000SocketFactory(IO::Device::SIM7000 *modem, Bool needRelease);
			virtual ~SIM7000SocketFactory();

			void SetAPN(const UTF8Char *apn);
			const UTF8Char *GetAPN();
			void Init();
			Bool NetworkStart();
			Bool NetworkEnd();

			virtual UInt32 *CreateTCPSocketv4(); ////////////////////////
			virtual UInt32 *CreateTCPSocketv6(); ////////////////////////
			virtual UInt32 *CreateUDPSocketv4();
			virtual UInt32 *CreateUDPSocketv6(); ////////////////////////

			virtual UInt32 *CreateICMPIPv4Socket(UInt32 ip);
			virtual UInt32 *CreateUDPRAWv4Socket(UInt32 ip);
			virtual UInt32 *CreateRAWIPv4Socket(UInt32 ip);
			virtual UInt32 *CreateARPSocket();
			virtual UInt32 *CreateRAWSocket();

			virtual void DestroySocket(UInt32 *socket);
			virtual Bool SocketIsInvalid(UInt32 *socket);
			virtual Bool SocketBindv4(UInt32 *socket, UInt32 ip, UInt16 port);
			virtual Bool SocketBind(UInt32 *socket, const Net::SocketUtil::AddressInfo *addr, UInt16 port); ////////////////////////
			virtual Bool SocketListen(UInt32 *socket); ////////////////////////
			virtual UInt32 *SocketAccept(UInt32 *socket); ////////////////////////
			virtual Int32 SocketGetLastError(); ////////////////////////
			virtual Bool GetRemoteAddr(UInt32 *socket, Net::SocketUtil::AddressInfo *addr, UInt16 *port); ////////////////////////
			virtual Bool GetLocalAddr(UInt32 *socket, Net::SocketUtil::AddressInfo *addr, UInt16 *port); ////////////////////////
			virtual Int32 SocketGetFD(UInt32 *socket);

			virtual void SetDontLinger(UInt32 *socket, Bool val); ////////////////////////
			virtual void SetLinger(UInt32 *socket, UInt32 ms); ////////////////////////
			virtual void SetRecvBuffSize(UInt32 *socket, Int32 buffSize); ////////////////////////
			virtual void SetNoDelay(UInt32 *socket, Bool val);
			virtual void SetRecvTimeout(UInt32 *socket, Int32 ms); ////////////////////////
			virtual void SetReuseAddr(UInt32 *socket, Bool val); ////////////////////////
			virtual void SetIPv4TTL(UInt32 *socket, Int32 ttl);
			virtual void SetBroadcast(UInt32 *socket, Bool val);
			virtual void AddIPMembership(UInt32 *socket, UInt32 ip); ////////////////////////

			virtual UOSInt SendData(UInt32 *socket, const UInt8 *buff, UOSInt buffSize, ErrorType *et); ////////////////////////
			virtual UOSInt ReceiveData(UInt32 *socket, UInt8 *buff, UOSInt buffSize, ErrorType *et); ////////////////////////
			virtual void *BeginReceiveData(UInt32 *socket, UInt8 *buff, UOSInt buffSize, Sync::Event *evt, ErrorType *et); ////////////////////////
			virtual UOSInt EndReceiveData(void *reqData, Bool toWait, Bool *incomplete); ////////////////////////
			virtual void CancelReceiveData(void *reqData); ////////////////////////

			virtual UOSInt UDPReceive(UInt32 *socket, UInt8 *buff, UOSInt buffSize, Net::SocketUtil::AddressInfo *addr, UInt16 *port, ErrorType *et); ////////////////////////
			virtual UOSInt SendTo(UInt32 *socket, const UInt8 *buff, UOSInt buffSize, const Net::SocketUtil::AddressInfo *addr, UInt16 port);
			virtual UOSInt SendToIF(UInt32 *socket, const UInt8 *buff, UOSInt buffSize, const UTF8Char *ifName);

			virtual Bool IcmpSendEcho2(const Net::SocketUtil::AddressInfo *addr, UInt32 *respTime_us, UInt32 *ttl);

			virtual Bool Connect(UInt32 *socket, UInt32 ip, UInt16 port); ////////////////////////
			virtual Bool Connect(UInt32 *socket, const Net::SocketUtil::AddressInfo *addr, UInt16 port); ////////////////////////
			virtual void ShutdownSend(UInt32 *socket); ////////////////////////

			virtual Bool SocketGetReadBuff(UInt32 *socket, UInt32 *size); ////////////////////////

			virtual Bool DNSResolveIPDef(const Char *host, Net::SocketUtil::AddressInfo *addr);
			virtual Bool GetDefDNS(Net::SocketUtil::AddressInfo *addr);
			virtual UOSInt GetDNSList(Data::ArrayList<UInt32> *dnsList);
			virtual Bool LoadHosts(Net::DNSHandler *dnsHdlr);

			virtual Bool ARPAddRecord(UOSInt ifIndex, const UInt8 *hwAddr, UInt32 ipv4);

			virtual UOSInt GetConnInfoList(Data::ArrayList<Net::ConnectionInfo*> *connInfoList);
			virtual Bool GetIPInfo(IPInfo *info); //////////////////////////////////
			virtual Bool GetTCPInfo(TCPInfo *info); //////////////////////////////////
			virtual Bool GetUDPInfo(UDPInfo *info); //////////////////////////////////
			virtual UOSInt QueryPortInfos(Data::ArrayList<PortInfo*> *portInfoList, ProtocolType protoType, UInt16 procId);
			virtual void FreePortInfos(Data::ArrayList<PortInfo*> *portInfoList);
			virtual UOSInt QueryPortInfos2(Data::ArrayList<PortInfo2*> *portInfoList, ProtocolType protoType, UInt16 procId);
			virtual void FreePortInfos2(Data::ArrayList<PortInfo2*> *portInfoList);
		};
	}
}
#endif
