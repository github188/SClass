#ifndef _SM_NET_WEBSITE_WEBSITE48IDOLCONTROL
#define _SM_NET_WEBSITE_WEBSITE48IDOLCONTROL
#include "Net/SocketFactory.h"
#include "Net/SSLEngine.h"
#include "Text/EncodingFactory.h"

namespace Net
{
	namespace WebSite
	{
		class WebSite48IdolControl
		{
		public:
			typedef struct
			{
				Int32 id;
				Int64 recTime;
				const UTF8Char *title;
			} ItemData;
			
		private:
			Net::SocketFactory *sockf;
			Net::SSLEngine *ssl;
			Text::EncodingFactory *encFact;
			const UTF8Char *userAgent;

		public:
			WebSite48IdolControl(Net::SocketFactory *sockf, Net::SSLEngine *ssl, Text::EncodingFactory *encFact, const UTF8Char *userAgent);
			~WebSite48IdolControl();

			OSInt GetTVPageItems(OSInt pageNo, Data::ArrayList<ItemData*> *itemList);
			OSInt GetArcPageItems(OSInt pageNo, Data::ArrayList<ItemData*> *itemList);
			void FreeItems(Data::ArrayList<ItemData*> *itemList);

			Bool GetDownloadLink(Int32 videoId, Int32 linkId, Text::StringBuilderUTF *link);
			Bool GetVideoName(Int32 videoId, Text::StringBuilderUTF *name);

			static void Title2DisplayName(const UTF8Char *title, Text::StringBuilderUTF *dispName);
		};
	}
}
#endif
