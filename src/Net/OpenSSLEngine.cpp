#include "Stdafx.h"
#include "Data/DateTime.h"
#include "IO/StmData/MemoryData.h"
#include "Net/OpenSSLClient.h"
#include "Net/OpenSSLCore.h"
#include "Net/OpenSSLEngine.h"
#include "Parser/FileParser/X509Parser.h"
#include "Text/MyString.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

struct Net::OpenSSLEngine::ClassData
{
	SSL_CTX *ctx;
};

Net::TCPClient *Net::OpenSSLEngine::CreateServerConn(UInt32 *s)
{
	SSL *ssl = SSL_new(this->clsData->ctx);
	this->sockf->SetRecvTimeout(s, 2000);
	this->sockf->SetNoDelay(s, true);
	SSL_set_fd(ssl, this->sockf->SocketGetFD(s));
	if (SSL_accept(ssl) <= 0)
	{
		SSL_free(ssl);
		this->sockf->DestroySocket(s);
		return 0;
	}
	else
	{
		Net::TCPClient *cli;
		NEW_CLASS(cli, OpenSSLClient(this->sockf, ssl, s));
		return cli;
	}
}

Net::OpenSSLEngine::OpenSSLEngine(Net::SocketFactory *sockf, Method method) : Net::SSLEngine(sockf)
{
	Net::OpenSSLCore::Init();
	const SSL_METHOD *m = 0;
	switch (method)
	{
	case M_SSLV3:
#ifdef OPENSSL_NO_SSL3_METHOD
		m = SSLv23_method();
#else
		m = SSLv3_method();
#endif
		break;
	case M_SSLV23:
		m = SSLv23_method();
		break;
	case M_DEFAULT:
	case M_TLS:
		m = TLS_method();
		break;
	case M_TLSV1:
#ifdef OPENSSL_NO_TLS1_METHOD
		m = TLS_method();
#else
		m = TLSv1_method();
#endif
		break;
	case M_TLSV1_1:
#ifdef OPENSSL_NO_TLS1_1_METHOD
		m = TLS_method();
#else
		m = TLSv1_1_method();
#endif
		break;
	case M_TLSV1_2:
#ifdef OPENSSL_NO_TLS1_2_METHOD
		m = TLS_method();
#else
		m = TLSv1_2_method();
#endif
		break;
	case M_DTLS:
		m = DTLS_method();
		break;
	case M_DTLSV1:
#ifdef OPENSSL_NO_DTLS1_METHOD
		m = DTLS_method();
#else
		m = DTLSv1_method();
#endif
		break;
	case M_DTLSV1_2:
#ifdef OPENSSL_NO_DTLS1_2_METHOD
		m = DTLS_method();
#else
		m = DTLSv1_2_method();
#endif
		break;
	}
	this->clsData = MemAlloc(ClassData, 1);
	this->clsData->ctx = SSL_CTX_new(m);
	this->skipCertCheck = false;
}

Net::OpenSSLEngine::~OpenSSLEngine()
{
	if (this->clsData->ctx)
	{
		SSL_CTX_free(this->clsData->ctx);
	}
	MemFree(this->clsData);
	Net::OpenSSLCore::Deinit();
}

void Net::OpenSSLEngine::SetSkipCertCheck(Bool skipCertCheck)
{
	this->skipCertCheck = skipCertCheck;
}

Bool Net::OpenSSLEngine::IsError()
{
	return this->clsData->ctx == 0;
}

Bool Net::OpenSSLEngine::SetServerCertsASN1(Crypto::X509File *certASN1, Crypto::X509File *keyASN1)
{
	if (this->clsData->ctx == 0)
	{
		return false;
	}
	
	if (certASN1 != 0 && certASN1->GetFileType() == Crypto::X509File::FT_CERT && keyASN1 != 0 && keyASN1->GetFileType() == Crypto::X509File::FT_PRIV_KEY)
	{
		SSL_CTX_set_ecdh_auto(this->clsData->ctx, 1);
		if (SSL_CTX_use_certificate_ASN1(this->clsData->ctx, (int)certASN1->GetASN1BuffSize(), certASN1->GetASN1Buff()) <= 0)
		{
			return false;
		}
		if (SSL_CTX_use_PrivateKey_ASN1(EVP_PKEY_RSA, this->clsData->ctx, keyASN1->GetASN1Buff(), (long)keyASN1->GetASN1BuffSize()) <= 0)
		{
			return false;
		}
		return true;
	}
	return false;

/*	SSL_CTX_set_ecdh_auto(this->clsData->ctx, 1);
	if (SSL_CTX_use_certificate_file(this->clsData->ctx, (const Char*)certFile, SSL_FILETYPE_PEM) <= 0)
	{
		return false;
	}

	if (SSL_CTX_use_PrivateKey_file(this->clsData->ctx, (const Char*)keyFile, SSL_FILETYPE_PEM) <= 0 )
	{
		return false;
	}
	return true;*/
}

