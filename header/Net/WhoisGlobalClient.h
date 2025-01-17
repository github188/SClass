#ifndef _SM_NET_WHOISGLOBALCLIENT
#define _SM_NET_WHOISGLOBALCLIENT
#include "Net/WhoisClient.h"

namespace Net
{
	class WhoisGlobalClient : public WhoisClient
	{
	public:
		WhoisGlobalClient(Net::SocketFactory *sockf);
		virtual ~WhoisGlobalClient();

		virtual WhoisRecord *RequestIP(UInt32 ip);
	};
};
#endif
