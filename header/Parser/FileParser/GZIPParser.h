#ifndef _SM_PARSER_FILEPARSER_GZIPPARSER
#define _SM_PARSER_FILEPARSER_GZIPPARSER
#include "IO/FileParser.h"

namespace Parser
{
	namespace FileParser
	{
		class GZIPParser : public IO::FileParser
		{
		public:
			GZIPParser();
			virtual ~GZIPParser();

			virtual Int32 GetName();
			virtual void PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t);
			virtual IO::ParsedObject::ParserType GetParserType();
			virtual IO::ParsedObject *ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType);
		};
	};
};
#endif
