#ifndef _SM_PARSER_FILEPARSER_NOAPARSER
#define _SM_PARSER_FILEPARSER_NOAPARSER
#include "IO/IFileParser.h"

namespace Parser
{
	namespace FileParser
	{
		class NOAParser : public IO::IFileParser
		{
		public:
			NOAParser();
			virtual ~NOAParser();

			virtual Int32 GetName();
			virtual void PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t);
			virtual IO::ParsedObject::ParserType GetParserType();
			virtual IO::ParsedObject *ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType);
		};
	};
};
#endif