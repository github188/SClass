#ifndef _SM_MEDIA_CODEIMAGEGEN_EAN8CODEIMAGEGEN
#define _SM_MEDIA_CODEIMAGEGEN_EAN8CODEIMAGEGEN
#include "Media/CodeImageGen/CodeImageGen.h"

namespace Media
{
	namespace CodeImageGen
	{
		class EAN8CodeImageGen : public Media::CodeImageGen::CodeImageGen
		{
		public:
			EAN8CodeImageGen();
			virtual ~EAN8CodeImageGen();

			virtual CodeType GetCodeType();
			virtual OSInt GetMinLength();
			virtual OSInt GetMaxLength();
			virtual Media::DrawImage *GenCode(const UTF8Char *code, OSInt codeWidth, Media::DrawEngine *eng);
		};
	};
};
#endif