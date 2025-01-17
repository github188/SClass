#ifndef _SM_PARSER_FILEPARSER_XLSPARSER
#define _SM_PARSER_FILEPARSER_XLSPARSER
#include "Data/Int32Map.h"
#include "IO/FileParser.h"
#include "Text/StringBuilderUTF.h"
#include "Text/SpreadSheet/Workbook.h"

namespace Parser
{
	namespace FileParser
	{
		class XLSParser : public IO::FileParser
		{
		private:
			typedef struct
			{
				Text::SpreadSheet::Worksheet *ws;
				UInt64 ofst;
			} WorksheetStatus;

			typedef struct
			{
				Int32 height;
				Int32 grbit;
				Int32 icv;
				Int32 bls;
				Int32 sss;
				Int32 uls;
				Int32 bFamily;
				Int32 bCharSet;
				const UTF8Char *fontName;
			} FontInfo;

			typedef struct
			{
				Data::ArrayList<const UTF8Char *> *sst;
				Data::ArrayList<WorksheetStatus*> *wsList;
				Data::ArrayList<FontInfo *> *fontList;
				Data::Int32Map<const UTF8Char *> *formatMap;
				UInt32 palette[56];
			} WorkbookStatus;

		public:
			XLSParser();
			virtual ~XLSParser();

			virtual Int32 GetName();
			virtual void PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t);
			virtual IO::ParsedObject::ParserType GetParserType();
			virtual IO::ParsedObject *ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType);
		private:
			static Bool ParseWorkbook(IO::IStreamData *fd, UInt64 ofst, UInt64 ofstRef, Text::SpreadSheet::Workbook *wb);
			static Bool ParseWorksheet(IO::IStreamData *fd, UInt64 ofst, Text::SpreadSheet::Workbook *wb, Text::SpreadSheet::Worksheet *ws, WorkbookStatus *status);
			static UOSInt ReadUString(UInt8 *buff, Text::StringBuilderUTF *sb);
			static UOSInt ReadUStringPartial(UInt8 *buff, UOSInt buffSize, UInt32 *charCnt, Text::StringBuilderUTF *sb);
			static UOSInt ReadUStringB(UInt8 *buff, Text::StringBuilderUTF *sb);
			static Double ParseRKNumber(Int32 rkValue);
		};
	}
}
#endif
