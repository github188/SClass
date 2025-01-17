#ifndef _SM_TEXT_ENCODINGFACTORY
#define _SM_TEXT_ENCODINGFACTORY
#include "Data/StringUTF8Map.h"
#include "Data/ArrayListInt32.h"

namespace Text
{
	class EncodingFactory
	{
	public:
		typedef struct
		{
			UInt32 codePage;
			const Char *dotNetName;
			const Char *desc;
			const Char *internetNames[12];
		} EncodingInfo;

	private:
		Data::StringUTF8Map<EncodingInfo*> *encMap;
		static EncodingInfo encInfo[];
	public:
		EncodingFactory();
		~EncodingFactory();
		
		UInt32 GetCodePage(const UTF8Char *shortName);

		static UTF8Char *GetName(UTF8Char *buff, UInt32 codePage);
		static UTF8Char *GetInternetName(UTF8Char *buff, UInt32 codePage);
		static UTF8Char *GetDotNetName(UTF8Char *buff, UInt32 codePage);
		static UInt32 GetSystemCodePage();
		static UInt32 GetSystemLCID();
		static void GetCodePages(Data::ArrayList<UInt32> *codePages);
	};
}
#endif
