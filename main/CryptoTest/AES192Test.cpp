#include "Stdafx.h"
#include "MyMemory.h"
#include "Core/Core.h"
#include "Crypto/Encrypt/AES192.h"
#include "IO/ConsoleWriter.h"
#include "Text/StringBuilderUTF8.h"

Int32 MyMain(Core::IProgControl *progCtrl)
{
	Text::StringBuilderUTF8 sb;
	IO::ConsoleWriter console;
	Crypto::Encrypt::AES192 *aes;
	UInt8 key[24] = {0x8e, 0x73, 0xb0, 0xf7, 0xda, 0x0e, 0x64, 0x52, 0xc8, 0x10, 0xf3, 0x2b, 0x80, 0x90, 0x79, 0xe5, 0x62, 0xf8, 0xea, 0xd2, 0x52, 0x2c, 0x6b, 0x7b};
	UInt8 testVector1[16] = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};
	UInt8 testVector2[16] = {0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51};
	UInt8 testVector3[16] = {0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef};
	UInt8 testVector4[16] = {0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10};
	NEW_CLASS(aes, Crypto::Encrypt::AES192(key));
	UInt8 cipherText[16];
	UInt8 decText[16];
	
	sb.ClearStr();
	sb.Append((const UTF8Char*)"Encryption key: ");
	sb.AppendHexBuff(key, 16, 0, Text::LBT_NONE);
	console.WriteLine(sb.ToString());
	console.WriteLine();

	sb.ClearStr();
	sb.Append((const UTF8Char*)"Test vector: ");
	sb.AppendHexBuff(testVector1, 16, 0, Text::LBT_NONE);
	console.WriteLine(sb.ToString());

	aes->Encrypt(testVector1, 16, cipherText, 0);
	sb.ClearStr();
	sb.Append((const UTF8Char*)"Cipher text: ");
	sb.AppendHexBuff(cipherText, 16, 0, Text::LBT_NONE);
	console.WriteLine(sb.ToString());

	sb.ClearStr();
	sb.Append((const UTF8Char*)"Expected:    ");
	sb.Append((const UTF8Char*)"bd334f1d6e45f25ff712a214571fa5cc");
	console.WriteLine(sb.ToString());

	aes->Decrypt(cipherText, 16, decText, 0);
	sb.ClearStr();
	sb.Append((const UTF8Char*)"DecryptText: ");
	sb.AppendHexBuff(decText, 16, 0, Text::LBT_NONE);
	console.WriteLine(sb.ToString());
	console.WriteLine();

	sb.ClearStr();
	sb.Append((const UTF8Char*)"Test vector: ");
	sb.AppendHexBuff(testVector2, 16, 0, Text::LBT_NONE);
	console.WriteLine(sb.ToString());

	aes->SetKey(key);
	aes->Encrypt(testVector2, 16, cipherText, 0);
	sb.ClearStr();
	sb.Append((const UTF8Char*)"Cipher text: ");
	sb.AppendHexBuff(cipherText, 16, 0, Text::LBT_NONE);
	console.WriteLine(sb.ToString());

	sb.ClearStr();
	sb.Append((const UTF8Char*)"Expected:    ");
	sb.Append((const UTF8Char*)"974104846d0ad3ad7734ecb3ecee4eef");
	console.WriteLine(sb.ToString());

	aes->Decrypt(cipherText, 16, decText, 0);
	sb.ClearStr();
	sb.Append((const UTF8Char*)"DecryptText: ");
	sb.AppendHexBuff(decText, 16, 0, Text::LBT_NONE);
	console.WriteLine(sb.ToString());
	console.WriteLine();

	sb.ClearStr();
	sb.Append((const UTF8Char*)"Test vector: ");
	sb.AppendHexBuff(testVector3, 16, 0, Text::LBT_NONE);
	console.WriteLine(sb.ToString());

	aes->SetKey(key);
	aes->Encrypt(testVector3, 16, cipherText, 0);
	sb.ClearStr();
	sb.Append((const UTF8Char*)"Cipher text: ");
	sb.AppendHexBuff(cipherText, 16, 0, Text::LBT_NONE);
	console.WriteLine(sb.ToString());

	sb.ClearStr();
	sb.Append((const UTF8Char*)"Expected:    ");
	sb.Append((const UTF8Char*)"ef7afd2270e2e60adce0ba2face6444e");
	console.WriteLine(sb.ToString());

	aes->Decrypt(cipherText, 16, decText, 0);
	sb.ClearStr();
	sb.Append((const UTF8Char*)"DecryptText: ");
	sb.AppendHexBuff(decText, 16, 0, Text::LBT_NONE);
	console.WriteLine(sb.ToString());
	console.WriteLine();

	sb.ClearStr();
	sb.Append((const UTF8Char*)"Test vector: ");
	sb.AppendHexBuff(testVector4, 16, 0, Text::LBT_NONE);
	console.WriteLine(sb.ToString());

	aes->SetKey(key);
	aes->Encrypt(testVector4, 16, cipherText, 0);
	sb.ClearStr();
	sb.Append((const UTF8Char*)"Cipher text: ");
	sb.AppendHexBuff(cipherText, 16, 0, Text::LBT_NONE);
	console.WriteLine(sb.ToString());

	sb.ClearStr();
	sb.Append((const UTF8Char*)"Expected:    ");
	sb.Append((const UTF8Char*)"9a4b41ba738d6c72fb16691603c18e0e");
	console.WriteLine(sb.ToString());

	aes->Decrypt(cipherText, 16, decText, 0);
	sb.ClearStr();
	sb.Append((const UTF8Char*)"DecryptText: ");
	sb.AppendHexBuff(decText, 16, 0, Text::LBT_NONE);
	console.WriteLine(sb.ToString());
	console.WriteLine();

	DEL_CLASS(aes);
	return 0;
}
