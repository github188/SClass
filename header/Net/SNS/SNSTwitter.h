#ifndef _SM_NET_SNS_SNSTWITTER
#define _SM_NET_SNS_SNSTWITTER
#include "Data/Int64Map.h"
#include "Net/SocketFactory.h"
#include "Net/SSLEngine.h"
#include "Net/SNS/SNSControl.h"
#include "Net/WebSite/WebSiteTwitterControl.h"
#include "Text/EncodingFactory.h"

namespace Net
{
	namespace SNS
	{
		class SNSTwitter : public SNSControl
		{
		private:
			Net::WebSite::WebSiteTwitterControl *ctrl;
			const UTF8Char *channelId;
			const UTF8Char *chName;
			const UTF8Char *chDesc;
			Bool chError;
			Data::Int64Map<SNSItem *> *itemMap;

		public:
			SNSTwitter(Net::SocketFactory *sockf, Net::SSLEngine *ssl, Text::EncodingFactory *encFact, const UTF8Char *userAgent, const UTF8Char *channelId);
			virtual ~SNSTwitter();

			virtual Bool IsError();
			virtual SNSType GetSNSType();
			virtual const UTF8Char *GetChannelId();
			virtual const UTF8Char *GetName();
			virtual UTF8Char *GetDirName(UTF8Char *dirName);
			virtual UOSInt GetCurrItems(Data::ArrayList<SNSItem*> *itemList);
			virtual UTF8Char *GetItemShortId(UTF8Char *buff, SNSItem *item);
			virtual Int32 GetMinIntevalMS();
			virtual Bool Reload();
		};
	}
}
#endif
