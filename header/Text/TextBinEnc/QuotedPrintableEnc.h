#ifndef _SM_TEXT_TEXTBINENC_QUOTEDPRINTABLEENC
#define _SM_TEXT_TEXTBINENC_QUOTEDPRINTABLEENC
#include "Text/TextBinEnc/ITextBinEnc.h"

namespace Text
{
	namespace TextBinEnc
	{
		class QuotedPrintableEnc : public Text::TextBinEnc::ITextBinEnc
		{
		public:
			QuotedPrintableEnc();
			virtual ~QuotedPrintableEnc();
			virtual UOSInt EncodeBin(Text::StringBuilderUTF *sb, const UInt8 *dataBuff, UOSInt buffSize);
			virtual UOSInt CalcBinSize(const UTF8Char *b64Str);
			virtual UOSInt DecodeBin(const UTF8Char *b64Str, UInt8 *dataBuff);
			UOSInt DecodeBin(const UTF8Char *b64Str, UOSInt len, UInt8 *dataBuff);
			virtual const UTF8Char *GetName();
		};
	}
}
#endif
