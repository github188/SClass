#ifndef _SM_NET_OPENSSLENGINE
#define _SM_NET_OPENSSLENGINE
#include "Net/SSLEngine.h"

namespace Net
{
	class OpenSSLEngine : public Net::SSLEngine
	{
	private:
		struct ClassData;
		struct ClassData *clsData;
		Bool skipCertCheck;

		virtual Net::TCPClient *CreateServerConn(UInt32 *s);
	public:
		OpenSSLEngine(Net::SocketFactory *sockf, Method method);
		virtual ~OpenSSLEngine();
		
		virtual void SetSkipCertCheck(Bool skipCertCheck);
		virtual Bool IsError();
		virtual Bool SetServerCertsASN1(Crypto::X509File *certASN1, Crypto::X509File *keyASN1);
		virtual UTF8Char *GetErrorDetail(UTF8Char *sbuff);
		virtual Net::TCPClient *Connect(const UTF8Char *hostName, UInt16 port, ErrorType *err);
		virtual Bool GenerateCert(const UTF8Char *country, const UTF8Char *company, const UTF8Char *commonName, Crypto::X509File **certASN1, Crypto::X509File **keyASN1);
	};
}
#endif
