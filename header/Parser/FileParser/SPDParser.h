#ifndef _SM_PARSER_FILEPARSER_SPDPARSER
#define _SM_PARSER_FILEPARSER_SPDPARSER
#include "IO/FileParser.h"

namespace Parser
{
	namespace FileParser
	{
		class SPDParser : public IO::FileParser
		{
		public:
			SPDParser();
			virtual ~SPDParser();

			virtual Int32 GetName();
			virtual void PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t);
			virtual IO::ParsedObject::ParserType GetParserType();
			virtual IO::ParsedObject *ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType);
		};
	};
};
#endif
