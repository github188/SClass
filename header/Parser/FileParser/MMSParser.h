#ifndef _SM_PARSER_FILEPARSER_MMSPARSER
#define _SM_PARSER_FILEPARSER_MMSPARSER
#include "IO/FileParser.h"

namespace Parser
{
	namespace FileParser
	{
		class MMSParser : public IO::FileParser
		{
		public:
			MMSParser();
			virtual ~MMSParser();

			virtual Int32 GetName();
			virtual void PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t);
			virtual IO::ParsedObject::ParserType GetParserType();
			virtual IO::ParsedObject *ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType);
		};
	};
};
#endif
