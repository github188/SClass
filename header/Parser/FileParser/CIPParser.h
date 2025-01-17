#ifndef _SM_PARSER_FILEPARSER_CIPPARSER
#define _SM_PARSER_FILEPARSER_CIPPARSER
#include "IO/FileParser.h"

namespace Parser
{
	namespace FileParser
	{
		class CIPParser : public IO::FileParser
		{
		public:
			CIPParser();
			virtual ~CIPParser();

			virtual Int32 GetName();
			virtual void PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t);
			virtual IO::ParsedObject::ParserType GetParserType();
			virtual IO::ParsedObject *ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType);
		};
	};
};
#endif
