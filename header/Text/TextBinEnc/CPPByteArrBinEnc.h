#ifndef _SM_TEXT_TEXTBINENC_CPPBYTEARRBINENC
#define _SM_TEXT_TEXTBINENC_CPPBYTEARRBINENC
#include "Text/TextBinEnc/ITextBinEnc.h"

namespace Text
{
	namespace TextBinEnc
	{
		class CPPByteArrBinEnc : public Text::TextBinEnc::ITextBinEnc
		{
		public:
			CPPByteArrBinEnc();
			virtual ~CPPByteArrBinEnc();
			virtual UOSInt EncodeBin(Text::StringBuilderUTF *sb, const UInt8 *dataBuff, UOSInt buffSize);
			virtual UOSInt CalcBinSize(const UTF8Char *sbuff);
			virtual UOSInt DecodeBin(const UTF8Char *sbuff, UInt8 *dataBuff);
			virtual const UTF8Char *GetName();
		};
	}
}
#endif