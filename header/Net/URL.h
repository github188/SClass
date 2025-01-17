#ifndef _SM_NET_URL
#define _SM_NET_URL
#include "IO/ParsedObject.h"
#include "Net/SocketFactory.h"
#include "Net/SSLEngine.h"

namespace Net
{
	class URL
	{
	public:
		static IO::ParsedObject *OpenObject(const UTF8Char *url, const UTF8Char *userAgent, Net::SocketFactory *sockf, Net::SSLEngine *ssl);
	};
}
#endif
