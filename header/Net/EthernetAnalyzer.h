#ifndef _SM_NET_ETHERNETANALYZER
#define _SM_NET_ETHERNETANALYZER
#include "Data/ICaseStringUTF8Map.h"
#include "Data/Integer32Map.h"
#include "Data/Integer64Map.h"
#include "IO/IWriter.h"
#include "IO/ParsedObject.h"
#include "Net/DNSClient.h"
#include "Text/StringBuilderUTF.h"

namespace Net
{
	class EthernetAnalyzer : public IO::ParsedObject
	{
	public:
		typedef void (__stdcall *Pingv4Handler)(void *userData, UInt32 fromIP, UInt32 toIP, UInt8 ttl, UOSInt packetSize);

		typedef enum
		{
			AT_DEVICE = 1,
			AT_IPTRANSFER = 2,
			AT_DNSCLI = 4,
			AT_DNSREQ = 8,
			AT_DNSTARGET = 16,
			AT_IPLOG = 32,
			AT_DHCP = 64,
			AT_UDP = AT_IPTRANSFER | AT_DNSCLI | AT_DNSREQ | AT_DNSTARGET | AT_IPLOG | AT_DHCP,
			AT_ICMP = AT_IPTRANSFER | AT_IPLOG,
			AT_ALL = AT_DEVICE | AT_IPTRANSFER | AT_DNSCLI | AT_DNSREQ | AT_DNSTARGET | AT_IPLOG | AT_DHCP
		} AnalyzeType;

		typedef struct
		{
			UInt32 srcIP;
			UInt32 destIP;
			Int64 tcpCnt;
			Int64 udpCnt;
			Int64 icmpCnt;
			Int64 otherCnt;
			Int64 tcpSize;
			Int64 udpSize;
			Int64 icmpSize;
			Int64 otherSize;
		} IPTranStatus;

		typedef struct
		{
			Int64 macAddr;
			UInt32 ipv4Addr[4];
			Net::SocketUtil::AddressInfo ipv6Addr;
			Int64 ipv4SrcCnt;
			Int64 ipv4DestCnt;
			Int64 ipv6SrcCnt;
			Int64 ipv6DestCnt;
			Int64 othSrcCnt;
			Int64 othDestCnt;
			const UTF8Char *name;
		} MACStatus;

		typedef struct
		{
			Int32 year;
			Int32 month;
			Int32 day;
			Int32 hour;
			Int64 reqCount;
		} DNSCliHourInfo;

		typedef struct
		{
			Int32 cliId;
			Net::SocketUtil::AddressInfo addr;
			Sync::Mutex *mut;
			Data::ArrayList<DNSCliHourInfo*> *hourInfos;
		} DNSClientInfo;

		typedef struct
		{
			UInt8 recBuff[512];
			OSInt recSize;
			Int64 reqTime;
			Int32 ttl;
			Int32 status; //0 = normal, 1 = blocked
			Sync::Mutex *mut;
		} DNSRequestResult;

		typedef struct
		{
			UInt32 ip;
			Sync::Mutex *mut;
			Data::ArrayListICaseStrUTF8 *addrList;
		} DNSTargetInfo;

		typedef struct
		{
			UInt32 ip;
			Sync::Mutex *mut;
			Data::ArrayList<const UTF8Char*> *logList;
		} IPLogInfo;

		typedef struct
		{
			Int64 iMAC;
			Bool updated;
			Sync::Mutex *mut;
			Int64 ipAddrTime;
			UInt32 ipAddr;
			UInt32 ipAddrLease;
			UInt32 subnetMask;
			UInt32 gwAddr;
			UInt32 dhcpServer;
			UInt32 renewTime;
			UInt32 rebindTime;
			UInt32 router;
			UInt32 dns[4];
			const UTF8Char *hostName;
			const UTF8Char *vendorClass;
		} DHCPInfo;
		
	private:
		AnalyzeType atype;
		Sync::Mutex *ipTranMut;
		Data::Integer64Map<IPTranStatus*> *ipTranMap;
		Sync::Mutex *macMut;
		Data::Integer64Map<MACStatus*> *macMap;
		Sync::Mutex *dnsCliInfoMut;
		Data::Integer32Map<DNSClientInfo*> *dnsCliInfos;
		Sync::Mutex *dnsReqv4Mut;
		Data::ICaseStringUTF8Map<DNSRequestResult*> *dnsReqv4Map;
		Sync::Mutex *dnsReqv6Mut;
		Data::ICaseStringUTF8Map<DNSRequestResult*> *dnsReqv6Map;
		Sync::Mutex *dnsReqOthMut;
		Data::ICaseStringUTF8Map<DNSRequestResult*> *dnsReqOthMap;
		Sync::Mutex *dnsTargetMut;
		Data::Integer32Map<DNSTargetInfo*> *dnsTargetMap;
		Sync::Mutex *ipLogMut;
		Data::Integer32Map<IPLogInfo*> *ipLogMap;
		Sync::Mutex *dhcpMut;
		Data::Integer64Map<DHCPInfo*> *dhcpMap;

		Pingv4Handler pingv4ReqHdlr;
		void *pingv4ReqObj;

		Int64 packetCnt;
		Int64 packetTotalSize;
		Bool isFirst;
		IO::IWriter *errWriter;

		static void NetBIOSDecName(UTF8Char *nameBuff, OSInt nameSize);

