#ifndef _SM_PARSER_FILEPARSER_PAC2PARSER
#define _SM_PARSER_FILEPARSER_PAC2PARSER
#include "IO/FileParser.h"

namespace Parser
{
	namespace FileParser
	{
		class PAC2Parser : public IO::FileParser
		{
		public:
			PAC2Parser();
			virtual ~PAC2Parser();

			virtual Int32 GetName();
			virtual void PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t);
			virtual IO::ParsedObject::ParserType GetParserType();
			virtual IO::ParsedObject *ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType);
		};
	};
};
#endif
