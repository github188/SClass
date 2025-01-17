#ifndef _SM_NET_SNMPMANAGER
#define _SM_NET_SNMPMANAGER
#include "Data/UInt32Map.h"
#include "Net/SNMPClient.h"
#include "SSWR/SMonitor/ISMonitorCore.h"
#include "SSWR/SMonitor/SAnalogSensor.h"

namespace Net
{
	class SNMPManager
	{
	public:
		typedef struct
		{
			const UTF8Char *name;
			UOSInt index;
			UInt8 objId[64];
			UOSInt objIdLen;
			Double mulVal;
			Int32 invVal;
			SSWR::SMonitor::SAnalogSensor::ReadingType readingType;
			Bool valValid;
			Double currVal;
		} ReadingInfo;
		
		typedef struct
		{
			Net::SocketUtil::AddressInfo addr;
			const UTF8Char *community;
			UInt8 objId[64];
			UOSInt objIdLen;
			const UTF8Char *descr;
			const UTF8Char *contact;
			const UTF8Char *name;
			const UTF8Char *location;
			const UTF8Char *model;
			const UTF8Char *vendor;
			const UTF8Char *cpuName;
			UInt8 mac[6];
			Data::ArrayList<ReadingInfo *> *readingList;
		} AgentInfo;
		
	private:
		Net::SNMPClient *cli;
		Sync::Mutex *agentMut;
		Data::ArrayList<AgentInfo *> *agentList;
		Data::UInt32Map<AgentInfo*> *ipv4Agents;

		static void FreeAllItems(Data::ArrayList<Net::SNMPUtil::BindingItem*> *itemList);
	public:
		SNMPManager(Net::SocketFactory *sockf);
		~SNMPManager();

		Bool IsError();
		void UpdateValues();
		UOSInt GetAgentList(Data::ArrayList<AgentInfo*> *agentList);

		AgentInfo *AddAgent(const Net::SocketUtil::AddressInfo *addr, const UTF8Char *community);
		UOSInt AddAgents(const Net::SocketUtil::AddressInfo *addr, const UTF8Char *community, Data::ArrayList<AgentInfo*> *agentList, Bool scanIP);

		static void Agent2Record(const AgentInfo *agent, SSWR::SMonitor::ISMonitorCore::DevRecord2 *rec, Int64 *cliId);
		static Int64 Agent2CliId(const AgentInfo *agent);
	};
}
#endif
