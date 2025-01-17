#include "Stdafx.h"
#include "MyMemory.h"
#include "Crypto/Encrypt/BlockCipher.h"

Crypto::Encrypt::BlockCipher::BlockCipher(UOSInt blockSize)
{
	this->blockSize = blockSize;
	this->cm = CM_ECB;
	this->iv = MemAlloc(UInt8, blockSize);
	MemClear(this->iv, this->blockSize);
}

Crypto::Encrypt::BlockCipher::~BlockCipher()
{
	MemFree(this->iv);
}

UOSInt Crypto::Encrypt::BlockCipher::Encrypt(const UInt8 *inBuff, UOSInt inSize, UInt8 *outBuff, void *encParam)
{
	UInt8 *blk;
	UOSInt blkCnt = 0;
	switch (this->cm)
	{
	case CM_ECB:
		while (inSize >= this->blockSize)
		{
			EncryptBlock(inBuff, outBuff, encParam);
			blkCnt++;
			inBuff += this->blockSize;
			outBuff += this->blockSize;
			inSize = inSize - this->blockSize;
		}
		if (inSize > 0)
		{
			blk = MemAlloc(UInt8, this->blockSize);
			MemClear(blk, this->blockSize);
			MemCopyNO(blk, inBuff, inSize);
			EncryptBlock(blk, outBuff, encParam);
			blkCnt++;
			MemFree(blk);
		}
		return blkCnt * this->blockSize;
	case CM_CBC:
		blk = MemAlloc(UInt8, this->blockSize);
		MemCopyNO(blk, this->iv, this->blockSize);
		while (inSize >= blockSize)
		{
			MemXOR(blk, inBuff, blk, this->blockSize);
			EncryptBlock(blk, outBuff, encParam);
			MemCopyNO(blk, outBuff, this->blockSize);
			blkCnt++;
			inBuff += this->blockSize;
			outBuff += this->blockSize;
			inSize = inSize - this->blockSize;
		}
		if (inSize > 0)
		{
			MemXOR(blk, inBuff, blk, inSize);
			EncryptBlock(blk, outBuff, encParam);
			blkCnt++;
		}
		MemFree(blk);
		return blkCnt * this->blockSize;
	case CM_PCBC:
		blk = MemAlloc(UInt8, this->blockSize);
		MemCopyNO(blk, this->iv, this->blockSize);
		while (inSize >= blockSize)
		{
			MemXOR(blk, inBuff, blk, this->blockSize);
			EncryptBlock(inBuff, outBuff, encParam);
			MemXOR(inBuff, outBuff, blk, this->blockSize);
			blkCnt++;
			inBuff += this->blockSize;
			outBuff += this->blockSize;
			inSize = inSize - this->blockSize;
		}
		if (inSize > 0)
		{
			MemXOR(blk, inBuff, blk, inSize);
			EncryptBlock(inBuff, outBuff, encParam);
			blkCnt++;
		}
		MemFree(blk);
		return blkCnt * this->blockSize;
	case CM_CFB:
		blk = MemAlloc(UInt8, this->blockSize);
		MemCopyNO(blk, this->iv, this->blockSize);
		while (inSize >= blockSize)
		{
			EncryptBlock(blk, outBuff, encParam);
			MemXOR(outBuff, inBuff, outBuff, this->blockSize);
			MemCopyNO(blk, outBuff, this->blockSize);
			blkCnt++;
			inBuff += this->blockSize;
			outBuff += this->blockSize;
			inSize = inSize - this->blockSize;
		}
		if (inSize > 0)
		{
			EncryptBlock(blk, outBuff, encParam);
			MemXOR(outBuff, inBuff, outBuff, inSize);
			blkCnt++;
		}
		MemFree(blk);
		return blkCnt * this->blockSize;
	case CM_OFB:
		blk = MemAlloc(UInt8, this->blockSize);
		MemCopyNO(blk, this->iv, this->blockSize);
		while (inSize >= blockSize)
		{
			EncryptBlock(blk, outBuff, encParam);
			MemCopyNO(blk, outBuff, this->blockSize);
			MemXOR(outBuff, inBuff, outBuff, this->blockSize);
			blkCnt++;
			inBuff += this->blockSize;
			outBuff += this->blockSize;
			inSize = inSize - this->blockSize;
		}
		if (inSize > 0)
		{
			EncryptBlock(blk, outBuff, encParam);
			MemXOR(outBuff, inBuff, outBuff, inSize);
			blkCnt++;
		}
		MemFree(blk);
		return blkCnt * this->blockSize;
	default:
		return 0;
	}
}

