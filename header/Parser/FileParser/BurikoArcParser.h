#ifndef _SM_PARSER_FILEPARSER_BURIKOARCPARSER
#define _SM_PARSER_FILEPARSER_BURIKOARCPARSER
#include "IO/IFileParser.h"

namespace Parser
{
	namespace FileParser
	{
		class BurikoArcParser : public IO::IFileParser
		{
		public:
			BurikoArcParser();
			virtual ~BurikoArcParser();

			virtual Int32 GetName();
			virtual void PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t);
			virtual IO::ParsedObject::ParserType GetParserType();
			virtual IO::ParsedObject *ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType);
		};
	};
};
#endif