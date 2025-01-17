#ifndef _SM_CRYPTO_HOTP
#define _SM_CRYPTO_HOTP
#include "Crypto/OTP.h"

namespace Crypto
{
	class HOTP : public OTP
	{
	private:
		UInt8 *key;
		UOSInt keySize;
		UInt64 counter;

	public:
		HOTP(const UInt8 *key, UOSInt keySize, UInt64 counter);
		virtual ~HOTP();

		virtual OTPType GetType();
		virtual UInt64 GetCounter();
		virtual UInt32 NextCode();
		virtual Bool IsValid(UInt32 code);
		virtual void GenURI(Text::StringBuilderUTF *sb, const UTF8Char *name);

		static UInt32 CalcCode(const UInt8 *key, UOSInt keySize, UInt64 counter, UInt32 nDigits);
	};
}
#endif
