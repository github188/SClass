#ifndef _SM_PARSER_FILEPARSER_HTRECPARSER
#define _SM_PARSER_FILEPARSER_HTRECPARSER
#include "IO/IFileParser.h"

namespace Parser
{
	namespace FileParser
	{
		class HTRecParser : public IO::IFileParser
		{
		public:
			HTRecParser();
			virtual ~HTRecParser();

			virtual Int32 GetName();
			virtual void PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t);
			virtual IO::ParsedObject::ParserType GetParserType();
			virtual IO::ParsedObject *ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType);
		};
	};
};
#endif