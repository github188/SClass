#ifndef _SM_TEXT_DOC_TEXTDOCUMENT
#define _SM_TEXT_DOC_TEXTDOCUMENT
#include "Data/ArrayList.h"
#include "IO/ParsedObject.h"
#include "Media/IPrintDocument.h"
#include "Text/Doc/DocSection.h"

namespace Text
{
	namespace Doc
	{
		class TextDocument : public IO::ParsedObject, public Data::ReadingList<DocSection*>, public Media::IPrintHandler
		{
		private:
			typedef enum
			{
				PF_TEXTCOLOR = 1,
				PF_BGCOLOR = 2,
				PF_LINKCOLOR = 4,
				PF_ACTIVELINKCOLOR = 8,
				PF_VISITEDLINKCOLOR = 16
			} PropertiesFlags;

			typedef struct
			{
				OSInt currSection;
			} PrintStatus;

		private:
			const UTF8Char *docName;
			PropertiesFlags pflags;
			UInt32 textColor;
			UInt32 bgColor;
			UInt32 linkColor;
			UInt32 activeLinkColor;
			UInt32 visitedLinkColor;

			PrintStatus pStatus;
			Data::ArrayList<DocSection*> *items;
		public:
			TextDocument();
			TextDocument(const UTF8Char *name);
			virtual ~TextDocument();

			virtual IO::ParsedObject::ParserType GetParserType();

			void SetDocumentName(const UTF8Char *docName);
			UTF8Char *GetDocumentName(UTF8Char *docName);
			void SetTextColor(UInt32 textColor);
			Bool GetTextColor(UInt32 *textColor);
			void SetBGColor(UInt32 bgColor);
			Bool GetBGColor(UInt32 *bgColor);
			void SetLinkColor(UInt32 linkColor);
			Bool GetLinkColor(UInt32 *linkColor);
			void SetActiveLinkColor(UInt32 activeLinkColor);
			Bool GetActiveLinkColor(UInt32 *activeLinkColor);
			void SetVisitedLinkColor(UInt32 visitedLinkColor);
			Bool GetVisitedLinkColor(UInt32 *visitedLinkColor);

			virtual UOSInt Add(DocSection *section);
			virtual UOSInt GetCount();
			virtual DocSection *GetItem(UOSInt Index);

			virtual Bool BeginPrint(Media::IPrintDocument *doc);
			virtual Bool PrintPage(Media::DrawImage *printPage); 
			virtual Bool EndPrint(Media::IPrintDocument *doc);
		};
	}
}
#endif
