#ifndef _SM_PARSER_FILEPARSER_PSSPARSER
#define _SM_PARSER_FILEPARSER_PSSPARSER
#include "IO/FileParser.h"

namespace Parser
{
	namespace FileParser
	{
		class PSSParser : public IO::FileParser
		{
		public:
			PSSParser();
			virtual ~PSSParser();

			virtual Int32 GetName();
			virtual void PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t);
			virtual IO::ParsedObject::ParserType GetParserType();
			virtual IO::ParsedObject *ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType);
		};
	};
};
#endif
