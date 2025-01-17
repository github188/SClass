#include "Stdafx.h"
#include "MyMemory.h"
#include "Crypto/Hash/CRC32R.h"
#include "Data/ByteTool.h"
#include "Text/MyString.h"

extern "C"
{
	void CRC32R_InitTable(UInt32 *tab, UInt32 rpn);
	UInt32 CRC32R_Calc(const UInt8 *buff, UOSInt buffSize, UInt32 *tab, UInt32 currVal);
}

Crypto::Hash::CRC32R::CRC32R(const CRC32R *crc)
{
	this->crctab = MemAlloc(UInt32, 256*16);
	this->currVal = crc->currVal;
	MemCopyNO(this->crctab, crc->crctab, sizeof(UInt32) * 256*16);
}

void Crypto::Hash::CRC32R::InitTable(UInt32 polynomial)
{
	currVal = 0xffffffff;

	UInt32 rpn = CRC32R_Reverse(polynomial);
	UInt32 *tab = crctab = MemAlloc(UInt32, 256*16);
	CRC32R_InitTable(tab, rpn);
}

Crypto::Hash::CRC32R::CRC32R()
{
	InitTable(GetPolynormialIEEE());
}

Crypto::Hash::CRC32R::CRC32R(UInt32 polynomial)
{
	InitTable(polynomial);
}

Crypto::Hash::CRC32R::~CRC32R()
{
	MemFree(crctab);
}

UTF8Char *Crypto::Hash::CRC32R::GetName(UTF8Char *sbuff)
{
	return Text::StrConcat(sbuff, (const UTF8Char*)"CRC (32-bit Reversed)");
}

Crypto::Hash::IHash *Crypto::Hash::CRC32R::Clone()
{
	Crypto::Hash::CRC32R *crc;
	NEW_CLASS(crc, Crypto::Hash::CRC32R(this));
	return crc;
}

void Crypto::Hash::CRC32R::Clear()
{
	currVal = 0xffffffff;
}

void Crypto::Hash::CRC32R::Calc(const UInt8 *buff, UOSInt buffSize)
{
	this->currVal = CRC32R_Calc(buff, buffSize, this->crctab, this->currVal);
}

void Crypto::Hash::CRC32R::GetValue(UInt8 *buff)
{
	WriteMUInt32(buff, ~currVal);
}

UOSInt Crypto::Hash::CRC32R::GetBlockSize()
{
	return 1;
}

UOSInt Crypto::Hash::CRC32R::GetResultSize()
{
	return 4;
}

UInt32 Crypto::Hash::CRC32R::GetPolynormialIEEE()
{
	return 0x04C11DB7;
//	return 0xedb88320;
}
