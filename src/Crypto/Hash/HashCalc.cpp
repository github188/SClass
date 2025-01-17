#include "Stdafx.h"
#include "MyMemory.h"
#include "Crypto/Hash/HashCalc.h"
#include "Sync/MutexUsage.h"
#include "Text/MyString.h"

Crypto::Hash::HashCalc::HashCalc(Crypto::Hash::IHash *hash)
{
	this->hash = hash;
	NEW_CLASS(this->mut, Sync::Mutex());
}

Crypto::Hash::HashCalc::~HashCalc()
{
	DEL_CLASS(this->mut);
	DEL_CLASS(this->hash);
}

void Crypto::Hash::HashCalc::Calc(const UInt8 *buff, UOSInt size, UInt8 *hashVal)
{
	Sync::MutexUsage mutUsage(this->mut);
	this->hash->Clear();
	this->hash->Calc(buff, size);
	this->hash->GetValue(hashVal);
}

void Crypto::Hash::HashCalc::CalcStr(const UTF8Char *s, UInt8 *hashVal)
{
	this->Calc(s, Text::StrCharCnt(s), hashVal);
}
