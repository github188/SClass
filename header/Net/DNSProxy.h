#ifndef _SM_NET_DNSPROXY
#define _SM_NET_DNSPROXY
#include "Data/ICaseStringUTF8Map.h"
#include "Net/DNSClient.h"
#include "Net/DNSServer.h"

namespace Net
{
	class DNSProxy
	{
	public:
		typedef struct
		{
			UInt32 ip;
			Sync::Mutex *mut;
			Data::ArrayListICaseStrUTF8 *addrList;
		} TargetInfo;

	private:
		typedef struct
		{
			UInt8 recBuff[512];
			UOSInt recSize;
			Int64 reqTime;
			UInt32 ttl;
			Int32 status; //0 = normal, 1 = blocked
			Sync::Mutex *mut;
		} RequestResult;

		typedef struct
		{
			UInt8 respBuff[512];
			UOSInt respSize;
			Sync::Event *finEvt;
		} CliRequestStatus;
	public:
		typedef void (__stdcall *DNSProxyRequest)(void *userObj, const UTF8Char *reqName, Int32 reqType, Int32 reqClass, const Net::SocketUtil::AddressInfo *addr, UInt16 reqPort, UInt32 reqId, Double timeUsed);
	private:
		Net::SocketFactory *sockf;
		Net::UDPServer *cli;
		Net::DNSServer *svr;

		UInt32 currServerIP;
		UOSInt currServerIndex;
		Data::DateTime *currIPTime;
		Sync::Mutex *dnsMut;
		Data::ArrayList<UInt32> *dnsList;
		Bool disableV6;

		Sync::Mutex *reqv4Mut;
		Data::ICaseStringUTF8Map<RequestResult*> *reqv4Map;
		Bool reqv4Updated;

		Sync::Mutex *reqv6Mut;
		Data::ICaseStringUTF8Map<RequestResult*> *reqv6Map;
		Bool reqv6Updated;

		Sync::Mutex *reqothMut;
		Data::ICaseStringUTF8Map<RequestResult*> *reqothMap;
		Bool reqothUpdated;

		Sync::Mutex *targetMut;
		Data::UInt32Map<TargetInfo*> *targetMap;
		Bool targetUpdated;

		Sync::Mutex *lastIdMut;
		UInt32 lastId;

		Sync::Mutex *cliReqMut;
		Data::UInt32Map<CliRequestStatus*> *cliReqMap;

		Sync::Mutex *hdlrMut;
		Data::ArrayList<DNSProxyRequest> *hdlrList;
		Data::ArrayList<void *> *hdlrObjs;

		Sync::Mutex *blackListMut;
		Data::ArrayList<const UTF8Char *> *blackList;

//		Sync::Mutex *whiteListMut;
//		Data::ArrayList<const UTF8Char *> *whiteList;

		static void __stdcall ClientPacket(const Net::SocketUtil::AddressInfo *addr, UInt16 port, const UInt8 *buff, UOSInt dataSize, void *userData);
		static void __stdcall OnDNSRequest(void *userObj, const UTF8Char *reqName, Int32 reqType, Int32 reqClass, const Net::SocketUtil::AddressInfo *reqAddr, UInt16 reqPort, UInt32 reqId);
		void RequestDNS(const UTF8Char *reqName, Int32 reqType, Int32 reqClass, RequestResult *req);
		UInt32 NextId();
		CliRequestStatus *NewCliReq(UInt32 id);
		void DelCliReq(UInt32 id);
		static UOSInt BuildEmptyReply(UInt8 *buff, UInt32 id, const UTF8Char *reqName, Int32 reqType, Int32 reqClass, Bool blockResult);
	public:
		DNSProxy(Net::SocketFactory *sockf, Bool analyzeTarget);
		~DNSProxy();

		Bool IsError();
		Bool IsReqListv4Chg();
		Bool IsReqListv6Chg();
		Bool IsReqListOthChg();
		Bool IsTargetChg();
		UOSInt GetReqv4List(Data::ArrayList<const UTF8Char *> *reqList); //no need release
		UOSInt GetReqv6List(Data::ArrayList<const UTF8Char *> *reqList); //no need release
		UOSInt GetReqOthList(Data::ArrayList<const UTF8Char *> *reqList); //no need release
		UOSInt GetTargetList(Data::ArrayList<TargetInfo*> *targetList); //no need release
		UOSInt SearchIPv4(Data::ArrayList<const UTF8Char *> *reqList, UInt32 ip, UInt32 mask); //no need release
		Bool GetRequestInfov4(const UTF8Char *req, Data::ArrayList<Net::DNSClient::RequestAnswer*> *ansList, Data::DateTime *reqTime, UInt32 *ttl);
		Bool GetRequestInfov6(const UTF8Char *req, Data::ArrayList<Net::DNSClient::RequestAnswer*> *ansList, Data::DateTime *reqTime, UInt32 *ttl);
		Bool GetRequestInfoOth(const UTF8Char *req, Data::ArrayList<Net::DNSClient::RequestAnswer*> *ansList, Data::DateTime *reqTime, UInt32 *ttl);
		UInt32 GetServerIP();
		void SetServerIP(UInt32 serverIP);
		void GetDNSList(Data::ArrayList<UInt32> *dnsList);
		void AddDNSIP(UInt32 serverIP);
		void SwitchDNS();
		Bool IsDisableV6();
		void SetDisableV6(Bool disableV6);
		UOSInt GetBlackList(Data::ArrayList<const UTF8Char*> *blackList);
		Bool AddBlackList(const UTF8Char *blackList);
		void HandleDNSRequest(DNSProxyRequest hdlr, void *userObj);
	};
}
#endif