UTF8Char *Net::OpenSSLEngine::GetErrorDetail(UTF8Char *sbuff)
{
	UInt32 err = (UInt32)ERR_get_error();
	if (err == 0)
	{
		*sbuff = 0;
		return sbuff;
	}
	ERR_error_string(err, (char*)sbuff);
	return &sbuff[Text::StrCharCnt(sbuff)];
}

Net::TCPClient *Net::OpenSSLEngine::Connect(const UTF8Char *hostName, UInt16 port, ErrorType *err)
{
	Net::SocketUtil::AddressInfo addr;
	if (!this->sockf->DNSResolveIP(hostName, &addr))
	{
		if (err)
			*err = ET_HOSTNAME_NOT_RESOLVED;
		return 0;
	}
	SSL *ssl = SSL_new(this->clsData->ctx);
	if (ssl == 0)
	{
		if (err)
			*err = ET_OUT_OF_MEMORY;
		return 0;
	}
	UInt32 *s;
	if (addr.addrType == Net::SocketUtil::AT_IPV4)
	{
		s = this->sockf->CreateTCPSocketv4();
	}
	else if (addr.addrType == Net::SocketUtil::AT_IPV6)
	{
		s = this->sockf->CreateTCPSocketv6();
	}
	else
	{
		SSL_free(ssl);
		if (err)
			*err = ET_HOSTNAME_NOT_RESOLVED;
		return 0;
	}
	if (s == 0)
	{
		SSL_free(ssl);
		if (err)
			*err = ET_OUT_OF_MEMORY;
		return 0;
	}
	if (!this->sockf->Connect(s, &addr, port))
	{
		this->sockf->DestroySocket(s);
		SSL_free(ssl);
		if (err)
			*err = ET_CANNOT_CONNECT;
		return 0;
	}
	this->sockf->SetNoDelay(s, true);
	this->sockf->SetRecvTimeout(s, 2000);
	SSL_set_fd(ssl, this->sockf->SocketGetFD(s));
	if (SSL_connect(ssl) <= 0)
	{
		this->sockf->DestroySocket(s);
		SSL_free(ssl);
		if (err)
			*err = ET_INIT_SESSION;
		return 0;
	}
	if (!this->skipCertCheck)
	{
		X509 *cert = SSL_get_peer_certificate(ssl);
		if (cert == 0)
		{
			this->sockf->DestroySocket(s);
			SSL_free(ssl);
			if (err)
				*err = ET_CERT_NOT_FOUND;
			return 0;
		}

		Char sbuff[512];
		Char *sarr[10];
		ASN1_TIME *notBefore = X509_get_notBefore(cert);
		ASN1_TIME *notAfter = X509_get_notAfter(cert);
		Data::DateTime dt;
		Int64 currTime;
		dt.SetCurrTimeUTC();
		currTime = dt.ToTicks();
		tm tm;
		ASN1_TIME_to_tm(notBefore, &tm);
		dt.SetValue((UInt16)(tm.tm_year + 1900), tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, 0, (Int8)(tm.tm_gmtoff / 60 / 15));
		if (currTime < dt.ToTicks())
		{
			this->sockf->DestroySocket(s);
			SSL_free(ssl);
			if (err)
				*err = ET_INVALID_PERIOD;
			return 0;
		}
		ASN1_TIME_to_tm(notAfter, &tm);
		dt.SetValue((UInt16)(tm.tm_year + 1900), tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, 0, (Int8)(tm.tm_gmtoff / 60 / 15));
		if (currTime > dt.ToTicks())
		{
			this->sockf->DestroySocket(s);
			SSL_free(ssl);
			if (err)
				*err = ET_INVALID_PERIOD;
			return 0;
		}

		X509_NAME *name = X509_get_subject_name(cert);
		X509_NAME_oneline(name, sbuff, 512);
		Bool nameValid = false;
		UOSInt i = 0;
		UOSInt j = Text::StrSplit(sarr, 10, sbuff, '/');
		while (i < j)
		{
			if (Text::StrStartsWith(sarr[i], "CN="))
			{
				if (Text::StrEquals(&sarr[i][3], (const Char*)hostName))
				{
					nameValid = true;
				}
				break;
			}
			i++;
		}
		if (!nameValid)
		{
			this->sockf->DestroySocket(s);
			SSL_free(ssl);
			if (err)
				*err = ET_INVALID_NAME;
			return 0;
		}
		X509_NAME *issuer = X509_get_issuer_name(cert);
		if (X509_NAME_cmp(name, issuer) == 0)
		{
			this->sockf->DestroySocket(s);
			SSL_free(ssl);
			if (err)
				*err = ET_SELF_SIGN;
			return 0;
		}
	}
	Net::TCPClient *cli;
	NEW_CLASS(cli, OpenSSLClient(this->sockf, ssl, s));
	return cli;
}