		MACStatus *MACGet(Int64 macAddr);

	public:
		EthernetAnalyzer(IO::IWriter *errWriter, AnalyzeType ctype, const UTF8Char *name);
		virtual ~EthernetAnalyzer();

		virtual IO::ParsedObject::ParserType GetParserType();

		Int64 GetPacketCnt();
		Int64 GetPacketTotalSize();
		void IPTranBeginGet();
		void IPTranEndGet();
		Data::ArrayList<IPTranStatus*> *IPTranGetList();
		OSInt IPTranGetCount();
		void MACBeginGet();
		void MACEndGet();
		Data::ArrayList<MACStatus*> *MACGetList();
		void DNSCliBeginGet();
		void DNSCliEndGet();
		Data::ArrayList<DNSClientInfo*> *DNSCliGetList();
		OSInt DNSCliGetCount();
		OSInt DNSReqv4GetList(Data::ArrayList<const UTF8Char *> *reqList); //no need release
		OSInt DNSReqv4GetCount();
		Bool DNSReqv4GetInfo(const UTF8Char *req, Data::ArrayList<Net::DNSClient::RequestAnswer*> *ansList, Data::DateTime *reqTime, Int32 *ttl);
		OSInt DNSReqv6GetList(Data::ArrayList<const UTF8Char *> *reqList); //no need release
		OSInt DNSReqv6GetCount();
		Bool DNSReqv6GetInfo(const UTF8Char *req, Data::ArrayList<Net::DNSClient::RequestAnswer*> *ansList, Data::DateTime *reqTime, Int32 *ttl);
		OSInt DNSReqOthGetList(Data::ArrayList<const UTF8Char *> *reqList); //no need release
		OSInt DNSReqOthGetCount();
		Bool DNSReqOthGetInfo(const UTF8Char *req, Data::ArrayList<Net::DNSClient::RequestAnswer*> *ansList, Data::DateTime *reqTime, Int32 *ttl);
		OSInt DNSTargetGetList(Data::ArrayList<DNSTargetInfo *> *targetList); //no need release
		OSInt DNSTargetGetCount();
		void DHCPBeginGet();
		void DHCPEndGet();
		Data::ArrayList<DHCPInfo*> *DHCPGetList();
		void IPLogBeginGet();
		void IPLogEndGet();
		Data::ArrayList<IPLogInfo*> *IPLogGetList();
		OSInt IPLogGetCount();

		Bool PacketData(UInt32 linkType, const UInt8 *packet, OSInt packetSize); //Return valid
		Bool PacketNull(const UInt8 *packet, OSInt packetSize); //Return valid
		Bool PacketEthernet(const UInt8 *packet, OSInt packetSize); //Return valid
		Bool PacketLinux(const UInt8 *packet, OSInt packetSize); //Return valid
		Bool PacketEthernetData(const UInt8 *packet, OSInt packetSize, UInt16 etherType, Int64 srcMAC, Int64 destMAC); //Return valid
		Bool PacketIPv4(const UInt8 *packet, OSInt packetSize, Int64 srcMAC, Int64 destMAC); //Return valid
		Bool PacketIPv6(const UInt8 *packet, OSInt packetSize, Int64 srcMAC, Int64 destMAC); //Return valid
		Bool PacketARP(const UInt8 *packet, OSInt packetSize, Int64 srcMAC, Int64 destMAC); //Return valid

		AnalyzeType GetAnalyzeType();
		void HandlePingv4Request(Pingv4Handler pingv4Hdlr, void *userObj);

		static Bool PacketDataGetName(UInt32 linkType, const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static Bool PacketNullGetName(const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static Bool PacketEthernetGetName(const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static Bool PacketLinuxGetName(const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static Bool PacketEthernetDataGetName(UInt16 etherType, const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static Bool PacketIPv4GetName(const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static Bool PacketIPv6GetName(const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static Bool PacketIPDataGetName(UInt8 protocol, const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static void PacketDataGetDetail(UInt32 linkType, const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static void PacketNullGetDetail(const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static void PacketEthernetGetDetail(const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static void PacketLinuxGetDetail(const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static void PacketEthernetDataGetDetail(UInt16 etherType, const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static void PacketIEEE802_2LLCGetDetail(const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static void PacketIPv4GetDetail(const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static void PacketIPv6GetDetail(const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static void PacketIPDataGetDetail(UInt8 protocol, const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static void PacketUDPGetDetail(UInt16 srcPort, UInt16 destPort, const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static void PacketDNSGetDetail(const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);
		static void PacketLoRaMACGetDetail(const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);

		static OSInt HeaderIPv4GetDetail(const UInt8 *packet, OSInt packetSize, Text::StringBuilderUTF *sb);

		static void XMLWellFormat(const UTF8Char *buff, OSInt buffSize, Text::StringBuilderUTF *sb);
		static UTF8Char *NetBIOSGetName(UTF8Char *sbuff, const UTF8Char *nbName);
		static const UTF8Char *TCPPortGetName(UInt16 port); //null = unknwon
		static const UTF8Char *UDPPortGetName(UInt16 port); //null = unknwon
		static const UTF8Char *LinkTypeGetName(UInt32 linkType);
		static const UTF8Char *LSAPGetName(UInt8 lsap);
		static const UTF8Char *DHCPOptionGetName(UInt8 t);
	};
}
#endif