#ifndef _SM_NET_SNS_SNSRSS
#define _SM_NET_SNS_SNSRSS
#include "Crypto/Hash/CRC32R.h"
#include "Data/StringUTF8Map.h"
#include "Net/SocketFactory.h"
#include "Net/SNS/SNSControl.h"
#include "Sync/Mutex.h"
#include "Text/EncodingFactory.h"

namespace Net
{
	namespace SNS
	{
		class SNSRSS : public SNSControl
		{
		private:
			Net::SocketFactory *sockf;
			Text::EncodingFactory *encFact;
			const UTF8Char *userAgent;
			const UTF8Char *channelId;
			const UTF8Char *chName;
			const UTF8Char *chDesc;
			Data::StringUTF8Map<SNSItem *> *itemMap;
			Sync::Mutex *crcMut;
			Crypto::Hash::CRC32R *crc;

		public:
			SNSRSS(Net::SocketFactory *sockf, Text::EncodingFactory *encFact, const UTF8Char *userAgent, const UTF8Char *channelId);
			virtual ~SNSRSS();

			virtual Bool IsError();
			virtual SNSType GetSNSType();
			virtual const UTF8Char *GetChannelId();
			virtual const UTF8Char *GetName();
			virtual UTF8Char *GetDirName(UTF8Char *dirName);
			virtual OSInt GetCurrItems(Data::ArrayList<SNSItem*> *itemList);
			virtual UTF8Char *GetItemShortId(UTF8Char *buff, SNSItem *item);
			virtual Int32 GetMinIntevalMS();
			virtual Bool Reload();
		};
	}
}
#endif