#ifndef _SM_PARSER_FILEPARSER_ISOPARSER
#define _SM_PARSER_FILEPARSER_ISOPARSER
#include "IO/FileParser.h"

namespace Parser
{
	namespace FileParser
	{
		class ISOParser : public IO::FileParser
		{
		private:
			ParserList *parsers;

		public:
			ISOParser();
			virtual ~ISOParser();

			virtual Int32 GetName();
			virtual void SetParserList(Parser::ParserList *parsers);
			virtual void PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t);
			virtual IO::ParsedObject::ParserType GetParserType();
			virtual IO::ParsedObject *ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType);
		};
	};
};
#endif
