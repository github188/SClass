#ifndef _SM_CRYPTO_OTP
#define _SM_CRYPTO_OTP
#include "Text/StringBuilderUTF.h"

namespace Crypto
{
	class OTP
	{
	public:
		typedef enum
		{
			OT_HOTP,
			OT_TOTP
		} OTPType;

	protected:
		UInt32 nDigits;

	public:
		OTP(UInt32 nDigits);
		virtual ~OTP();

		virtual OTPType GetType() = 0;
		virtual UInt64 GetCounter() = 0;
		virtual UInt32 NextCode() = 0;
		virtual Bool IsValid(UInt32 code) = 0;
		virtual void GenURI(Text::StringBuilderUTF *sb, const UTF8Char *name) = 0;

		UTF8Char *CodeString(UTF8Char *sbuff, UInt32 code);
		static const UTF8Char *OTPTypeGetName(OTPType type);
	};
}
#endif
