#include "Stdafx.h"

extern "C" void CRC16_InitTable(UInt16 *tab, UInt16 polynomial)
{
	Int32 i = 256;
	Int32 j;
	UInt16 v;
	while (i-- > 0)
	{
		v = i << 8;
		j = 8;
		while (j-- > 0)
		{
			if (v & 0x8000)
			{
				v = (v << 1) ^ polynomial;
			}
			else
			{
				v = (v << 1);
			}
		}
		tab[i] = v;
	}

	i = 0;
	while (i < 256)
	{
		tab[256  + i] = (tab[0    + i] << 8) ^ tab[tab[0    + i] >> 8];
		tab[512  + i] = (tab[256  + i] << 8) ^ tab[tab[256  + i] >> 8];
		tab[768  + i] = (tab[512  + i] << 8) ^ tab[tab[512  + i] >> 8];
		tab[1024 + i] = (tab[768  + i] << 8) ^ tab[tab[768  + i] >> 8];
		tab[1280 + i] = (tab[1024 + i] << 8) ^ tab[tab[1024 + i] >> 8];
		tab[1536 + i] = (tab[1280 + i] << 8) ^ tab[tab[1280 + i] >> 8];
		tab[1792 + i] = (tab[1536 + i] << 8) ^ tab[tab[1536 + i] >> 8];
		tab[2048 + i] = (tab[1792 + i] << 8) ^ tab[tab[1792 + i] >> 8];
		tab[2304 + i] = (tab[2048 + i] << 8) ^ tab[tab[2048 + i] >> 8];
		tab[2560 + i] = (tab[2304 + i] << 8) ^ tab[tab[2304 + i] >> 8];
		tab[2816 + i] = (tab[2560 + i] << 8) ^ tab[tab[2560 + i] >> 8];
		tab[3072 + i] = (tab[2816 + i] << 8) ^ tab[tab[2816 + i] >> 8];
		tab[3328 + i] = (tab[3072 + i] << 8) ^ tab[tab[3072 + i] >> 8];
		tab[3584 + i] = (tab[3328 + i] << 8) ^ tab[tab[3328 + i] >> 8];
		tab[3840 + i] = (tab[3584 + i] << 8) ^ tab[tab[3584 + i] >> 8];
		i++;
	}
}

extern "C" UInt16 CRC16_Calc(const UInt8 *buff, UOSInt buffSize, UInt16 *tab, UInt16 currVal)
{
	UInt16 v = currVal;

	while (buffSize >= 16)
	{
		v = tab[   0 + buff[15]] ^
			tab[ 256 + buff[14]] ^
			tab[ 512 + buff[13]] ^
			tab[ 768 + buff[12]] ^
			tab[1024 + buff[11]] ^
			tab[1280 + buff[10]] ^
			tab[1536 + buff[9]] ^
			tab[1792 + buff[8]] ^
			tab[2048 + buff[7]] ^
			tab[2304 + buff[6]] ^
			tab[2560 + buff[5]] ^
			tab[2816 + buff[4]] ^
			tab[3072 + buff[3]] ^
			tab[3328 + buff[2]] ^
			tab[3584 + (buff[1] ^ (v & 0xff))] ^
			tab[3840 + (buff[0] ^ (v >> 8))];
		buff += 16;
		buffSize -= 16;
	}
	while (buffSize--)
	{
		v = tab[(v >> 8) ^ (*buff)] ^ (v << 8);
		buff++;
	}

	return v;
}