Bool Net::OpenSSLEngine::GenerateCert(const UTF8Char *country, const UTF8Char *company, const UTF8Char *commonName, Crypto::X509File **certASN1, Crypto::X509File **keyASN1)
{
	if (certASN1 == 0 || keyASN1 == 0)
	{
		return false;
	}

	Bool succ = false;
	BIGNUM *bn = BN_new();
	BN_set_word(bn, RSA_F4);
	RSA *rsa = RSA_new();
	if (RSA_generate_key_ex(rsa, 2048, bn, 0) > 0)
	{
		EVP_PKEY *pkey = EVP_PKEY_new();
		X509 *cert = X509_new();
		EVP_PKEY_assign(pkey, EVP_PKEY_RSA, rsa);
		ASN1_INTEGER_set(X509_get_serialNumber(cert), 1);

		X509_gmtime_adj(X509_get_notBefore(cert), 0);
		X509_gmtime_adj(X509_get_notAfter(cert), 365 * 24 * 3600);

		X509_set_pubkey(cert, pkey);

		X509_name_st *name = X509_get_subject_name(cert);
		X509_NAME_add_entry_by_txt(name, "C",  MBSTRING_ASC, country, -1, -1, 0);
		X509_NAME_add_entry_by_txt(name, "O",  MBSTRING_ASC, company, -1, -1, 0);
		X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, commonName, -1, -1, 0);

		X509_set_issuer_name(cert, name);
		X509_sign(cert, pkey, EVP_sha256());

		BIO *bio1;
		BIO *bio2;
		UInt8 buff[4096];
		Crypto::X509File *pobjKey = 0;
		Crypto::X509File *pobjCert = 0;
		IO::StmData::MemoryData *mdata;
		Parser::FileParser::X509Parser parser;

		BIO_new_bio_pair(&bio1, 4096, &bio2, 4096);
		PEM_write_bio_PrivateKey(bio1, pkey, nullptr, nullptr, 0, nullptr, nullptr);
		int readSize = BIO_read(bio2, buff, 4096);
		if (readSize > 0)
		{
			NEW_CLASS(mdata, IO::StmData::MemoryData(buff, (UInt32)readSize));
			pobjKey = (Crypto::X509File*)parser.ParseFile(mdata, 0, IO::ParsedObject::PT_ASN1_DATA);
			DEL_CLASS(mdata);
		}
		PEM_write_bio_X509(bio1, cert);
		readSize = BIO_read(bio2, buff, 4096);
		if (readSize > 0)
		{
			NEW_CLASS(mdata, IO::StmData::MemoryData(buff, (UInt32)readSize));
			pobjCert = (Crypto::X509File*)parser.ParseFile(mdata, 0, IO::ParsedObject::PT_ASN1_DATA);
			DEL_CLASS(mdata);
		}
		BIO_free(bio1);
		BIO_free(bio2);
		X509_free(cert);
		EVP_PKEY_free(pkey);

		if (pobjCert && pobjKey)
		{
			succ = true;
			*certASN1 = pobjCert;
			*keyASN1 = pobjKey;
		}
		else
		{
			SDEL_CLASS(pobjCert);
			SDEL_CLASS(pobjKey);
		}
	}
	else
	{
		RSA_free(rsa);
	}
	BN_free(bn);
	return succ;
}
