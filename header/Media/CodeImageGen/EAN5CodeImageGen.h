#ifndef _SM_MEDIA_CODEIMAGEGEN_EAN5CODEIMAGEGEN
#define _SM_MEDIA_CODEIMAGEGEN_EAN5CODEIMAGEGEN
#include "Media/CodeImageGen/CodeImageGen.h"

namespace Media
{
	namespace CodeImageGen
	{
		class EAN5CodeImageGen : public Media::CodeImageGen::CodeImageGen
		{
		public:
			EAN5CodeImageGen();
			virtual ~EAN5CodeImageGen();

			virtual CodeType GetCodeType();
			virtual OSInt GetMinLength();
			virtual OSInt GetMaxLength();
			virtual Media::DrawImage *GenCode(const UTF8Char *code, OSInt codeWidth, Media::DrawEngine *eng);
		};
	};
};
#endif