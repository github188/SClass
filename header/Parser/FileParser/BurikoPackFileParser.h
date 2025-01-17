#ifndef _SM_PARSER_FILEPARSER_BURIKOPACKFILEPARSER
#define _SM_PARSER_FILEPARSER_BURIKOPACKFILEPARSER
#include "IO/FileParser.h"

namespace Parser
{
	namespace FileParser
	{
		class BurikoPackFileParser : public IO::FileParser
		{
		public:
			BurikoPackFileParser();
			virtual ~BurikoPackFileParser();

			virtual Int32 GetName();
			virtual void PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t);
			virtual IO::ParsedObject::ParserType GetParserType();
			virtual IO::ParsedObject *ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType);
		};
	};
};
#endif
