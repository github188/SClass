#ifndef _SM_PARSER_FILEPARSER_MD5PARSER
#define _SM_PARSER_FILEPARSER_MD5PARSER
#include "IO/IFileParser.h"

namespace Parser
{
	namespace FileParser
	{
		class MD5Parser : public IO::IFileParser
		{
		private:
			Int32 codePage;
		public:
			MD5Parser();
			virtual ~MD5Parser();

			virtual Int32 GetName();
			virtual void SetCodePage(Int32 codePage);
			virtual void PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t);
			virtual IO::ParsedObject::ParserType GetParserType();
			virtual IO::ParsedObject *ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType);
		};
	};
};
#endif