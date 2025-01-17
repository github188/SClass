#include "Stdafx.h"
#include "MyMemory.h"
#include "Core/Core.h"
#include "Crypto/Hash/CRC32R.h"
#include "IO/ConsoleWriter.h"
#include "Text/MyString.h"

Int32 MyMain(Core::IProgControl *progCtrl)
{
	UInt8 testBlock[] = "123456789";
	UInt8 testBlock2[] = "12345678901234567";
	UInt8 hashVal[32];
	UTF8Char sbuff[65];
	Crypto::Hash::IHash *hash;
	IO::ConsoleWriter console;

	NEW_CLASS(hash, Crypto::Hash::CRC32R());

	hash->GetValue(hashVal);
	Text::StrHexBytes(sbuff, hashVal, 4, 0);
	console.WriteLine(sbuff);
	console.WriteLine((const UTF8Char*)"00000000");
	console.WriteLine();

	hash->Calc(testBlock, sizeof(testBlock) - 1);
	hash->GetValue(hashVal);
	Text::StrHexBytes(sbuff, hashVal, 4, 0);
	console.WriteLine(sbuff);
	console.WriteLine((const UTF8Char*)"CBF43926");
	console.WriteLine();

	hash->Clear();
	hash->Calc(testBlock2, sizeof(testBlock2) - 1);
	hash->GetValue(hashVal);
	Text::StrHexBytes(sbuff, hashVal, 4, 0);
	console.WriteLine(sbuff);
	console.WriteLine((const UTF8Char*)"3FA43360");

	DEL_CLASS(hash);
	return 0;
}