UOSInt Crypto::Encrypt::BlockCipher::Decrypt(const UInt8 *inBuff, UOSInt inSize, UInt8 *outBuff, void *decParam)
{
	UInt8 *blk;
	UOSInt blkCnt = 0;
	switch (this->cm)
	{
	case CM_ECB:
		while (inSize >= this->blockSize)
		{
			DecryptBlock(inBuff, outBuff, decParam);
			blkCnt++;
			inBuff += this->blockSize;
			outBuff += this->blockSize;
			inSize = inSize - this->blockSize;
		}
		return blkCnt * this->blockSize;
	case CM_CBC:
		blk = MemAlloc(UInt8, this->blockSize);
		MemCopyNO(blk, this->iv, this->blockSize);
		while (inSize >= this->blockSize)
		{
			DecryptBlock(inBuff, outBuff, decParam);
			blkCnt++;
			MemXOR(outBuff, blk, outBuff, this->blockSize);
			MemCopyNO(blk, inBuff, this->blockSize);
			inBuff += this->blockSize;
			outBuff += this->blockSize;
			inSize = inSize - this->blockSize;
		}
		MemFree(blk);
		return blkCnt * this->blockSize;
	case CM_PCBC:
		blk = MemAlloc(UInt8, this->blockSize);
		MemCopyNO(blk, this->iv, this->blockSize);
		while (inSize >= this->blockSize)
		{
			DecryptBlock(inBuff, outBuff, decParam);
			blkCnt++;
			MemXOR(outBuff, blk, outBuff, this->blockSize);
			MemXOR(inBuff, outBuff, blk, this->blockSize);
			inBuff += this->blockSize;
			outBuff += this->blockSize;
			inSize = inSize - this->blockSize;
		}
		MemFree(blk);
		return blkCnt * this->blockSize;
	case CM_CFB:
		blk = MemAlloc(UInt8, this->blockSize);
		MemCopyNO(blk, this->iv, this->blockSize);
		while (inSize >= this->blockSize)
		{
			EncryptBlock(blk, outBuff, decParam);
			blkCnt++;
			MemXOR(outBuff, inBuff, outBuff, this->blockSize);
			MemCopyNO(blk, inBuff, this->blockSize);
			inBuff += this->blockSize;
			outBuff += this->blockSize;
			inSize = inSize - this->blockSize;
		}
		MemFree(blk);
		return blkCnt * this->blockSize;
	case CM_OFB:
		blk = MemAlloc(UInt8, this->blockSize);
		MemCopyNO(blk, this->iv, this->blockSize);
		while (inSize >= this->blockSize)
		{
			EncryptBlock(blk, outBuff, decParam);
			blkCnt++;
			MemCopyNO(blk, outBuff, this->blockSize);
			MemXOR(outBuff, inBuff, outBuff, this->blockSize);
			inBuff += this->blockSize;
			outBuff += this->blockSize;
			inSize = inSize - this->blockSize;
		}
		MemFree(blk);
		return blkCnt * this->blockSize;
	default:
		return 0;
	}
}

UOSInt Crypto::Encrypt::BlockCipher::GetEncBlockSize()
{
	return this->blockSize;
}

UOSInt Crypto::Encrypt::BlockCipher::GetDecBlockSize()
{
	return this->blockSize;
}

void Crypto::Encrypt::BlockCipher::SetChainMode(ChainMode cm)
{
	this->cm = cm;
}

void Crypto::Encrypt::BlockCipher::SetIV(const UInt8 *iv)
{
	MemCopyNO(this->iv, iv, this->blockSize);
}
