#ifndef _SM_TEXT_LOCALE
#define _SM_TEXT_LOCALE
#include "Data/StringUTF8Map.h"

namespace Text
{
	class Locale
	{
	public:
		typedef struct
		{
			Int32 lcid;
			const UTF8Char *shortName;
			const UTF8Char *desc;
			Int32 defCodePage;
		} LocaleEntry;

	private:
		static LocaleEntry locales[];
	public:
		static LocaleEntry *GetLocaleEntry(Int32 lcid); //see Text::EncodingFactory::GetSystemLCID()
		static LocaleEntry *GetLocaleEntryByCodePage(Int32 codePage);

	private:
		Data::StringUTF8Map<LocaleEntry*> *names;
	public:
		Locale();
		~Locale();

		LocaleEntry *GetLocaleEntryByName(const UTF8Char *name);
	};
};
#endif