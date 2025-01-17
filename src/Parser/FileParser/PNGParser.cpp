#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ByteTool.h"
#include "Data/Compress/InflateStream.h"
#include "IO/MemoryStream.h"
#include "IO/WriteCacheStream.h"
#include "Media/ICCProfile.h"
#include "Media/StaticImage.h"
#include "Math/Unit/Distance.h"
#include "Parser/FileParser/PNGParser.h"

UInt8 PNGParser_PaethPredictor(UInt8 a, UInt8 b, UInt8 c)
{
	Int32 p = a + (Int32)b - (Int32)c;
	Int32 pa = p - a;
	Int32 pb = p - b;
	Int32 pc = p - c;
	if (pa < 0)
		pa = -pa;
	if (pb < 0)
		pb = -pb;
	if (pc < 0)
		pc = -pc;
	if (pa <= pb && pa <= pc)
	{
		return a;
	}
	else if (pb <= pc)
	{
		return b;
	}
	else
	{
		return c;
	}
}

UInt8 *PNGParser_ParsePixelsBits(UInt8 *srcData, UInt8 *destBuff, UOSInt bpl, UOSInt initX, UOSInt initY, UOSInt maxX, UOSInt maxY, UOSInt xAdd, UOSInt yAdd, OSInt pxMask, OSInt pxAMask, OSInt pxShift)
{
	UInt8 *lineStart = destBuff + initY * bpl;
	UOSInt currX;
	UOSInt currY;
	OSInt pxId;
	UInt8 px;
	currY = initY;
	while (currY < maxY)
	{
		currX = initX;
		pxId = 0;
		px = *srcData++;

		if (px == 0)
		{
			while (currX < maxX)
			{
				if ((pxId & pxMask) == 0)
				{
					px = *srcData++;
				}
				lineStart[currX] = (UInt8)((px >> (8 - pxShift)) & pxAMask);
				px = (UInt8)(px << pxShift);
				pxId++;
				currX += xAdd;
			}
		}
		else
		{
//			printf("%d, Line type = %d\r\n", px);
//			sb.AppendHexBuff(srcData, (maxX - currX) * pxShift / 8, ' ', Text::LBT_CRLF);
//			printf("%s\r\n", sb.ToString());
			while (currX < maxX)
			{
				if ((pxId & pxMask) == 0)
				{
					px = *srcData++;
				}
				lineStart[currX] = (UInt8)((px >> (8 - pxShift)) & pxAMask);
				px = (UInt8)(px << pxShift);
				pxId++;
				currX += xAdd;
			}
		}
		currY += yAdd;
		lineStart += bpl * yAdd;
	}
	return srcData;
}

UInt8 *PNGParser_ParsePixelsByte(UInt8 *srcData, UInt8 *destBuff, UOSInt bpl, UOSInt initX, UOSInt initY, UOSInt maxX, UOSInt maxY, UOSInt xAdd, UOSInt yAdd)
{
	UInt8 *lineStart = destBuff + initY * bpl;
	UOSInt currX;
	UOSInt currY;
	UInt8 px;
	UInt8 *lastLineStart;
	UInt8 lastPx;
	lastLineStart = 0;
	currY = initY;
	while (currY < maxY)
	{
		currX = initX;
		px = *srcData++;
		if (px == 0)
		{
			while (currX < maxX)
			{
				lineStart[currX] = *srcData++;
				currX += xAdd;
			}
		}
		else if (px == 1)
		{
			lastPx = 0;
			while (currX < maxX)
			{
				lastPx = (UInt8)(lastPx + *srcData++);
				lineStart[currX] = lastPx;
				currX += xAdd;
			}
		}
		else if (px == 2)
		{
			if (lastLineStart)
			{
				while (currX < maxX)
				{
					lineStart[currX] = (UInt8)(lastLineStart[currX] + *srcData++);
					currX += xAdd;
				}
			}
			else
			{
				while (currX < maxX)
				{
					lineStart[currX] = *srcData++;
					currX += xAdd;
				}
			}
		}
		else if (px == 3)
		{
			if (lastLineStart)
			{
				lastPx = 0;
				while (currX < maxX)
				{
					lastPx = (UInt8)(((lastPx + lastLineStart[currX]) >> 1) + *srcData++);
					lineStart[currX] = lastPx;
					currX += xAdd;
				}
			}
			else
			{
				lastPx = 0;
				while (currX < maxX)
				{
					lastPx = (UInt8)((lastPx >> 1) + *srcData++);
					lineStart[currX] = lastPx;
					currX += xAdd;
				}
			}
		}
		else if (px == 4)
		{
			if (lastLineStart)
			{
				lastPx = 0;
				while (currX < maxX)
				{
					if (currX < xAdd)
					{
						lastPx = (UInt8)(lastLineStart[currX] + *srcData++);
					}
					else
					{
						lastPx = (UInt8)(PNGParser_PaethPredictor(lastPx, lastLineStart[currX], lastLineStart[currX - xAdd]) + *srcData++);
					}
					lineStart[currX] = lastPx;
					currX += xAdd;
				}
			}
			else
			{
				lastPx = 0;
				while (currX < maxX)
				{
					if (currX < xAdd)
					{
						lastPx = *srcData++;
					}
					else
					{
						lastPx = (UInt8)(lastPx + *srcData++);
					}
					lineStart[currX] = lastPx;
					currX += xAdd;
				}
			}
		}
		else
		{
			while (currX < maxX)
			{
				lineStart[currX] = *srcData++;
				currX += xAdd;
			}
		}
		currY += yAdd;
		lastLineStart = lineStart;
		lineStart += bpl * yAdd;
	}
	return srcData;
}

UInt8 *PNGParser_ParsePixelsWord(UInt8 *srcData, UInt8 *destBuff, UOSInt bpl, UOSInt initX, UOSInt initY, UOSInt maxX, UOSInt maxY, UOSInt xAdd, UOSInt yAdd)
{
	UInt8 *lineStart = destBuff + initY * bpl;
	UOSInt currX;
	UOSInt currY;
	UInt8 px;
	UInt8 *lastLineStart;
	UInt8 lastPx[2];
	lastLineStart = 0;
	currY = initY;
	while (currY < maxY)
	{
		currX = initX;
		px = *srcData++;
		if (px == 0)
		{
			while (currX < maxX)
			{
				lineStart[currX * 2 + 0] = srcData[1];
				lineStart[currX * 2 + 1] = srcData[0];
				srcData += 2;
				currX += xAdd;
			}
		}
		else if (px == 1)
		{
			lastPx[0] = 0;
			lastPx[1] = 0;
			while (currX < maxX)
			{
				lastPx[0] = (UInt8)(lastPx[0] + srcData[1]);
				lastPx[1] = (UInt8)(lastPx[1] + srcData[0]);
				lineStart[currX * 2 + 0] = lastPx[0];
				lineStart[currX * 2 + 1] = lastPx[1];
				srcData += 2;
				currX += xAdd;
			}
		}
		else if (px == 2)
		{
			if (lastLineStart)
			{
				while (currX < maxX)
				{
					lineStart[currX * 2 + 0] = (UInt8)(lastLineStart[currX * 2 + 0] + srcData[1]);
					lineStart[currX * 2 + 1] = (UInt8)(lastLineStart[currX * 2 + 1] + srcData[0]);
					srcData += 2;
					currX += xAdd;
				}
			}
			else
			{
				while (currX < maxX)
				{
					lineStart[currX * 2 + 0] = srcData[1];
					lineStart[currX * 2 + 1] = srcData[0];
					srcData += 2;
					currX += xAdd;
				}
			}
		}
		else if (px == 3)
		{
			if (lastLineStart)
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				while (currX < maxX)
				{
					lastPx[0] = (UInt8)(((lastPx[0] + lastLineStart[currX * 2 + 0]) >> 1) + srcData[1]);
					lastPx[1] = (UInt8)(((lastPx[1] + lastLineStart[currX * 2 + 1]) >> 1) + srcData[0]);
					lineStart[currX * 2 + 0] = lastPx[0];
					lineStart[currX * 2 + 1] = lastPx[1];
					srcData += 2;
					currX += xAdd;
				}
			}
			else
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				while (currX < maxX)
				{
					lastPx[0] = (UInt8)((lastPx[0] >> 1) + srcData[1]);
					lastPx[1] = (UInt8)((lastPx[1] >> 1) + srcData[0]);
					lineStart[currX * 2 + 0] = lastPx[0];
					lineStart[currX * 2 + 1] = lastPx[1];
					srcData += 2;
					currX += xAdd;
				}
			}
		}
		else if (px == 4)
		{
			if (lastLineStart)
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				while (currX < maxX)
				{
					if (currX < xAdd)
					{
						lastPx[0] = (UInt8)(lastLineStart[currX * 2 + 0] + srcData[1]);
						lastPx[1] = (UInt8)(lastLineStart[currX * 2 + 1] + srcData[0]);
					}
					else
					{
						lastPx[0] = (UInt8)(PNGParser_PaethPredictor(lastPx[0], lastLineStart[currX * 2 + 0], lastLineStart[(currX - xAdd) * 2 + 0]) + srcData[1]);
						lastPx[1] = (UInt8)(PNGParser_PaethPredictor(lastPx[1], lastLineStart[currX * 2 + 1], lastLineStart[(currX - xAdd) * 2 + 1]) + srcData[0]);
					}
					lineStart[currX * 2 + 0] = lastPx[0];
					lineStart[currX * 2 + 1] = lastPx[1];
					srcData += 2;
					currX += xAdd;
				}
			}
			else
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				while (currX < maxX)
				{
					if (currX < xAdd)
					{
						lastPx[0] = srcData[1];
						lastPx[1] = srcData[0];
					}
					else
					{
						lastPx[0] = (UInt8)(lastPx[0] + srcData[1]);
						lastPx[1] = (UInt8)(lastPx[1] + srcData[0]);
					}
					lineStart[currX * 2 + 0] = lastPx[0];
					lineStart[currX * 2 + 1] = lastPx[1];
					srcData += 2;
					currX += xAdd;
				}
			}
		}
		else
		{
			while (currX < maxX)
			{
				lineStart[currX * 2 + 0] = srcData[1];
				lineStart[currX * 2 + 1] = srcData[0];
				srcData += 2;
				currX += xAdd;
			}
		}
		currY += yAdd;
		lastLineStart = lineStart;
		lineStart += bpl * yAdd;
	}
	return srcData;
}

UInt8 *PNGParser_ParsePixelsRGB24(UInt8 *srcData, UInt8 *destBuff, UOSInt bpl, UOSInt initX, UOSInt initY, UOSInt maxX, UOSInt maxY, UOSInt xAdd, UOSInt yAdd)
{
	UInt8 *lineStart = destBuff + initY * bpl;
	UOSInt currX;
	UOSInt currY;
	UInt8 px;
	UInt8 *lastLineStart;
	UInt8 lastPx[3];
	lastLineStart = 0;
	currY = initY;
	while (currY < maxY)
	{
		currX = initX;
		px = *srcData++;
		if (px == 0)
		{
			while (currX < maxX)
			{
				lineStart[currX * 3 + 0] = srcData[2];
				lineStart[currX * 3 + 1] = srcData[1];
				lineStart[currX * 3 + 2] = srcData[0];
				srcData += 3;
				currX += xAdd;
			}
		}
		else if (px == 1)
		{
			lastPx[0] = 0;
			lastPx[1] = 0;
			lastPx[2] = 0;
			while (currX < maxX)
			{
				lastPx[0] = (UInt8)(lastPx[0] + srcData[2]);
				lastPx[1] = (UInt8)(lastPx[1] + srcData[1]);
				lastPx[2] = (UInt8)(lastPx[2] + srcData[0]);
				lineStart[currX * 3 + 0] = lastPx[0];
				lineStart[currX * 3 + 1] = lastPx[1];
				lineStart[currX * 3 + 2] = lastPx[2];
				srcData += 3;
				currX += xAdd;
			}
		}
		else if (px == 2)
		{
			if (lastLineStart)
			{
				while (currX < maxX)
				{
					lineStart[currX * 3 + 0] = (UInt8)(lastLineStart[currX * 3 + 0] + srcData[2]);
					lineStart[currX * 3 + 1] = (UInt8)(lastLineStart[currX * 3 + 1] + srcData[1]);
					lineStart[currX * 3 + 2] = (UInt8)(lastLineStart[currX * 3 + 2] + srcData[0]);
					srcData += 3;
					currX += xAdd;
				}
			}
			else
			{
				while (currX < maxX)
				{
					lineStart[currX * 3 + 0] = srcData[2];
					lineStart[currX * 3 + 1] = srcData[1];
					lineStart[currX * 3 + 2] = srcData[0];
					srcData += 3;
					currX += xAdd;
				}
			}
		}
		else if (px == 3)
		{
			if (lastLineStart)
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				while (currX < maxX)
				{
					lastPx[0] = (UInt8)(((lastPx[0] + lastLineStart[currX * 3 + 0]) >> 1) + srcData[2]);
					lastPx[1] = (UInt8)(((lastPx[1] + lastLineStart[currX * 3 + 1]) >> 1) + srcData[1]);
					lastPx[2] = (UInt8)(((lastPx[2] + lastLineStart[currX * 3 + 2]) >> 1) + srcData[0]);
					lineStart[currX * 3 + 0] = lastPx[0];
					lineStart[currX * 3 + 1] = lastPx[1];
					lineStart[currX * 3 + 2] = lastPx[2];
					srcData += 3;
					currX += xAdd;
				}
			}
			else
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				while (currX < maxX)
				{
					lastPx[0] = (UInt8)((lastPx[0] >> 1) + srcData[2]);
					lastPx[1] = (UInt8)((lastPx[1] >> 1) + srcData[1]);
					lastPx[2] = (UInt8)((lastPx[2] >> 1) + srcData[0]);
					lineStart[currX * 3 + 0] = lastPx[0];
					lineStart[currX * 3 + 1] = lastPx[1];
					lineStart[currX * 3 + 2] = lastPx[2];
					srcData += 3;
					currX += xAdd;
				}
			}
		}
		else if (px == 4)
		{
			if (lastLineStart)
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				while (currX < maxX)
				{
					if (currX < xAdd)
					{
						lastPx[0] = (UInt8)(lastLineStart[currX * 3 + 0] + srcData[2]);
						lastPx[1] = (UInt8)(lastLineStart[currX * 3 + 1] + srcData[1]);
						lastPx[2] = (UInt8)(lastLineStart[currX * 3 + 2] + srcData[0]);
					}
					else
					{
						lastPx[0] = (UInt8)(PNGParser_PaethPredictor(lastPx[0], lastLineStart[currX * 3 + 0], lastLineStart[(currX - xAdd) * 3 + 0]) + srcData[2]);
						lastPx[1] = (UInt8)(PNGParser_PaethPredictor(lastPx[1], lastLineStart[currX * 3 + 1], lastLineStart[(currX - xAdd) * 3 + 1]) + srcData[1]);
						lastPx[2] = (UInt8)(PNGParser_PaethPredictor(lastPx[2], lastLineStart[currX * 3 + 2], lastLineStart[(currX - xAdd) * 3 + 2]) + srcData[0]);
					}
					lineStart[currX * 3 + 0] = lastPx[0];
					lineStart[currX * 3 + 1] = lastPx[1];
					lineStart[currX * 3 + 2] = lastPx[2];
					srcData += 3;
					currX += xAdd;
				}
			}
			else
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				while (currX < maxX)
				{
					if (currX < xAdd)
					{
						lastPx[0] = srcData[2];
						lastPx[1] = srcData[1];
						lastPx[2] = srcData[0];
					}
					else
					{
						lastPx[0] = (UInt8)(lastPx[0] + srcData[2]);
						lastPx[1] = (UInt8)(lastPx[1] + srcData[1]);
						lastPx[2] = (UInt8)(lastPx[2] + srcData[0]);
					}
					lineStart[currX * 3 + 0] = lastPx[0];
					lineStart[currX * 3 + 1] = lastPx[1];
					lineStart[currX * 3 + 2] = lastPx[2];
					srcData += 3;
					currX += xAdd;
				}
			}
		}
		else
		{
			while (currX < maxX)
			{
				lineStart[currX * 3 + 0] = srcData[2];
				lineStart[currX * 3 + 1] = srcData[1];
				lineStart[currX * 3 + 2] = srcData[0];
				srcData += 3;
				currX += xAdd;
			}
		}
		currY += yAdd;
		lastLineStart = lineStart;
		lineStart += bpl * yAdd;
	}
	return srcData;
}

UInt8 *PNGParser_ParsePixelsRGB48(UInt8 *srcData, UInt8 *destBuff, UOSInt bpl, UOSInt initX, UOSInt initY, UOSInt maxX, UOSInt maxY, UOSInt xAdd, UOSInt yAdd)
{
	UInt8 *lineStart = destBuff + initY * bpl;
	UOSInt currX;
	UOSInt currY;
	UInt8 px;
	UInt8 *lastLineStart;
	UInt8 lastPx[6];
	lastLineStart = 0;
	currY = initY;
	while (currY < maxY)
	{
		currX = initX;
		px = *srcData++;
		if (px == 0)
		{
			while (currX < maxX)
			{
				lineStart[currX * 6 + 0] = srcData[5];
				lineStart[currX * 6 + 1] = srcData[4];
				lineStart[currX * 6 + 2] = srcData[3];
				lineStart[currX * 6 + 3] = srcData[2];
				lineStart[currX * 6 + 4] = srcData[1];
				lineStart[currX * 6 + 5] = srcData[0];
				srcData += 6;
				currX += xAdd;
			}
		}
		else if (px == 1)
		{
			lastPx[0] = 0;
			lastPx[1] = 0;
			lastPx[2] = 0;
			lastPx[3] = 0;
			lastPx[4] = 0;
			lastPx[5] = 0;
			while (currX < maxX)
			{
				lastPx[0] = (UInt8)(lastPx[0] + srcData[5]);
				lastPx[1] = (UInt8)(lastPx[1] + srcData[4]);
				lastPx[2] = (UInt8)(lastPx[2] + srcData[3]);
				lastPx[3] = (UInt8)(lastPx[3] + srcData[2]);
				lastPx[4] = (UInt8)(lastPx[4] + srcData[1]);
				lastPx[5] = (UInt8)(lastPx[5] + srcData[0]);
				lineStart[currX * 6 + 0] = lastPx[0];
				lineStart[currX * 6 + 1] = lastPx[1];
				lineStart[currX * 6 + 2] = lastPx[2];
				lineStart[currX * 6 + 3] = lastPx[3];
				lineStart[currX * 6 + 4] = lastPx[4];
				lineStart[currX * 6 + 5] = lastPx[5];
				srcData += 6;
				currX += xAdd;
			}
		}
		else if (px == 2)
		{
			if (lastLineStart)
			{
				while (currX < maxX)
				{
					lineStart[currX * 6 + 0] = (UInt8)(lastLineStart[currX * 6 + 0] + srcData[5]);
					lineStart[currX * 6 + 1] = (UInt8)(lastLineStart[currX * 6 + 1] + srcData[4]);
					lineStart[currX * 6 + 2] = (UInt8)(lastLineStart[currX * 6 + 2] + srcData[3]);
					lineStart[currX * 6 + 3] = (UInt8)(lastLineStart[currX * 6 + 3] + srcData[2]);
					lineStart[currX * 6 + 4] = (UInt8)(lastLineStart[currX * 6 + 4] + srcData[1]);
					lineStart[currX * 6 + 5] = (UInt8)(lastLineStart[currX * 6 + 5] + srcData[0]);
					srcData += 6;
					currX += xAdd;
				}
			}
			else
			{
				while (currX < maxX)
				{
					lineStart[currX * 6 + 0] = srcData[5];
					lineStart[currX * 6 + 1] = srcData[4];
					lineStart[currX * 6 + 2] = srcData[3];
					lineStart[currX * 6 + 3] = srcData[2];
					lineStart[currX * 6 + 4] = srcData[1];
					lineStart[currX * 6 + 5] = srcData[0];
					srcData += 6;
					currX += xAdd;
				}
			}
		}
		else if (px == 3)
		{
			if (lastLineStart)
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				lastPx[3] = 0;
				lastPx[4] = 0;
				lastPx[5] = 0;
				while (currX < maxX)
				{
					lastPx[0] = (UInt8)(((lastPx[0] + lastLineStart[currX * 6 + 0]) >> 1) + srcData[5]);
					lastPx[1] = (UInt8)(((lastPx[1] + lastLineStart[currX * 6 + 1]) >> 1) + srcData[4]);
					lastPx[2] = (UInt8)(((lastPx[2] + lastLineStart[currX * 6 + 2]) >> 1) + srcData[3]);
					lastPx[3] = (UInt8)(((lastPx[3] + lastLineStart[currX * 6 + 3]) >> 1) + srcData[2]);
					lastPx[4] = (UInt8)(((lastPx[4] + lastLineStart[currX * 6 + 4]) >> 1) + srcData[1]);
					lastPx[5] = (UInt8)(((lastPx[5] + lastLineStart[currX * 6 + 5]) >> 1) + srcData[0]);
					lineStart[currX * 6 + 0] = lastPx[0];
					lineStart[currX * 6 + 1] = lastPx[1];
					lineStart[currX * 6 + 2] = lastPx[2];
					lineStart[currX * 6 + 3] = lastPx[3];
					lineStart[currX * 6 + 4] = lastPx[4];
					lineStart[currX * 6 + 5] = lastPx[5];
					srcData += 6;
					currX += xAdd;
				}
			}
			else
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				lastPx[3] = 0;
				lastPx[4] = 0;
				lastPx[5] = 0;
				while (currX < maxX)
				{
					lastPx[0] = (UInt8)((lastPx[0] >> 1) + srcData[5]);
					lastPx[1] = (UInt8)((lastPx[1] >> 1) + srcData[4]);
					lastPx[2] = (UInt8)((lastPx[2] >> 1) + srcData[3]);
					lastPx[3] = (UInt8)((lastPx[3] >> 1) + srcData[2]);
					lastPx[4] = (UInt8)((lastPx[4] >> 1) + srcData[1]);
					lastPx[5] = (UInt8)((lastPx[5] >> 1) + srcData[0]);
					lineStart[currX * 6 + 0] = lastPx[0];
					lineStart[currX * 6 + 1] = lastPx[1];
					lineStart[currX * 6 + 2] = lastPx[2];
					lineStart[currX * 6 + 3] = lastPx[3];
					lineStart[currX * 6 + 4] = lastPx[4];
					lineStart[currX * 6 + 5] = lastPx[5];
					srcData += 6;
					currX += xAdd;
				}
			}
		}
		else if (px == 4)
		{
			if (lastLineStart)
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				lastPx[3] = 0;
				lastPx[4] = 0;
				lastPx[5] = 0;
				while (currX < maxX)
				{
					if (currX < xAdd)
					{
						lastPx[0] = (UInt8)(lastLineStart[currX * 6 + 0] + srcData[5]);
						lastPx[1] = (UInt8)(lastLineStart[currX * 6 + 1] + srcData[4]);
						lastPx[2] = (UInt8)(lastLineStart[currX * 6 + 2] + srcData[3]);
						lastPx[3] = (UInt8)(lastLineStart[currX * 6 + 3] + srcData[2]);
						lastPx[4] = (UInt8)(lastLineStart[currX * 6 + 4] + srcData[1]);
						lastPx[5] = (UInt8)(lastLineStart[currX * 6 + 5] + srcData[0]);
					}
					else
					{
						lastPx[0] = (UInt8)(PNGParser_PaethPredictor(lastPx[0], lastLineStart[currX * 6 + 0], lastLineStart[(currX - xAdd) * 6 + 0]) + srcData[5]);
						lastPx[1] = (UInt8)(PNGParser_PaethPredictor(lastPx[1], lastLineStart[currX * 6 + 1], lastLineStart[(currX - xAdd) * 6 + 1]) + srcData[4]);
						lastPx[2] = (UInt8)(PNGParser_PaethPredictor(lastPx[2], lastLineStart[currX * 6 + 2], lastLineStart[(currX - xAdd) * 6 + 2]) + srcData[3]);
						lastPx[3] = (UInt8)(PNGParser_PaethPredictor(lastPx[3], lastLineStart[currX * 6 + 3], lastLineStart[(currX - xAdd) * 6 + 3]) + srcData[2]);
						lastPx[4] = (UInt8)(PNGParser_PaethPredictor(lastPx[4], lastLineStart[currX * 6 + 4], lastLineStart[(currX - xAdd) * 6 + 4]) + srcData[1]);
						lastPx[5] = (UInt8)(PNGParser_PaethPredictor(lastPx[5], lastLineStart[currX * 6 + 5], lastLineStart[(currX - xAdd) * 6 + 5]) + srcData[0]);
					}
					lineStart[currX * 6 + 0] = lastPx[0];
					lineStart[currX * 6 + 1] = lastPx[1];
					lineStart[currX * 6 + 2] = lastPx[2];
					lineStart[currX * 6 + 3] = lastPx[3];
					lineStart[currX * 6 + 4] = lastPx[4];
					lineStart[currX * 6 + 5] = lastPx[5];
					srcData += 6;
					currX += xAdd;
				}
			}
			else
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				lastPx[3] = 0;
				lastPx[4] = 0;
				lastPx[5] = 0;
				while (currX < maxX)
				{
					if (currX < xAdd)
					{
						lastPx[0] = srcData[5];
						lastPx[1] = srcData[4];
						lastPx[2] = srcData[3];
						lastPx[3] = srcData[2];
						lastPx[4] = srcData[1];
						lastPx[5] = srcData[0];
					}
					else
					{
						lastPx[0] = (UInt8)(lastPx[0] + srcData[5]);
						lastPx[1] = (UInt8)(lastPx[1] + srcData[4]);
						lastPx[2] = (UInt8)(lastPx[2] + srcData[3]);
						lastPx[3] = (UInt8)(lastPx[3] + srcData[2]);
						lastPx[4] = (UInt8)(lastPx[4] + srcData[1]);
						lastPx[5] = (UInt8)(lastPx[5] + srcData[0]);
					}
					lineStart[currX * 6 + 0] = lastPx[0];
					lineStart[currX * 6 + 1] = lastPx[1];
					lineStart[currX * 6 + 2] = lastPx[2];
					lineStart[currX * 6 + 3] = lastPx[3];
					lineStart[currX * 6 + 4] = lastPx[4];
					lineStart[currX * 6 + 5] = lastPx[5];
					srcData += 6;
					currX += xAdd;
				}
			}
		}
		else
		{
			while (currX < maxX)
			{
				lineStart[currX * 6 + 0] = srcData[5];
				lineStart[currX * 6 + 1] = srcData[4];
				lineStart[currX * 6 + 2] = srcData[3];
				lineStart[currX * 6 + 3] = srcData[2];
				lineStart[currX * 6 + 4] = srcData[1];
				lineStart[currX * 6 + 5] = srcData[0];
				srcData += 6;
				currX += xAdd;
			}
		}
		currY += yAdd;
		lastLineStart = lineStart;
		lineStart += bpl * yAdd;
	}
	return srcData;
}

UInt8 *PNGParser_ParsePixelsARGB32(UInt8 *srcData, UInt8 *destBuff, UOSInt bpl, UOSInt initX, UOSInt initY, UOSInt maxX, UOSInt maxY, UOSInt xAdd, UOSInt yAdd, Bool *alphaFound)
{
	UInt8 *lineStart = destBuff + initY * bpl;
	UOSInt currX;
	UOSInt currY;
	UInt8 px;
	UInt8 *lastLineStart;
	UInt8 lastPx[4];
	UInt8 a;
	Bool semiTr = false;
	lastLineStart = 0;
	currY = initY;
	while (currY < maxY)
	{
		currX = initX;
		px = *srcData++;
		if (px == 0)
		{
			while (currX < maxX)
			{
				lineStart[currX * 4 + 0] = srcData[2];
				lineStart[currX * 4 + 1] = srcData[1];
				lineStart[currX * 4 + 2] = srcData[0];
				a = lineStart[currX * 4 + 3] = srcData[3];
				if (a != 0xff)
				{
					semiTr = true;
				}
				srcData += 4;
				currX += xAdd;
			}
		}
		else if (px == 1)
		{
			lastPx[0] = 0;
			lastPx[1] = 0;
			lastPx[2] = 0;
			lastPx[3] = 0;
			while (currX < maxX)
			{
				lastPx[0] = (UInt8)(lastPx[0] + srcData[2]);
				lastPx[1] = (UInt8)(lastPx[1] + srcData[1]);
				lastPx[2] = (UInt8)(lastPx[2] + srcData[0]);
				lastPx[3] = (UInt8)(lastPx[3] + srcData[3]);
				lineStart[currX * 4 + 0] = lastPx[0];
				lineStart[currX * 4 + 1] = lastPx[1];
				lineStart[currX * 4 + 2] = lastPx[2];
				lineStart[currX * 4 + 3] = lastPx[3];
				if (lastPx[3] != 0xff)
				{
					semiTr = true;
				}
				srcData += 4;
				currX += xAdd;
			}
		}
		else if (px == 2)
		{
			if (lastLineStart)
			{
				while (currX < maxX)
				{
					lineStart[currX * 4 + 0] = (UInt8)(lastLineStart[currX * 4 + 0] + srcData[2]);
					lineStart[currX * 4 + 1] = (UInt8)(lastLineStart[currX * 4 + 1] + srcData[1]);
					lineStart[currX * 4 + 2] = (UInt8)(lastLineStart[currX * 4 + 2] + srcData[0]);
					a = lineStart[currX * 4 + 3] = (UInt8)(lastLineStart[currX * 4 + 3] + srcData[3]);
					if (a != 0xff)
					{
						semiTr = true;
					}
					srcData += 4;
					currX += xAdd;
				}
			}
			else
			{
				while (currX < maxX)
				{
					lineStart[currX * 4 + 0] = srcData[2];
					lineStart[currX * 4 + 1] = srcData[1];
					lineStart[currX * 4 + 2] = srcData[0];
					a = lineStart[currX * 4 + 3] = srcData[3];
					if (a != 0xff)
					{
						semiTr = true;
					}
					srcData += 4;
					currX += xAdd;
				}
			}
		}
		else if (px == 3)
		{
			if (lastLineStart)
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				lastPx[3] = 0;
				while (currX < maxX)
				{
					lastPx[0] = (UInt8)(((lastPx[0] + lastLineStart[currX * 4 + 0]) >> 1) + srcData[2]);
					lastPx[1] = (UInt8)(((lastPx[1] + lastLineStart[currX * 4 + 1]) >> 1) + srcData[1]);
					lastPx[2] = (UInt8)(((lastPx[2] + lastLineStart[currX * 4 + 2]) >> 1) + srcData[0]);
					lastPx[3] = (UInt8)(((lastPx[3] + lastLineStart[currX * 4 + 3]) >> 1) + srcData[3]);
					lineStart[currX * 4 + 0] = lastPx[0];
					lineStart[currX * 4 + 1] = lastPx[1];
					lineStart[currX * 4 + 2] = lastPx[2];
					lineStart[currX * 4 + 3] = lastPx[3];
					if (lastPx[3] != 0xff)
					{
						semiTr = true;
					}
					srcData += 4;
					currX += xAdd;
				}
			}
			else
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				lastPx[3] = 0;
				while (currX < maxX)
				{
					lastPx[0] = (UInt8)((lastPx[0] >> 1) + srcData[2]);
					lastPx[1] = (UInt8)((lastPx[1] >> 1) + srcData[1]);
					lastPx[2] = (UInt8)((lastPx[2] >> 1) + srcData[0]);
					lastPx[3] = (UInt8)((lastPx[3] >> 1) + srcData[3]);
					lineStart[currX * 4 + 0] = lastPx[0];
					lineStart[currX * 4 + 1] = lastPx[1];
					lineStart[currX * 4 + 2] = lastPx[2];
					lineStart[currX * 4 + 3] = lastPx[3];
					if (lastPx[3] != 0xff)
					{
						semiTr = true;
					}
					srcData += 4;
					currX += xAdd;
				}
			}
		}
		else if (px == 4)
		{
			if (lastLineStart)
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				lastPx[3] = 0;
				while (currX < maxX)
				{
					if (currX < xAdd)
					{
						lastPx[0] = (UInt8)(lastLineStart[currX * 4 + 0] + srcData[2]);
						lastPx[1] = (UInt8)(lastLineStart[currX * 4 + 1] + srcData[1]);
						lastPx[2] = (UInt8)(lastLineStart[currX * 4 + 2] + srcData[0]);
						lastPx[3] = (UInt8)(lastLineStart[currX * 4 + 3] + srcData[3]);
					}
					else
					{
						lastPx[0] = (UInt8)(PNGParser_PaethPredictor(lastPx[0], lastLineStart[currX * 4 + 0], lastLineStart[(currX - xAdd) * 4 + 0]) + srcData[2]);
						lastPx[1] = (UInt8)(PNGParser_PaethPredictor(lastPx[1], lastLineStart[currX * 4 + 1], lastLineStart[(currX - xAdd) * 4 + 1]) + srcData[1]);
						lastPx[2] = (UInt8)(PNGParser_PaethPredictor(lastPx[2], lastLineStart[currX * 4 + 2], lastLineStart[(currX - xAdd) * 4 + 2]) + srcData[0]);
						lastPx[3] = (UInt8)(PNGParser_PaethPredictor(lastPx[3], lastLineStart[currX * 4 + 3], lastLineStart[(currX - xAdd) * 4 + 3]) + srcData[3]);
					}
					lineStart[currX * 4 + 0] = lastPx[0];
					lineStart[currX * 4 + 1] = lastPx[1];
					lineStart[currX * 4 + 2] = lastPx[2];
					lineStart[currX * 4 + 3] = lastPx[3];
					if (lastPx[3] != 0xff)
					{
						semiTr = true;
					}
					srcData += 4;
					currX += xAdd;
				}
			}
			else
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				lastPx[3] = 0;
				while (currX < maxX)
				{
					if (currX < xAdd)
					{
						lastPx[0] = srcData[2];
						lastPx[1] = srcData[1];
						lastPx[2] = srcData[0];
						lastPx[3] = srcData[3];
					}
					else
					{
						lastPx[0] = (UInt8)(lastPx[0] + srcData[2]);
						lastPx[1] = (UInt8)(lastPx[1] + srcData[1]);
						lastPx[2] = (UInt8)(lastPx[2] + srcData[0]);
						lastPx[3] = (UInt8)(lastPx[3] + srcData[3]);
					}
					lineStart[currX * 4 + 0] = lastPx[0];
					lineStart[currX * 4 + 1] = lastPx[1];
					lineStart[currX * 4 + 2] = lastPx[2];
					lineStart[currX * 4 + 3] = lastPx[3];
					if (lastPx[3] != 0xff)
					{
						semiTr = true;
					}
					srcData += 4;
					currX += xAdd;
				}
			}
		}
		else
		{
			while (currX < maxX)
			{
				lineStart[currX * 4 + 0] = srcData[2];
				lineStart[currX * 4 + 1] = srcData[1];
				lineStart[currX * 4 + 2] = srcData[0];
				a = lineStart[currX * 4 + 3] = srcData[3];
				if (a != 0xff)
				{
					semiTr = true;
				}
				srcData += 4;
				currX += xAdd;
			}
		}
		currY += yAdd;
		lastLineStart = lineStart;
		lineStart += bpl * yAdd;
	}
	if (semiTr)
	{
		*alphaFound = true;
	}
	return srcData;
}

UInt8 *PNGParser_ParsePixelsARGB64(UInt8 *srcData, UInt8 *destBuff, UOSInt bpl, UOSInt initX, UOSInt initY, UOSInt maxX, UOSInt maxY, UOSInt xAdd, UOSInt yAdd, Bool *alphaFound)
{
	UInt8 *lineStart = destBuff + initY * bpl;
	UOSInt currX;
	UOSInt currY;
	UInt8 px;
	UInt8 *lastLineStart;
	UInt8 lastPx[8];
	UInt8 a0;
	UInt8 a1;
	Bool semiTr = false;
	lastLineStart = 0;
	currY = initY;
	while (currY < maxY)
	{
		currX = initX;
		px = *srcData++;
		if (px == 0)
		{
			while (currX < maxX)
			{
				lineStart[currX * 8 + 0] = srcData[5];
				lineStart[currX * 8 + 1] = srcData[4];
				lineStart[currX * 8 + 2] = srcData[3];
				lineStart[currX * 8 + 3] = srcData[2];
				lineStart[currX * 8 + 4] = srcData[1];
				lineStart[currX * 8 + 5] = srcData[0];
				a0 = lineStart[currX * 8 + 6] = srcData[7];
				a1 = lineStart[currX * 8 + 7] = srcData[6];
				if (a0 != 0xff || a1 != 0xff)
				{
					semiTr = true;
				}
				srcData += 8;
				currX += xAdd;
			}
		}
		else if (px == 1)
		{
			lastPx[0] = 0;
			lastPx[1] = 0;
			lastPx[2] = 0;
			lastPx[3] = 0;
			lastPx[4] = 0;
			lastPx[5] = 0;
			lastPx[6] = 0;
			lastPx[7] = 0;
			while (currX < maxX)
			{
				lastPx[0] = (UInt8)(lastPx[0] + srcData[5]);
				lastPx[1] = (UInt8)(lastPx[1] + srcData[4]);
				lastPx[2] = (UInt8)(lastPx[2] + srcData[3]);
				lastPx[3] = (UInt8)(lastPx[3] + srcData[2]);
				lastPx[4] = (UInt8)(lastPx[4] + srcData[1]);
				lastPx[5] = (UInt8)(lastPx[5] + srcData[0]);
				lastPx[6] = (UInt8)(lastPx[6] + srcData[7]);
				lastPx[7] = (UInt8)(lastPx[7] + srcData[6]);
				lineStart[currX * 8 + 0] = lastPx[0];
				lineStart[currX * 8 + 1] = lastPx[1];
				lineStart[currX * 8 + 2] = lastPx[2];
				lineStart[currX * 8 + 3] = lastPx[3];
				lineStart[currX * 8 + 4] = lastPx[4];
				lineStart[currX * 8 + 5] = lastPx[5];
				lineStart[currX * 8 + 6] = lastPx[6];
				lineStart[currX * 8 + 7] = lastPx[7];
				if (lastPx[6] != 0xff || lastPx[7] != 0xff)
				{
					semiTr = true;
				}
				srcData += 8;
				currX += xAdd;
			}
		}
		else if (px == 2)
		{
			if (lastLineStart)
			{
				while (currX < maxX)
				{
					lineStart[currX * 8 + 0] = (UInt8)(lastLineStart[currX * 8 + 0] + srcData[5]);
					lineStart[currX * 8 + 1] = (UInt8)(lastLineStart[currX * 8 + 1] + srcData[4]);
					lineStart[currX * 8 + 2] = (UInt8)(lastLineStart[currX * 8 + 2] + srcData[3]);
					lineStart[currX * 8 + 3] = (UInt8)(lastLineStart[currX * 8 + 3] + srcData[2]);
					lineStart[currX * 8 + 4] = (UInt8)(lastLineStart[currX * 8 + 4] + srcData[1]);
					lineStart[currX * 8 + 5] = (UInt8)(lastLineStart[currX * 8 + 5] + srcData[0]);
					a0 = lineStart[currX * 8 + 6] = (UInt8)(lastLineStart[currX * 8 + 6] + srcData[7]);
					a1 = lineStart[currX * 8 + 7] = (UInt8)(lastLineStart[currX * 8 + 7] + srcData[6]);
					if (a0 != 0xff || a1 != 0xff)
					{
						semiTr = true;
					}
					srcData += 8;
					currX += xAdd;
				}
			}
			else
			{
				while (currX < maxX)
				{
					lineStart[currX * 8 + 0] = srcData[5];
					lineStart[currX * 8 + 1] = srcData[4];
					lineStart[currX * 8 + 2] = srcData[3];
					lineStart[currX * 8 + 3] = srcData[2];
					lineStart[currX * 8 + 4] = srcData[1];
					lineStart[currX * 8 + 5] = srcData[0];
					a0 = lineStart[currX * 8 + 6] = srcData[7];
					a1 = lineStart[currX * 8 + 7] = srcData[6];
					if (a0 != 0xff || a1 != 0xff)
					{
						semiTr = true;
					}
					srcData += 8;
					currX += xAdd;
				}
			}
		}
		else if (px == 3)
		{
			if (lastLineStart)
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				lastPx[3] = 0;
				lastPx[4] = 0;
				lastPx[5] = 0;
				lastPx[6] = 0;
				lastPx[7] = 0;
				while (currX < maxX)
				{
					lastPx[0] = (UInt8)(((lastPx[0] + lastLineStart[currX * 8 + 0]) >> 1) + srcData[5]);
					lastPx[1] = (UInt8)(((lastPx[1] + lastLineStart[currX * 8 + 1]) >> 1) + srcData[4]);
					lastPx[2] = (UInt8)(((lastPx[2] + lastLineStart[currX * 8 + 2]) >> 1) + srcData[3]);
					lastPx[3] = (UInt8)(((lastPx[3] + lastLineStart[currX * 8 + 3]) >> 1) + srcData[2]);
					lastPx[4] = (UInt8)(((lastPx[4] + lastLineStart[currX * 8 + 4]) >> 1) + srcData[1]);
					lastPx[5] = (UInt8)(((lastPx[5] + lastLineStart[currX * 8 + 5]) >> 1) + srcData[0]);
					lastPx[6] = (UInt8)(((lastPx[6] + lastLineStart[currX * 8 + 6]) >> 1) + srcData[7]);
					lastPx[7] = (UInt8)(((lastPx[7] + lastLineStart[currX * 8 + 7]) >> 1) + srcData[6]);
					lineStart[currX * 8 + 0] = lastPx[0];
					lineStart[currX * 8 + 1] = lastPx[1];
					lineStart[currX * 8 + 2] = lastPx[2];
					lineStart[currX * 8 + 3] = lastPx[3];
					lineStart[currX * 8 + 4] = lastPx[4];
					lineStart[currX * 8 + 5] = lastPx[5];
					lineStart[currX * 8 + 6] = lastPx[6];
					lineStart[currX * 8 + 7] = lastPx[7];
					if (lastPx[6] != 0xff || lastPx[7] != 0xff)
					{
						semiTr = true;
					}
					srcData += 8;
					currX += xAdd;
				}
			}
			else
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				lastPx[3] = 0;
				lastPx[4] = 0;
				lastPx[5] = 0;
				lastPx[6] = 0;
				lastPx[7] = 0;
				while (currX < maxX)
				{
					lastPx[0] = (UInt8)((lastPx[0] >> 1) + srcData[5]);
					lastPx[1] = (UInt8)((lastPx[1] >> 1) + srcData[4]);
					lastPx[2] = (UInt8)((lastPx[2] >> 1) + srcData[3]);
					lastPx[3] = (UInt8)((lastPx[3] >> 1) + srcData[2]);
					lastPx[4] = (UInt8)((lastPx[4] >> 1) + srcData[1]);
					lastPx[5] = (UInt8)((lastPx[5] >> 1) + srcData[0]);
					lastPx[6] = (UInt8)((lastPx[6] >> 1) + srcData[7]);
					lastPx[7] = (UInt8)((lastPx[7] >> 1) + srcData[6]);
					lineStart[currX * 8 + 0] = lastPx[0];
					lineStart[currX * 8 + 1] = lastPx[1];
					lineStart[currX * 8 + 2] = lastPx[2];
					lineStart[currX * 8 + 3] = lastPx[3];
					lineStart[currX * 8 + 4] = lastPx[4];
					lineStart[currX * 8 + 5] = lastPx[5];
					lineStart[currX * 8 + 6] = lastPx[6];
					lineStart[currX * 8 + 7] = lastPx[7];
					if (lastPx[6] != 0xff || lastPx[7] != 0xff)
					{
						semiTr = true;
					}
					srcData += 8;
					currX += xAdd;
				}
			}
		}
		else if (px == 4)
		{
			if (lastLineStart)
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				lastPx[3] = 0;
				lastPx[4] = 0;
				lastPx[5] = 0;
				lastPx[6] = 0;
				lastPx[7] = 0;
				while (currX < maxX)
				{
					if (currX < xAdd)
					{
						lastPx[0] = (UInt8)(lastLineStart[currX * 8 + 0] + srcData[5]);
						lastPx[1] = (UInt8)(lastLineStart[currX * 8 + 1] + srcData[4]);
						lastPx[2] = (UInt8)(lastLineStart[currX * 8 + 2] + srcData[3]);
						lastPx[3] = (UInt8)(lastLineStart[currX * 8 + 3] + srcData[2]);
						lastPx[4] = (UInt8)(lastLineStart[currX * 8 + 4] + srcData[1]);
						lastPx[5] = (UInt8)(lastLineStart[currX * 8 + 5] + srcData[0]);
						lastPx[6] = (UInt8)(lastLineStart[currX * 8 + 6] + srcData[7]);
						lastPx[7] = (UInt8)(lastLineStart[currX * 8 + 7] + srcData[6]);
					}
					else
					{
						lastPx[0] = (UInt8)(PNGParser_PaethPredictor(lastPx[0], lastLineStart[currX * 8 + 0], lastLineStart[(currX - xAdd) * 8 + 0]) + srcData[5]);
						lastPx[1] = (UInt8)(PNGParser_PaethPredictor(lastPx[1], lastLineStart[currX * 8 + 1], lastLineStart[(currX - xAdd) * 8 + 1]) + srcData[4]);
						lastPx[2] = (UInt8)(PNGParser_PaethPredictor(lastPx[2], lastLineStart[currX * 8 + 2], lastLineStart[(currX - xAdd) * 8 + 2]) + srcData[3]);
						lastPx[3] = (UInt8)(PNGParser_PaethPredictor(lastPx[3], lastLineStart[currX * 8 + 3], lastLineStart[(currX - xAdd) * 8 + 3]) + srcData[2]);
						lastPx[4] = (UInt8)(PNGParser_PaethPredictor(lastPx[4], lastLineStart[currX * 8 + 4], lastLineStart[(currX - xAdd) * 8 + 4]) + srcData[1]);
						lastPx[5] = (UInt8)(PNGParser_PaethPredictor(lastPx[5], lastLineStart[currX * 8 + 5], lastLineStart[(currX - xAdd) * 8 + 5]) + srcData[0]);
						lastPx[6] = (UInt8)(PNGParser_PaethPredictor(lastPx[6], lastLineStart[currX * 8 + 6], lastLineStart[(currX - xAdd) * 8 + 6]) + srcData[7]);
						lastPx[7] = (UInt8)(PNGParser_PaethPredictor(lastPx[7], lastLineStart[currX * 8 + 7], lastLineStart[(currX - xAdd) * 8 + 7]) + srcData[6]);
					}
					lineStart[currX * 8 + 0] = lastPx[0];
					lineStart[currX * 8 + 1] = lastPx[1];
					lineStart[currX * 8 + 2] = lastPx[2];
					lineStart[currX * 8 + 3] = lastPx[3];
					lineStart[currX * 8 + 4] = lastPx[4];
					lineStart[currX * 8 + 5] = lastPx[5];
					lineStart[currX * 8 + 6] = lastPx[6];
					lineStart[currX * 8 + 7] = lastPx[7];
					if (lastPx[6] != 0xff || lastPx[7] != 0xff)
					{
						semiTr = true;
					}
					srcData += 8;
					currX += xAdd;
				}
			}
			else
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				lastPx[3] = 0;
				lastPx[4] = 0;
				lastPx[5] = 0;
				lastPx[6] = 0;
				lastPx[7] = 0;
				while (currX < maxX)
				{
					if (currX < xAdd)
					{
						lastPx[0] = srcData[5];
						lastPx[1] = srcData[4];
						lastPx[2] = srcData[3];
						lastPx[3] = srcData[2];
						lastPx[4] = srcData[1];
						lastPx[5] = srcData[0];
						lastPx[6] = srcData[7];
						lastPx[7] = srcData[6];
					}
					else
					{
						lastPx[0] = (UInt8)(lastPx[0] + srcData[5]);
						lastPx[1] = (UInt8)(lastPx[1] + srcData[4]);
						lastPx[2] = (UInt8)(lastPx[2] + srcData[3]);
						lastPx[3] = (UInt8)(lastPx[3] + srcData[2]);
						lastPx[4] = (UInt8)(lastPx[4] + srcData[1]);
						lastPx[5] = (UInt8)(lastPx[5] + srcData[0]);
						lastPx[6] = (UInt8)(lastPx[6] + srcData[7]);
						lastPx[7] = (UInt8)(lastPx[7] + srcData[6]);
					}
					lineStart[currX * 8 + 0] = lastPx[0];
					lineStart[currX * 8 + 1] = lastPx[1];
					lineStart[currX * 8 + 2] = lastPx[2];
					lineStart[currX * 8 + 3] = lastPx[3];
					lineStart[currX * 8 + 4] = lastPx[4];
					lineStart[currX * 8 + 5] = lastPx[5];
					lineStart[currX * 8 + 6] = lastPx[6];
					lineStart[currX * 8 + 7] = lastPx[7];
					if (lastPx[6] != 0xff || lastPx[7] != 0xff)
					{
						semiTr = true;
					}
					srcData += 8;
					currX += xAdd;
				}
			}
		}
		else
		{
			while (currX < maxX)
			{
				lineStart[currX * 8 + 0] = srcData[5];
				lineStart[currX * 8 + 1] = srcData[4];
				lineStart[currX * 8 + 2] = srcData[3];
				lineStart[currX * 8 + 3] = srcData[2];
				lineStart[currX * 8 + 4] = srcData[1];
				lineStart[currX * 8 + 5] = srcData[0];
				a0 = lineStart[currX * 8 + 6] = srcData[7];
				a1 = lineStart[currX * 8 + 7] = srcData[6];
				if (a0 != 0xff || a1 != 0xff)
				{
					semiTr = true;
				}
				srcData += 8;
				currX += xAdd;
			}
		}
		currY += yAdd;
		lastLineStart = lineStart;
		lineStart += bpl * yAdd;
	}
	if (semiTr)
	{
		*alphaFound = true;
	}
	return srcData;
}

UInt8 *PNGParser_ParsePixelsAW16(UInt8 *srcData, UInt8 *destBuff, UOSInt bpl, UOSInt initX, UOSInt initY, UOSInt maxX, UOSInt maxY, UOSInt xAdd, UOSInt yAdd, Bool *alphaFound)
{
	UInt8 *lineStart = destBuff + initY * bpl;
	UOSInt currX;
	UOSInt currY;
	UInt8 px;
	UInt8 *lastLineStart;
	UInt8 lastPx[2];
	UInt8 a;
	Bool semiTr = false;
	lastLineStart = 0;
	currY = initY;
	while (currY < maxY)
	{
		currX = initX;
		px = *srcData++;
		if (px == 0)
		{
			while (currX < maxX)
			{
				lineStart[currX * 2 + 0] = srcData[0];
				a = lineStart[currX * 2 + 1] = srcData[1];
				if (a != 0xff)
				{
					semiTr = true;
				}
				srcData += 2;
				currX += xAdd;
			}
		}
		else if (px == 1)
		{
			lastPx[0] = 0;
			lastPx[1] = 0;
			while (currX < maxX)
			{
				lastPx[0] = (UInt8)(lastPx[0] + srcData[0]);
				lastPx[1] = (UInt8)(lastPx[1] + srcData[1]);
				lineStart[currX * 2 + 0] = lastPx[0];
				lineStart[currX * 2 + 1] = lastPx[1];
				if (lastPx[1] != 0xff)
				{
					semiTr = true;
				}
				srcData += 2;
				currX += xAdd;
			}
		}
		else if (px == 2)
		{
			if (lastLineStart)
			{
				while (currX < maxX)
				{
					lineStart[currX * 2 + 0] = (UInt8)(lastLineStart[currX * 2 + 0] + srcData[0]);
					a = lineStart[currX * 2 + 1] = (UInt8)(lastLineStart[currX * 2 + 1] + srcData[1]);
					if (a != 0xff)
					{
						semiTr = true;
					}
					srcData += 2;
					currX += xAdd;
				}
			}
			else
			{
				while (currX < maxX)
				{
					lineStart[currX * 2 + 0] = srcData[0];
					a = lineStart[currX * 2 + 1] = srcData[1];
					if (a != 0xff)
					{
						semiTr = true;
					}
					srcData += 2;
					currX += xAdd;
				}
			}
		}
		else if (px == 3)
		{
			if (lastLineStart)
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				while (currX < maxX)
				{
					lastPx[0] = (UInt8)(((lastPx[0] + lastLineStart[currX * 2 + 0]) >> 1) + srcData[0]);
					lastPx[1] = (UInt8)(((lastPx[1] + lastLineStart[currX * 2 + 1]) >> 1) + srcData[1]);
					lineStart[currX * 2 + 0] = lastPx[0];
					lineStart[currX * 2 + 1] = lastPx[1];
					if (lastPx[1] != 0xff)
					{
						semiTr = true;
					}
					srcData += 2;
					currX += xAdd;
				}
			}
			else
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				while (currX < maxX)
				{
					lastPx[0] = (UInt8)((lastPx[0] >> 1) + srcData[0]);
					lastPx[1] = (UInt8)((lastPx[1] >> 1) + srcData[1]);
					lineStart[currX * 2 + 0] = lastPx[0];
					lineStart[currX * 2 + 1] = lastPx[1];
					if (lastPx[1] != 0xff)
					{
						semiTr = true;
					}
					srcData += 2;
					currX += xAdd;
				}
			}
		}
		else if (px == 4)
		{
			if (lastLineStart)
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				while (currX < maxX)
				{
					if (currX < xAdd)
					{
						lastPx[0] = (UInt8)(lastLineStart[currX * 2 + 0] + srcData[0]);
						lastPx[1] = (UInt8)(lastLineStart[currX * 2 + 1] + srcData[1]);
					}
					else
					{
						lastPx[0] = (UInt8)(PNGParser_PaethPredictor(lastPx[0], lastLineStart[currX * 2 + 0], lastLineStart[(currX - xAdd) * 2 + 0]) + srcData[0]);
						lastPx[1] = (UInt8)(PNGParser_PaethPredictor(lastPx[1], lastLineStart[currX * 2 + 1], lastLineStart[(currX - xAdd) * 2 + 1]) + srcData[1]);
					}
					lineStart[currX * 2 + 0] = lastPx[0];
					lineStart[currX * 2 + 1] = lastPx[1];
					if (lastPx[1] != 0xff)
					{
						semiTr = true;
					}
					srcData += 2;
					currX += xAdd;
				}
			}
			else
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				while (currX < maxX)
				{
					if (currX < xAdd)
					{
						lastPx[0] = srcData[0];
						lastPx[1] = srcData[1];
					}
					else
					{
						lastPx[0] = (UInt8)(lastPx[0] + srcData[0]);
						lastPx[1] = (UInt8)(lastPx[1] + srcData[1]);
					}
					lineStart[currX * 2 + 0] = lastPx[0];
					lineStart[currX * 2 + 1] = lastPx[1];
					if (lastPx[1] != 0xff)
					{
						semiTr = true;
					}
					srcData += 2;
					currX += xAdd;
				}
			}
		}
		else
		{
			while (currX < maxX)
			{
				lineStart[currX * 2 + 0] = srcData[0];
				a = lineStart[currX * 2 + 1] = srcData[1];
				if (a != 0xff)
				{
					semiTr = true;
				}
				srcData += 2;
				currX += xAdd;
			}
		}
		currY += yAdd;
		lastLineStart = lineStart;
		lineStart += bpl * yAdd;
	}
	if (semiTr)
	{
		*alphaFound = true;
	}
	return srcData;
}

UInt8 *PNGParser_ParsePixelsAW32(UInt8 *srcData, UInt8 *destBuff, UOSInt bpl, UOSInt initX, UOSInt initY, UOSInt maxX, UOSInt maxY, UOSInt xAdd, UOSInt yAdd, Bool *alphaFound)
{
	UInt8 *lineStart = destBuff + initY * bpl;
	UOSInt currX;
	UOSInt currY;
	UInt8 px;
	UInt8 *lastLineStart;
	UInt8 lastPx[4];
	UInt8 a0;
	UInt8 a1;
	Bool semiTr = false;
	lastLineStart = 0;
	currY = initY;
	while (currY < maxY)
	{
		currX = initX;
		px = *srcData++;
		if (px == 0)
		{
			while (currX < maxX)
			{
				lineStart[currX * 4 + 0] = srcData[1];
				lineStart[currX * 4 + 1] = srcData[0];
				a0 = lineStart[currX * 4 + 2] = srcData[3];
				a1 = lineStart[currX * 4 + 3] = srcData[2];
				if (a0 != 0xff || a1 != 0xff)
				{
					semiTr = true;
				}
				srcData += 4;
				currX += xAdd;
			}
		}
		else if (px == 1)
		{
			lastPx[0] = 0;
			lastPx[1] = 0;
			lastPx[2] = 0;
			lastPx[3] = 0;
			while (currX < maxX)
			{
				lastPx[0] = (UInt8)(lastPx[0] + srcData[1]);
				lastPx[1] = (UInt8)(lastPx[1] + srcData[0]);
				lastPx[2] = (UInt8)(lastPx[2] + srcData[3]);
				lastPx[3] = (UInt8)(lastPx[3] + srcData[2]);
				lineStart[currX * 4 + 0] = lastPx[0];
				lineStart[currX * 4 + 1] = lastPx[1];
				lineStart[currX * 4 + 2] = lastPx[2];
				lineStart[currX * 4 + 3] = lastPx[3];
				if (lastPx[2] != 0xff || lastPx[3] != 0xff)
				{
					semiTr = true;
				}
				srcData += 4;
				currX += xAdd;
			}
		}
		else if (px == 2)
		{
			if (lastLineStart)
			{
				while (currX < maxX)
				{
					lineStart[currX * 4 + 0] = (UInt8)(lastLineStart[currX * 4 + 0] + srcData[1]);
					lineStart[currX * 4 + 1] = (UInt8)(lastLineStart[currX * 4 + 1] + srcData[0]);
					a0 = lineStart[currX * 4 + 2] = (UInt8)(lastLineStart[currX * 4 + 2] + srcData[3]);
					a1 = lineStart[currX * 4 + 3] = (UInt8)(lastLineStart[currX * 4 + 3] + srcData[2]);
					if (a0 != 0xff || a1 != 0xff)
					{
						semiTr = true;
					}
					srcData += 4;
					currX += xAdd;
				}
			}
			else
			{
				while (currX < maxX)
				{
					lineStart[currX * 4 + 0] = srcData[1];
					lineStart[currX * 4 + 1] = srcData[0];
					a0 = lineStart[currX * 4 + 2] = srcData[3];
					a1 = lineStart[currX * 4 + 3] = srcData[2];
					if (a0 != 0xff || a1 != 0xff)
					{
						semiTr = true;
					}
					srcData += 4;
					currX += xAdd;
				}
			}
		}
		else if (px == 3)
		{
			if (lastLineStart)
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				lastPx[3] = 0;
				while (currX < maxX)
				{
					lastPx[0] = (UInt8)(((lastPx[0] + lastLineStart[currX * 4 + 0]) >> 1) + srcData[1]);
					lastPx[1] = (UInt8)(((lastPx[1] + lastLineStart[currX * 4 + 1]) >> 1) + srcData[0]);
					lastPx[2] = (UInt8)(((lastPx[2] + lastLineStart[currX * 4 + 2]) >> 1) + srcData[3]);
					lastPx[3] = (UInt8)(((lastPx[3] + lastLineStart[currX * 4 + 3]) >> 1) + srcData[2]);
					lineStart[currX * 4 + 0] = lastPx[0];
					lineStart[currX * 4 + 1] = lastPx[1];
					lineStart[currX * 4 + 2] = lastPx[2];
					lineStart[currX * 4 + 3] = lastPx[3];
					if (lastPx[2] != 0xff || lastPx[3] != 0xff)
					{
						semiTr = true;
					}
					srcData += 4;
					currX += xAdd;
				}
			}
			else
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				lastPx[3] = 0;
				while (currX < maxX)
				{
					lastPx[0] = (UInt8)((lastPx[0] >> 1) + srcData[1]);
					lastPx[1] = (UInt8)((lastPx[1] >> 1) + srcData[0]);
					lastPx[2] = (UInt8)((lastPx[2] >> 1) + srcData[3]);
					lastPx[3] = (UInt8)((lastPx[3] >> 1) + srcData[2]);
					lineStart[currX * 4 + 0] = lastPx[0];
					lineStart[currX * 4 + 1] = lastPx[1];
					lineStart[currX * 4 + 2] = lastPx[2];
					lineStart[currX * 4 + 3] = lastPx[3];
					if (lastPx[2] != 0xff || lastPx[3] != 0xff)
					{
						semiTr = true;
					}
					srcData += 4;
					currX += xAdd;
				}
			}
		}
		else if (px == 4)
		{
			if (lastLineStart)
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				lastPx[3] = 0;
				while (currX < maxX)
				{
					if (currX < xAdd)
					{
						lastPx[0] = (UInt8)(lastLineStart[currX * 4 + 0] + srcData[1]);
						lastPx[1] = (UInt8)(lastLineStart[currX * 4 + 1] + srcData[0]);
						lastPx[2] = (UInt8)(lastLineStart[currX * 4 + 2] + srcData[3]);
						lastPx[3] = (UInt8)(lastLineStart[currX * 4 + 3] + srcData[2]);
					}
					else
					{
						lastPx[0] = (UInt8)(PNGParser_PaethPredictor(lastPx[0], lastLineStart[currX * 4 + 0], lastLineStart[(currX - xAdd) * 4 + 0]) + srcData[1]);
						lastPx[1] = (UInt8)(PNGParser_PaethPredictor(lastPx[1], lastLineStart[currX * 4 + 1], lastLineStart[(currX - xAdd) * 4 + 1]) + srcData[0]);
						lastPx[2] = (UInt8)(PNGParser_PaethPredictor(lastPx[2], lastLineStart[currX * 4 + 2], lastLineStart[(currX - xAdd) * 4 + 2]) + srcData[3]);
						lastPx[3] = (UInt8)(PNGParser_PaethPredictor(lastPx[3], lastLineStart[currX * 4 + 3], lastLineStart[(currX - xAdd) * 4 + 3]) + srcData[2]);
					}
					lineStart[currX * 4 + 0] = lastPx[0];
					lineStart[currX * 4 + 1] = lastPx[1];
					lineStart[currX * 4 + 2] = lastPx[2];
					lineStart[currX * 4 + 3] = lastPx[3];
					if (lastPx[2] != 0xff || lastPx[3] != 0xff)
					{
						semiTr = true;
					}
					srcData += 4;
					currX += xAdd;
				}
			}
			else
			{
				lastPx[0] = 0;
				lastPx[1] = 0;
				lastPx[2] = 0;
				lastPx[3] = 0;
				while (currX < maxX)
				{
					if (currX < xAdd)
					{
						lastPx[0] = srcData[1];
						lastPx[1] = srcData[0];
						lastPx[2] = srcData[3];
						lastPx[3] = srcData[2];
					}
					else
					{
						lastPx[0] = (UInt8)(lastPx[0] + srcData[1]);
						lastPx[1] = (UInt8)(lastPx[1] + srcData[0]);
						lastPx[2] = (UInt8)(lastPx[2] + srcData[3]);
						lastPx[3] = (UInt8)(lastPx[3] + srcData[2]);
					}
					lineStart[currX * 4 + 0] = lastPx[0];
					lineStart[currX * 4 + 1] = lastPx[1];
					lineStart[currX * 4 + 2] = lastPx[2];
					lineStart[currX * 4 + 3] = lastPx[3];
					if (lastPx[2] != 0xff || lastPx[3] != 0xff)
					{
						semiTr = true;
					}
					srcData += 4;
					currX += xAdd;
				}
			}
		}
		else
		{
			while (currX < maxX)
			{
				lineStart[currX * 4 + 0] = srcData[1];
				lineStart[currX * 4 + 1] = srcData[0];
				a0 = lineStart[currX * 4 + 2] = srcData[3];
				a1 = lineStart[currX * 4 + 3] = srcData[2];
				if (a0 != 0xff || a1 != 0xff)
				{
					semiTr = true;
				}
				srcData += 4;
				currX += xAdd;
			}
		}
		currY += yAdd;
		lastLineStart = lineStart;
		lineStart += bpl * yAdd;
	}
	if (semiTr)
	{
		*alphaFound = true;
	}
	return srcData;
}

void Parser::FileParser::PNGParser::ParseImage(UInt8 bitDepth, UInt8 colorType, UInt8 *dataBuff, Media::FrameInfo *info, Media::ImageList *imgList, UInt32 imgDelay, UInt32 imgX, UInt32 imgY, UInt32 imgW, UInt32 imgH, UInt8 interlaceMeth, UInt8 *palette)
{
	Media::StaticImage *simg;
	if (colorType == 0) //Grayscale
	{
		if (bitDepth < 8)
		{
			UOSInt storeWidth = ((info->dispWidth + 15) >> 4) << 4;
			UInt8 *tmpData = MemAllocA(UInt8, storeWidth * info->dispHeight);
			UInt8 *lineStart;
			OSInt pxMask;
			OSInt pxAMask;
			OSInt pxShift;
			if (bitDepth == 1)
			{
				pxMask = 7;
				pxAMask = 1;
				pxShift = 1;
			}
			else if (bitDepth == 2)
			{
				pxMask = 3;
				pxAMask = 3;
				pxShift = 2;
			}
			else if (bitDepth == 4)
			{
				pxMask = 1;
				pxAMask = 15;
				pxShift = 4;
			}
			else
			{
				pxMask = 1;
				pxAMask = 15;
				pxShift = 4;
			}

			MemClear(tmpData, storeWidth * info->dispHeight);
				
			if (interlaceMeth == 1)
			{
				//Pass1
				dataBuff = PNGParser_ParsePixelsBits(dataBuff, tmpData + imgY * storeWidth + imgX, storeWidth, 0, 0, imgW, imgH, 8, 8, pxMask, pxAMask, pxShift);
				//Pass2
				dataBuff = PNGParser_ParsePixelsBits(dataBuff, tmpData + imgY * storeWidth + imgX, storeWidth, 4, 0, imgW, imgH, 8, 8, pxMask, pxAMask, pxShift);
				//Pass3
				dataBuff = PNGParser_ParsePixelsBits(dataBuff, tmpData + imgY * storeWidth + imgX, storeWidth, 0, 4, imgW, imgH, 4, 8, pxMask, pxAMask, pxShift);
				//Pass4
				dataBuff = PNGParser_ParsePixelsBits(dataBuff, tmpData + imgY * storeWidth + imgX, storeWidth, 2, 0, imgW, imgH, 4, 4, pxMask, pxAMask, pxShift);
				//Pass5
				dataBuff = PNGParser_ParsePixelsBits(dataBuff, tmpData + imgY * storeWidth + imgX, storeWidth, 0, 2, imgW, imgH, 2, 4, pxMask, pxAMask, pxShift);
				//Pass6
				dataBuff = PNGParser_ParsePixelsBits(dataBuff, tmpData + imgY * storeWidth + imgX, storeWidth, 1, 0, imgW, imgH, 2, 2, pxMask, pxAMask, pxShift);
				//Pass7
				dataBuff = PNGParser_ParsePixelsBits(dataBuff, tmpData + imgY * storeWidth + imgX, storeWidth, 0, 1, imgW, imgH, 1, 2, pxMask, pxAMask, pxShift);
			}
			else
			{
				dataBuff = PNGParser_ParsePixelsBits(dataBuff, tmpData + imgY * storeWidth + imgX, storeWidth, 0, 0, imgW, imgH, 1, 1, pxMask, pxAMask, pxShift);
			}

			info->atype = Media::AT_NO_ALPHA;
			info->storeWidth = storeWidth;
			UOSInt byteCnt;
			if (bitDepth == 1)
			{
				info->storeBPP = 1;
				info->pf = Media::PF_PAL_W1;
				info->byteSize = storeWidth * info->storeHeight >> 3;
				NEW_CLASS(simg, Media::StaticImage(info));
				WriteUInt32(&simg->pal[0], 0xff000000);
				WriteUInt32(&simg->pal[4], 0xffffffff);

				byteCnt = info->byteSize;
				dataBuff = tmpData;
				lineStart = simg->data;
				while (byteCnt-- > 0)
				{
					*lineStart++ = (UInt8)((dataBuff[0] << 7) | (dataBuff[1] << 6) | (dataBuff[2] << 5) | (dataBuff[3] << 4) | (dataBuff[4] << 3) | (dataBuff[5] << 2) | (dataBuff[6] << 1) | dataBuff[7]);
					dataBuff += 8;
				}

				imgList->AddImage(simg, imgDelay);
			}
			else if (bitDepth == 2)
			{
				info->storeBPP = 2;
				info->pf = Media::PF_PAL_W2;
				info->byteSize = storeWidth * info->storeHeight >> 2;
				NEW_CLASS(simg, Media::StaticImage(info));
				WriteUInt32(&simg->pal[0], 0xff000000);
				WriteUInt32(&simg->pal[4], 0xff555555);
				WriteUInt32(&simg->pal[8], 0xffaaaaaa);
				WriteUInt32(&simg->pal[12], 0xffffffff);

				byteCnt = info->byteSize;
				dataBuff = tmpData;
				lineStart = simg->data;
				while (byteCnt-- > 0)
				{
					*lineStart++ = (UInt8)((dataBuff[0] << 6) | (dataBuff[1] << 4) | (dataBuff[2] << 2) | dataBuff[3]);
					dataBuff += 4;
				}

				imgList->AddImage(simg, imgDelay);
			}
			else if (bitDepth == 4)
			{
				info->storeBPP = 4;
				info->pf = Media::PF_PAL_W4;
				info->byteSize = storeWidth * info->storeHeight >> 1;
				NEW_CLASS(simg, Media::StaticImage(info));
				WriteUInt32(&simg->pal[0], 0xff000000);
				WriteUInt32(&simg->pal[4], 0xff111111);
				WriteUInt32(&simg->pal[8], 0xff222222);
				WriteUInt32(&simg->pal[12], 0xff333333);
				WriteUInt32(&simg->pal[16], 0xff444444);
				WriteUInt32(&simg->pal[20], 0xff555555);
				WriteUInt32(&simg->pal[24], 0xff666666);
				WriteUInt32(&simg->pal[28], 0xff777777);
				WriteUInt32(&simg->pal[32], 0xff888888);
				WriteUInt32(&simg->pal[36], 0xff999999);
				WriteUInt32(&simg->pal[40], 0xffaaaaaa);
				WriteUInt32(&simg->pal[44], 0xffbbbbbb);
				WriteUInt32(&simg->pal[48], 0xffcccccc);
				WriteUInt32(&simg->pal[52], 0xffdddddd);
				WriteUInt32(&simg->pal[56], 0xffeeeeee);
				WriteUInt32(&simg->pal[60], 0xffffffff);

				byteCnt = info->byteSize;
				dataBuff = tmpData;
				lineStart = simg->data;
				while (byteCnt-- > 0)
				{
					*lineStart++ = (UInt8)((dataBuff[0] << 4) | dataBuff[1]);
					dataBuff += 2;
				}

				imgList->AddImage(simg, imgDelay);
			}

			MemFreeA(tmpData);
		}
		else if (bitDepth == 8)
		{
			UInt32 pxId;

			info->atype = Media::AT_NO_ALPHA;
			info->storeWidth = info->dispWidth;
			info->storeBPP = 8;
			info->pf = Media::PF_PAL_W8;
			info->byteSize = info->storeWidth * info->storeHeight;
			NEW_CLASS(simg, Media::StaticImage(info));
			pxId = 0;
			while (pxId < 256)
			{
				WriteUInt32(&simg->pal[pxId << 2], (0xff000000 | (pxId << 16) | (pxId << 8) | pxId));
				pxId++;
			}
			if (info->dispWidth != imgW || info->dispHeight != imgH)
			{
				MemClear(simg->data, info->byteSize);
			}

			if (interlaceMeth == 1)
			{
				//Pass1
				dataBuff = PNGParser_ParsePixelsByte(dataBuff, simg->data + imgY * info->storeWidth + imgX, info->storeWidth, 0, 0, imgW, imgH, 8, 8);
				//Pass2
				dataBuff = PNGParser_ParsePixelsByte(dataBuff, simg->data + imgY * info->storeWidth + imgX, info->storeWidth, 4, 0, imgW, imgH, 8, 8);
				//Pass3
				dataBuff = PNGParser_ParsePixelsByte(dataBuff, simg->data + imgY * info->storeWidth + imgX, info->storeWidth, 0, 4, imgW, imgH, 4, 8);
				//Pass4
				dataBuff = PNGParser_ParsePixelsByte(dataBuff, simg->data + imgY * info->storeWidth + imgX, info->storeWidth, 2, 0, imgW, imgH, 4, 4);
				//Pass5
				dataBuff = PNGParser_ParsePixelsByte(dataBuff, simg->data + imgY * info->storeWidth + imgX, info->storeWidth, 0, 2, imgW, imgH, 2, 4);
				//Pass6
				dataBuff = PNGParser_ParsePixelsByte(dataBuff, simg->data + imgY * info->storeWidth + imgX, info->storeWidth, 1, 0, imgW, imgH, 2, 2);
				//Pass7
				dataBuff = PNGParser_ParsePixelsByte(dataBuff, simg->data + imgY * info->storeWidth + imgX, info->storeWidth, 0, 1, imgW, imgH, 1, 2);
			}
			else
			{
				dataBuff = PNGParser_ParsePixelsByte(dataBuff, simg->data + imgY * info->storeWidth + imgX, info->storeWidth, 0, 0, imgW, imgH, 1, 1);
			}

			imgList->AddImage(simg, imgDelay);
		}
		else if (bitDepth == 16)
		{
			UOSInt lineAdd;

			info->atype = Media::AT_NO_ALPHA;
			info->storeBPP = 16;
			info->pf = Media::PF_LE_W16;
			info->byteSize = info->storeWidth * info->storeHeight * 2;
			lineAdd = imgList->GetCount();
			if (lineAdd > 0)
			{
				simg = (Media::StaticImage*)imgList->GetImage(lineAdd - 1, 0);
				simg = (Media::StaticImage*)simg->Clone();
			}
			else
			{
				NEW_CLASS(simg, Media::StaticImage(info));
			}

			UOSInt bpl = simg->GetDataBpl();
			if (interlaceMeth == 1)
			{
				//Pass1
				dataBuff = PNGParser_ParsePixelsWord(dataBuff, simg->data + imgY * bpl + imgX * 2, bpl, 0, 0, imgW, imgH, 8, 8);
				//Pass2
				dataBuff = PNGParser_ParsePixelsWord(dataBuff, simg->data + imgY * bpl + imgX * 2, bpl, 4, 0, imgW, imgH, 8, 8);
				//Pass3
				dataBuff = PNGParser_ParsePixelsWord(dataBuff, simg->data + imgY * bpl + imgX * 2, bpl, 0, 4, imgW, imgH, 4, 8);
				//Pass4
				dataBuff = PNGParser_ParsePixelsWord(dataBuff, simg->data + imgY * bpl + imgX * 2, bpl, 2, 0, imgW, imgH, 4, 4);
				//Pass5
				dataBuff = PNGParser_ParsePixelsWord(dataBuff, simg->data + imgY * bpl + imgX * 2, bpl, 0, 2, imgW, imgH, 2, 4);
				//Pass6
				dataBuff = PNGParser_ParsePixelsWord(dataBuff, simg->data + imgY * bpl + imgX * 2, bpl, 1, 0, imgW, imgH, 2, 2);
				//Pass7
				dataBuff = PNGParser_ParsePixelsWord(dataBuff, simg->data + imgY * bpl + imgX * 2, bpl, 0, 1, imgW, imgH, 1, 2);
			}
			else
			{
				dataBuff = PNGParser_ParsePixelsWord(dataBuff, simg->data + imgY * bpl + imgX * 2, bpl, 0, 0, imgW, imgH, 1, 1);
			}
			imgList->AddImage(simg, imgDelay);
		}
	}
	else if (colorType == 2) //RGB
	{
		if (bitDepth == 8)
		{
			UOSInt lineAdd;

			info->atype = Media::AT_NO_ALPHA;
			info->storeBPP = 24;
			info->pf = Media::PF_B8G8R8;
			info->byteSize = info->storeWidth * info->storeHeight * 3;
			lineAdd = imgList->GetCount();
			if (lineAdd > 0)
			{
				simg = (Media::StaticImage*)imgList->GetImage(lineAdd - 1, 0);
				simg = (Media::StaticImage*)simg->Clone();
			}
			else
			{
				NEW_CLASS(simg, Media::StaticImage(info));
			}
			UOSInt bpl = simg->GetDataBpl();
			if (interlaceMeth == 1)
			{
				//Pass1
				dataBuff = PNGParser_ParsePixelsRGB24(dataBuff, simg->data + imgY * bpl + imgX * 3, bpl, 0, 0, imgW, imgH, 8, 8);
				//Pass2
				dataBuff = PNGParser_ParsePixelsRGB24(dataBuff, simg->data + imgY * bpl + imgX * 3, bpl, 4, 0, imgW, imgH, 8, 8);
				//Pass3
				dataBuff = PNGParser_ParsePixelsRGB24(dataBuff, simg->data + imgY * bpl + imgX * 3, bpl, 0, 4, imgW, imgH, 4, 8);
				//Pass4
				dataBuff = PNGParser_ParsePixelsRGB24(dataBuff, simg->data + imgY * bpl + imgX * 3, bpl, 2, 0, imgW, imgH, 4, 4);
				//Pass5
				dataBuff = PNGParser_ParsePixelsRGB24(dataBuff, simg->data + imgY * bpl + imgX * 3, bpl, 0, 2, imgW, imgH, 2, 4);
				//Pass6
				dataBuff = PNGParser_ParsePixelsRGB24(dataBuff, simg->data + imgY * bpl + imgX * 3, bpl, 1, 0, imgW, imgH, 2, 2);
				//Pass7
				dataBuff = PNGParser_ParsePixelsRGB24(dataBuff, simg->data + imgY * bpl + imgX * 3, bpl, 0, 1, imgW, imgH, 1, 2);
			}
			else
			{
				dataBuff = PNGParser_ParsePixelsRGB24(dataBuff, simg->data + imgY * bpl + imgX * 3, bpl, 0, 0, imgW, imgH, 1, 1);
			}
			imgList->AddImage(simg, imgDelay);
		}
		else if (bitDepth == 16)
		{
			UOSInt lineAdd;

			info->atype = Media::AT_NO_ALPHA;
			info->storeBPP = 48;
			info->pf = Media::PF_LE_B16G16R16;
			info->byteSize = info->storeWidth * info->storeHeight * 3;
			lineAdd = imgList->GetCount();
			if (lineAdd > 0)
			{
				simg = (Media::StaticImage*)imgList->GetImage(lineAdd - 1, 0);
				simg = (Media::StaticImage*)simg->Clone();
			}
			else
			{
				NEW_CLASS(simg, Media::StaticImage(info));
			}
			UOSInt bpl = simg->GetDataBpl();
			if (interlaceMeth == 1)
			{
				//Pass1
				dataBuff = PNGParser_ParsePixelsRGB48(dataBuff, simg->data + imgY * bpl + imgX * 6, bpl, 0, 0, imgW, imgH, 8, 8);
				//Pass2
				dataBuff = PNGParser_ParsePixelsRGB48(dataBuff, simg->data + imgY * bpl + imgX * 6, bpl, 4, 0, imgW, imgH, 8, 8);
				//Pass3
				dataBuff = PNGParser_ParsePixelsRGB48(dataBuff, simg->data + imgY * bpl + imgX * 6, bpl, 0, 4, imgW, imgH, 4, 8);
				//Pass4
				dataBuff = PNGParser_ParsePixelsRGB48(dataBuff, simg->data + imgY * bpl + imgX * 6, bpl, 2, 0, imgW, imgH, 4, 4);
				//Pass5
				dataBuff = PNGParser_ParsePixelsRGB48(dataBuff, simg->data + imgY * bpl + imgX * 6, bpl, 0, 2, imgW, imgH, 2, 4);
				//Pass6
				dataBuff = PNGParser_ParsePixelsRGB48(dataBuff, simg->data + imgY * bpl + imgX * 6, bpl, 1, 0, imgW, imgH, 2, 2);
				//Pass7
				dataBuff = PNGParser_ParsePixelsRGB48(dataBuff, simg->data + imgY * bpl + imgX * 6, bpl, 0, 1, imgW, imgH, 1, 2);
			}
			else
			{
				dataBuff = PNGParser_ParsePixelsRGB48(dataBuff, simg->data + imgY * bpl + imgX * 6, bpl, 0, 0, imgW, imgH, 1, 1);
			}
			imgList->AddImage(simg, imgDelay);
		}
	}
	else if (colorType == 3 && palette != 0) //Indexed
	{
		if (bitDepth < 8)
		{
			UOSInt storeWidth = ((info->dispWidth + 15) >> 4) << 4;
			UInt8 *tmpData = MemAllocA(UInt8, storeWidth * info->dispHeight);
			UInt8 *lineStart;
			OSInt pxMask;
			OSInt pxAMask;
			OSInt pxShift;
			if (bitDepth == 1)
			{
				pxMask = 7;
				pxAMask = 1;
				pxShift = 1;
			}
			else if (bitDepth == 2)
			{
				pxMask = 3;
				pxAMask = 3;
				pxShift = 2;
			}
			else if (bitDepth == 4)
			{
				pxMask = 1;
				pxAMask = 15;
				pxShift = 4;
			}
			else
			{
				pxMask = 1;
				pxAMask = 15;
				pxShift = 4;
			}

			MemClear(tmpData, storeWidth * info->dispHeight);
				
			if (interlaceMeth == 1)
			{
				//Pass1
				dataBuff = PNGParser_ParsePixelsBits(dataBuff, tmpData + imgY * storeWidth + imgX, storeWidth, 0, 0, imgW, imgH, 8, 8, pxMask, pxAMask, pxShift);
				//Pass2
				dataBuff = PNGParser_ParsePixelsBits(dataBuff, tmpData + imgY * storeWidth + imgX, storeWidth, 4, 0, imgW, imgH, 8, 8, pxMask, pxAMask, pxShift);
				//Pass3
				dataBuff = PNGParser_ParsePixelsBits(dataBuff, tmpData + imgY * storeWidth + imgX, storeWidth, 0, 4, imgW, imgH, 4, 8, pxMask, pxAMask, pxShift);
				//Pass4
				dataBuff = PNGParser_ParsePixelsBits(dataBuff, tmpData + imgY * storeWidth + imgX, storeWidth, 2, 0, imgW, imgH, 4, 4, pxMask, pxAMask, pxShift);
				//Pass5
				dataBuff = PNGParser_ParsePixelsBits(dataBuff, tmpData + imgY * storeWidth + imgX, storeWidth, 0, 2, imgW, imgH, 2, 4, pxMask, pxAMask, pxShift);
				//Pass6
				dataBuff = PNGParser_ParsePixelsBits(dataBuff, tmpData + imgY * storeWidth + imgX, storeWidth, 1, 0, imgW, imgH, 2, 2, pxMask, pxAMask, pxShift);
				//Pass7
				dataBuff = PNGParser_ParsePixelsBits(dataBuff, tmpData + imgY * storeWidth + imgX, storeWidth, 0, 1, imgW, imgH, 1, 2, pxMask, pxAMask, pxShift);
			}
			else
			{
				dataBuff = PNGParser_ParsePixelsBits(dataBuff, tmpData + imgY * storeWidth + imgX, storeWidth, 0, 0, imgW, imgH, 1, 1, pxMask, pxAMask, pxShift);
			}

			info->atype = Media::AT_NO_ALPHA;
			info->storeWidth = storeWidth;
			UOSInt byteCnt;
			if (bitDepth == 1)
			{
				info->storeBPP = 1;
				info->pf = Media::PF_PAL_1;
				info->byteSize = storeWidth * info->storeHeight >> 3;
				NEW_CLASS(simg, Media::StaticImage(info));
				MemCopyNO(simg->pal, palette, 8);

				byteCnt = info->byteSize;
				dataBuff = tmpData;
				lineStart = simg->data;
				while (byteCnt-- > 0)
				{
					*lineStart++ = (UInt8)((dataBuff[0] << 7) | (dataBuff[1] << 6) | (dataBuff[2] << 5) | (dataBuff[3] << 4) | (dataBuff[4] << 3) | (dataBuff[5] << 2) | (dataBuff[6] << 1) | dataBuff[7]);
					dataBuff += 8;
				}

				imgList->AddImage(simg, imgDelay);
			}
			else if (bitDepth == 2)
			{
				info->storeBPP = 2;
				info->pf = Media::PF_PAL_2;
				info->byteSize = storeWidth * info->storeHeight >> 2;
				NEW_CLASS(simg, Media::StaticImage(info));
				MemCopyNO(simg->pal, palette, 16);

				byteCnt = info->byteSize;
				dataBuff = tmpData;
				lineStart = simg->data;
				while (byteCnt-- > 0)
				{
					*lineStart++ = (UInt8)((dataBuff[0] << 6) | (dataBuff[1] << 4) | (dataBuff[2] << 2) | dataBuff[3]);
					dataBuff += 4;
				}

				imgList->AddImage(simg, imgDelay);
			}
			else if (bitDepth == 4)
			{
				info->storeBPP = 4;
				info->pf = Media::PF_PAL_4;
				info->byteSize = storeWidth * info->storeHeight >> 1;
				NEW_CLASS(simg, Media::StaticImage(info));
				MemCopyNO(simg->pal, palette, 64);

				byteCnt = info->byteSize;
				dataBuff = tmpData;
				lineStart = simg->data;
				while (byteCnt-- > 0)
				{
					*lineStart++ = (UInt8)((dataBuff[0] << 4) | dataBuff[1]);
					dataBuff += 2;
				}

				imgList->AddImage(simg, imgDelay);
			}

			MemFreeA(tmpData);
		}
		else if (bitDepth == 8)
		{
			info->atype = Media::AT_NO_ALPHA;
			info->storeWidth = info->dispWidth;
			info->storeBPP = 8;
			info->pf = Media::PF_PAL_8;
			info->byteSize = info->storeWidth * info->storeHeight;
			NEW_CLASS(simg, Media::StaticImage(info));
			MemCopyNO(simg->pal, palette, 1024);

			if (interlaceMeth == 1)
			{
				//Pass1
				dataBuff = PNGParser_ParsePixelsByte(dataBuff, simg->data + imgY * info->storeWidth + imgX, info->storeWidth, 0, 0, imgW, imgH, 8, 8);
				//Pass2
				dataBuff = PNGParser_ParsePixelsByte(dataBuff, simg->data + imgY * info->storeWidth + imgX, info->storeWidth, 4, 0, imgW, imgH, 8, 8);
				//Pass3
				dataBuff = PNGParser_ParsePixelsByte(dataBuff, simg->data + imgY * info->storeWidth + imgX, info->storeWidth, 0, 4, imgW, imgH, 4, 8);
				//Pass4
				dataBuff = PNGParser_ParsePixelsByte(dataBuff, simg->data + imgY * info->storeWidth + imgX, info->storeWidth, 2, 0, imgW, imgH, 4, 4);
				//Pass5
				dataBuff = PNGParser_ParsePixelsByte(dataBuff, simg->data + imgY * info->storeWidth + imgX, info->storeWidth, 0, 2, imgW, imgH, 2, 4);
				//Pass6
				dataBuff = PNGParser_ParsePixelsByte(dataBuff, simg->data + imgY * info->storeWidth + imgX, info->storeWidth, 1, 0, imgW, imgH, 2, 2);
				//Pass7
				dataBuff = PNGParser_ParsePixelsByte(dataBuff, simg->data + imgY * info->storeWidth + imgX, info->storeWidth, 0, 1, imgW, imgH, 1, 2);
			}
			else
			{
				dataBuff = PNGParser_ParsePixelsByte(dataBuff, simg->data + imgY * info->storeWidth + imgX, info->storeWidth, 0, 0, imgW, imgH, 1, 1);
			}

			imgList->AddImage(simg, imgDelay);
		}
	}
	else if (colorType == 4) //Alpha + White
	{
		Bool semiTr = false;
		if (bitDepth == 8)
		{
			UOSInt lineAdd;

			info->atype = Media::AT_ALPHA;
			info->storeBPP = 16;
			info->pf = Media::PF_W8A8;
			info->byteSize = info->storeWidth * info->storeHeight * 2;
			lineAdd = imgList->GetCount();
			if (lineAdd > 0)
			{
				simg = (Media::StaticImage*)imgList->GetImage(lineAdd - 1, 0);
				simg = (Media::StaticImage*)simg->Clone();
				simg->FillColor(0);
			}
			else
			{
				NEW_CLASS(simg, Media::StaticImage(info));
			}
			UOSInt bpl = simg->GetDataBpl();
			if (interlaceMeth == 1)
			{
				//Pass1
				dataBuff = PNGParser_ParsePixelsAW16(dataBuff, simg->data + imgY * bpl + imgX * 2, bpl, 0, 0, imgW, imgH, 8, 8, &semiTr);
				//Pass2
				dataBuff = PNGParser_ParsePixelsAW16(dataBuff, simg->data + imgY * bpl + imgX * 2, bpl, 4, 0, imgW, imgH, 8, 8, &semiTr);
				//Pass3
				dataBuff = PNGParser_ParsePixelsAW16(dataBuff, simg->data + imgY * bpl + imgX * 2, bpl, 0, 4, imgW, imgH, 4, 8, &semiTr);
				//Pass4
				dataBuff = PNGParser_ParsePixelsAW16(dataBuff, simg->data + imgY * bpl + imgX * 2, bpl, 2, 0, imgW, imgH, 4, 4, &semiTr);
				//Pass5
				dataBuff = PNGParser_ParsePixelsAW16(dataBuff, simg->data + imgY * bpl + imgX * 2, bpl, 0, 2, imgW, imgH, 2, 4, &semiTr);
				//Pass6
				dataBuff = PNGParser_ParsePixelsAW16(dataBuff, simg->data + imgY * bpl + imgX * 2, bpl, 1, 0, imgW, imgH, 2, 2, &semiTr);
				//Pass7
				dataBuff = PNGParser_ParsePixelsAW16(dataBuff, simg->data + imgY * bpl + imgX * 2, bpl, 0, 1, imgW, imgH, 1, 2, &semiTr);
			}
			else
			{
				dataBuff = PNGParser_ParsePixelsAW16(dataBuff, simg->data + imgY * bpl + imgX * 2, bpl, 0, 0, imgW, imgH, 1, 1, &semiTr);
			}
			if (!semiTr)
			{
				simg->info->atype = Media::AT_NO_ALPHA;
			}
			imgList->AddImage(simg, imgDelay);
		}
		else if (bitDepth == 16)
		{
			UOSInt lineAdd;

			info->atype = Media::AT_ALPHA;
			info->storeBPP = 32;
			info->pf = Media::PF_LE_W16A16;
			info->byteSize = info->storeWidth * info->storeHeight * 4;
			lineAdd = imgList->GetCount();
			if (lineAdd > 0)
			{
				simg = (Media::StaticImage*)imgList->GetImage(lineAdd - 1, 0);
				simg = (Media::StaticImage*)simg->Clone();
				simg->FillColor(0);
			}
			else
			{
				NEW_CLASS(simg, Media::StaticImage(info));
			}
			UOSInt bpl = simg->GetDataBpl();
			if (interlaceMeth == 1)
			{
				//Pass1
				dataBuff = PNGParser_ParsePixelsAW32(dataBuff, simg->data + imgY * bpl + imgX * 4, bpl, 0, 0, imgW, imgH, 8, 8, &semiTr);
				//Pass2
				dataBuff = PNGParser_ParsePixelsAW32(dataBuff, simg->data + imgY * bpl + imgX * 4, bpl, 4, 0, imgW, imgH, 8, 8, &semiTr);
				//Pass3
				dataBuff = PNGParser_ParsePixelsAW32(dataBuff, simg->data + imgY * bpl + imgX * 4, bpl, 0, 4, imgW, imgH, 4, 8, &semiTr);
				//Pass4
				dataBuff = PNGParser_ParsePixelsAW32(dataBuff, simg->data + imgY * bpl + imgX * 4, bpl, 2, 0, imgW, imgH, 4, 4, &semiTr);
				//Pass5
				dataBuff = PNGParser_ParsePixelsAW32(dataBuff, simg->data + imgY * bpl + imgX * 4, bpl, 0, 2, imgW, imgH, 2, 4, &semiTr);
				//Pass6
				dataBuff = PNGParser_ParsePixelsAW32(dataBuff, simg->data + imgY * bpl + imgX * 4, bpl, 1, 0, imgW, imgH, 2, 2, &semiTr);
				//Pass7
				dataBuff = PNGParser_ParsePixelsAW32(dataBuff, simg->data + imgY * bpl + imgX * 4, bpl, 0, 1, imgW, imgH, 1, 2, &semiTr);
			}
			else
			{
				dataBuff = PNGParser_ParsePixelsAW32(dataBuff, simg->data + imgY * bpl + imgX * 4, bpl, 0, 0, imgW, imgH, 1, 1, &semiTr);
			}
			if (!semiTr)
			{
				simg->info->atype = Media::AT_NO_ALPHA;
			}
			imgList->AddImage(simg, imgDelay);
		}
	}
	else if (colorType == 6) //ARGB
	{
		Bool semiTr = false;
		if (bitDepth == 8)
		{
			UOSInt lineAdd;

			info->atype = Media::AT_ALPHA;
			info->storeBPP = 32;
			info->pf = Media::PF_B8G8R8A8;
			info->byteSize = info->storeWidth * info->storeHeight * 4;
			lineAdd = imgList->GetCount();
			if (lineAdd > 0)
			{
				simg = (Media::StaticImage*)imgList->GetImage(lineAdd - 1, 0);
				simg = (Media::StaticImage*)simg->Clone();
				simg->FillColor(0);
			}
			else
			{
				NEW_CLASS(simg, Media::StaticImage(info));
			}
			UOSInt bpl = simg->GetDataBpl();
			if (interlaceMeth == 1)
			{
				//Pass1
				dataBuff = PNGParser_ParsePixelsARGB32(dataBuff, simg->data + imgY * bpl + imgX * 4, bpl, 0, 0, imgW, imgH, 8, 8, &semiTr);
				//Pass2
				dataBuff = PNGParser_ParsePixelsARGB32(dataBuff, simg->data + imgY * bpl + imgX * 4, bpl, 4, 0, imgW, imgH, 8, 8, &semiTr);
				//Pass3
				dataBuff = PNGParser_ParsePixelsARGB32(dataBuff, simg->data + imgY * bpl + imgX * 4, bpl, 0, 4, imgW, imgH, 4, 8, &semiTr);
				//Pass4
				dataBuff = PNGParser_ParsePixelsARGB32(dataBuff, simg->data + imgY * bpl + imgX * 4, bpl, 2, 0, imgW, imgH, 4, 4, &semiTr);
				//Pass5
				dataBuff = PNGParser_ParsePixelsARGB32(dataBuff, simg->data + imgY * bpl + imgX * 4, bpl, 0, 2, imgW, imgH, 2, 4, &semiTr);
				//Pass6
				dataBuff = PNGParser_ParsePixelsARGB32(dataBuff, simg->data + imgY * bpl + imgX * 4, bpl, 1, 0, imgW, imgH, 2, 2, &semiTr);
				//Pass7
				dataBuff = PNGParser_ParsePixelsARGB32(dataBuff, simg->data + imgY * bpl + imgX * 4, bpl, 0, 1, imgW, imgH, 1, 2, &semiTr);
			}
			else
			{
				dataBuff = PNGParser_ParsePixelsARGB32(dataBuff, simg->data + imgY * bpl + imgX * 4, bpl, 0, 0, imgW, imgH, 1, 1, &semiTr);
			}
			if (!semiTr)
			{
				simg->info->atype = Media::AT_NO_ALPHA;
			}
			imgList->AddImage(simg, imgDelay);
		}
		else if (bitDepth == 16)
		{
			UOSInt lineAdd;

			info->atype = Media::AT_ALPHA;
			info->storeBPP = 64;
			info->pf = Media::PF_LE_B16G16R16A16;
			info->byteSize = info->storeWidth * info->storeHeight * 8;
			lineAdd = imgList->GetCount();
			if (lineAdd > 0)
			{
				simg = (Media::StaticImage*)imgList->GetImage(lineAdd - 1, 0);
				simg = (Media::StaticImage*)simg->Clone();
				simg->FillColor(0);
			}
			else
			{
				NEW_CLASS(simg, Media::StaticImage(info));
			}
			UOSInt bpl = simg->GetDataBpl();
			if (interlaceMeth == 1)
			{
				//Pass1
				dataBuff = PNGParser_ParsePixelsARGB64(dataBuff, simg->data + imgY * bpl + imgX * 8, bpl, 0, 0, imgW, imgH, 8, 8, &semiTr);
				//Pass2
				dataBuff = PNGParser_ParsePixelsARGB64(dataBuff, simg->data + imgY * bpl + imgX * 8, bpl, 4, 0, imgW, imgH, 8, 8, &semiTr);
				//Pass3
				dataBuff = PNGParser_ParsePixelsARGB64(dataBuff, simg->data + imgY * bpl + imgX * 8, bpl, 0, 4, imgW, imgH, 4, 8, &semiTr);
				//Pass4
				dataBuff = PNGParser_ParsePixelsARGB64(dataBuff, simg->data + imgY * bpl + imgX * 8, bpl, 2, 0, imgW, imgH, 4, 4, &semiTr);
				//Pass5
				dataBuff = PNGParser_ParsePixelsARGB64(dataBuff, simg->data + imgY * bpl + imgX * 8, bpl, 0, 2, imgW, imgH, 2, 4, &semiTr);
				//Pass6
				dataBuff = PNGParser_ParsePixelsARGB64(dataBuff, simg->data + imgY * bpl + imgX * 8, bpl, 1, 0, imgW, imgH, 2, 2, &semiTr);
				//Pass7
				dataBuff = PNGParser_ParsePixelsARGB64(dataBuff, simg->data + imgY * bpl + imgX * 8, bpl, 0, 1, imgW, imgH, 1, 2, &semiTr);
			}
			else
			{
				dataBuff = PNGParser_ParsePixelsARGB64(dataBuff, simg->data + imgY * bpl + imgX * 8, bpl, 0, 0, imgW, imgH, 1, 1, &semiTr);
			}
			if (!semiTr)
			{
				simg->info->atype = Media::AT_NO_ALPHA;
			}
			imgList->AddImage(simg, imgDelay);
		}
	}
}

Parser::FileParser::PNGParser::PNGParser()
{
}

Parser::FileParser::PNGParser::~PNGParser()
{
}

Int32 Parser::FileParser::PNGParser::GetName()
{
	return *(Int32*)"PNGP";
}

void Parser::FileParser::PNGParser::PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t)
{
	if (t == IO::ParsedObject::PT_UNKNOWN || t == IO::ParsedObject::PT_IMAGE_LIST_PARSER)
	{
		selector->AddFilter((const UTF8Char*)"*.png", (const UTF8Char*)"Portable Network Graphic File");
	}
}

IO::ParsedObject::ParserType Parser::FileParser::PNGParser::GetParserType()
{
	return IO::ParsedObject::PT_IMAGE_LIST_PARSER;
}

IO::ParsedObject *Parser::FileParser::PNGParser::ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType)
{
	UInt8 buff[12];
	UInt8 *chunkData;
	UInt64 ofst;
	if (fd->GetRealData(0, 8, buff) != 8)
		return 0;

	if (buff[0] != 0x89 && buff[1] != 0x50 && buff[2] != 0x4e && buff[3] != 0x47 && buff[4] != 0x0d && buff[5] != 0x0a && buff[6] != 0x1a && buff[7] != 0x0a)
	{
		return 0;
	}
	Media::FrameInfo info;
	Media::ImageList *imgList;
	Bool ihdrFound = false;
	Bool iccpFound = false;
	Bool srgbFound = false;
	UInt32 size;
	UInt8 bitDepth = 0;
	UInt8 colorType = 0;
//	UInt8 compMeth;
//	UInt8 filterMeth;
	UInt8 interlaceMeth = 0;
	IO::MemoryStream *mstm = 0;
	Data::Compress::InflateStream *cstm = 0;
	IO::WriteCacheStream *wcstm = 0;
	UOSInt imgSize = 0;
	UInt32 imgDelay = 0;
	UInt32 imgX = 0;
	UInt32 imgY = 0;
	UInt32 imgW = 0;
	UInt32 imgH = 0;
	UInt8 *palette = 0;

	NEW_CLASS(imgList, Media::ImageList(fd->GetFullFileName()));

	ofst = 8;
	while (true)
	{
		if (fd->GetRealData(ofst, 8, buff) != 8)
		{
			break;
		}
		size = ReadMUInt32(&buff[0]);
		if (*(Int32*)&buff[4] == *(Int32*)"IHDR")
		{
			if (size >= 13)
			{
				chunkData = MemAlloc(UInt8, size);
				if (fd->GetRealData(ofst + 8, size, chunkData) == size)
				{
					info.dispWidth = ReadMUInt32(&chunkData[0]);
					info.dispHeight = ReadMUInt32(&chunkData[4]);
					info.storeWidth = info.dispWidth;
					info.storeHeight = info.dispHeight;
					imgW = (UInt32)info.dispWidth;
					imgH = (UInt32)info.dispHeight;
					bitDepth = chunkData[8];
					colorType = chunkData[9];
//					compMeth = chunkData[10];
//					filterMeth = chunkData[11];
					interlaceMeth = chunkData[12];

					info.par2 = 1.0;
					info.hdpi = 72.0;
					info.vdpi = 72.0;
					info.fourcc = 0;
					info.ftype = Media::FT_NON_INTERLACE;
					info.ycOfst = Media::YCOFST_C_CENTER_LEFT;
					
					ihdrFound = true;
				}
				MemFree(chunkData);
			}
		}
		else if (*(Int32*)&buff[4] == *(Int32*)"pHYs")
		{
			if (size >= 9)
			{
				chunkData = MemAlloc(UInt8, size);
				if (fd->GetRealData(ofst + 8, size, chunkData) == size)
				{
					if (chunkData[8] == 1)
					{
						info.hdpi = 1.0 / Math::Unit::Distance::Convert(Math::Unit::Distance::DU_METER, Math::Unit::Distance::DU_INCH, 1.0 / ReadMInt32(&chunkData[0]));
						info.par2 = info.hdpi * Math::Unit::Distance::Convert(Math::Unit::Distance::DU_METER, Math::Unit::Distance::DU_INCH, 1.0 / ReadMInt32(&chunkData[4]));
						info.vdpi = info.hdpi / info.par2;
					}
					else if (chunkData[8] == 0)
					{
						info.par2 = ReadMInt32(&chunkData[0]) / (Double)ReadMInt32(&chunkData[4]);
						if (info.par2 > 1)
						{
							info.vdpi = 72.0 / info.par2;
						}
						else
						{
							info.hdpi = 72.0 * info.par2;
						}
					}
				}
				MemFree(chunkData);
			}
		}
		else if (*(Int32*)&buff[4] == *(Int32*)"PLTE")
		{
			if (size <= 768 && (size % 3) == 0)
			{
				chunkData = MemAlloc(UInt8, size);
				if (fd->GetRealData(ofst + 8, size, chunkData) == size)
				{
					if (palette == 0)
					{
						palette = MemAlloc(UInt8, 1024);
					}
					UInt8 *palPtr = palette;
					UInt8 *dataBuff = chunkData;
					UOSInt sizeLeft = size;
					while (sizeLeft > 0)
					{
						palPtr[0] = dataBuff[2];
						palPtr[1] = dataBuff[1];
						palPtr[2] = dataBuff[0];
						palPtr[3] = 0xff;
						palPtr += 4;
						dataBuff += 3;
						sizeLeft -= 3;
					}
				}
				MemFree(chunkData);
			}
		}
		else if (*(Int32*)&buff[4] == *(Int32*)"iCCP")
		{
			chunkData = MemAlloc(UInt8, size);
			if (fd->GetRealData(ofst + 8, size, chunkData) == size)
			{
				UOSInt i = Text::StrCharCnt((Char*)chunkData) + 1;
				if (chunkData[i] == 0)
				{
					IO::MemoryStream *mstm;
					NEW_CLASS(mstm, IO::MemoryStream((const UTF8Char*)"IO.FileAnalyse.PNGFileAnalyse"));
					Data::Compress::InflateStream cstm(mstm);
					if (cstm.Write(&chunkData[i + 3], size - i - 7) == (size - i - 7))
					{
						UOSInt iccSize;
						UInt8 *iccBuff = mstm->GetBuff(&iccSize);
						Media::ICCProfile *icc = Media::ICCProfile::Parse(iccBuff, iccSize);
						if (icc)
						{
							icc->GetColorPrimaries(&info.color->primaries);
							icc->GetRedTransferParam(info.color->rtransfer);
							icc->GetGreenTransferParam(info.color->gtransfer);
							icc->GetBlueTransferParam(info.color->btransfer);
							info.color->SetRAWICC(iccBuff);
							DEL_CLASS(icc);

							iccpFound = true;
						}
					}
					DEL_CLASS(mstm);
				}
			}
			MemFree(chunkData);
		}
		else if (*(Int32*)&buff[4] == *(Int32*)"sRGB")
		{
			if (!iccpFound)
			{
				info.color->SetCommonProfile(Media::ColorProfile::CPT_SRGB);
				srgbFound = true;
			}
		}
		else if (*(Int32*)&buff[4] == *(Int32*)"gAMA")
		{
			if (!iccpFound && !srgbFound && size >= 4)
			{
				chunkData = MemAlloc(UInt8, size);
				if (fd->GetRealData(ofst + 8, size, chunkData) == size)
				{
					Double g = (Double)ReadMInt32(chunkData);
					if (g != 0)
					{
						g = 100000 / g;
						info.color->rtransfer->Set(Media::CS::TRANT_GAMMA, g);
						info.color->gtransfer->Set(Media::CS::TRANT_GAMMA, g);
						info.color->btransfer->Set(Media::CS::TRANT_GAMMA, g);
					}
				}
				MemFree(chunkData);
			}
		}
		else if (*(Int32*)&buff[4] == *(Int32*)"cHRM")
		{
			if (!iccpFound && !srgbFound && size >= 32)
			{
				chunkData = MemAlloc(UInt8, size);
				if (fd->GetRealData(ofst + 8, size, chunkData) == size)
				{
					info.color->primaries.SetColorType(Media::ColorProfile::CT_CUSTOM);
					info.color->primaries.wx = ReadMInt32(&chunkData[0]) / 100000.0;
					info.color->primaries.wy = ReadMInt32(&chunkData[4]) / 100000.0;
					info.color->primaries.rx = ReadMInt32(&chunkData[8]) / 100000.0;
					info.color->primaries.ry = ReadMInt32(&chunkData[12]) / 100000.0;
					info.color->primaries.gx = ReadMInt32(&chunkData[16]) / 100000.0;
					info.color->primaries.gy = ReadMInt32(&chunkData[20]) / 100000.0;
					info.color->primaries.bx = ReadMInt32(&chunkData[24]) / 100000.0;
					info.color->primaries.by = ReadMInt32(&chunkData[28]) / 100000.0;
					if (info.color->primaries.rx == info.color->primaries.gx && info.color->primaries.ry == info.color->primaries.gy)
					{
						info.color->primaries.SetColorType(Media::ColorProfile::CT_SRGB);
					}
				}
				MemFree(chunkData);
			}
		}
		else if (*(Int32*)&buff[4] == *(Int32*)"fcTL")
		{
			if (size >= 26)
			{
				if (mstm)
				{
					UOSInt dataSize;
					UInt8 *dataBuff = mstm->GetBuff(&dataSize);
					if (dataSize == imgSize || imgSize != 0)
					{
						ParseImage(bitDepth, colorType, dataBuff, &info, imgList, imgDelay, imgX, imgY, imgW, imgH, interlaceMeth, palette);
					}
					DEL_CLASS(wcstm);
					wcstm = 0;
					DEL_CLASS(cstm);
					cstm = 0;
					DEL_CLASS(mstm);
					mstm = 0;
				}

				chunkData = MemAlloc(UInt8, size);
				if (fd->GetRealData(ofst + 8, size, chunkData) == size)
				{
					imgDelay = 1000 * (UInt32)ReadMUInt16(&chunkData[20]) / ReadMUInt16(&chunkData[22]);
					imgX = ReadMUInt32(&chunkData[12]);
					imgY = ReadMUInt32(&chunkData[16]);
					imgW = ReadMUInt32(&chunkData[4]);
					imgH = ReadMUInt32(&chunkData[8]);
				}
				MemFree(chunkData);
			}
		}
		else if (*(Int32*)&buff[4] == *(Int32*)"sBIT")
		{
		}
		else if (*(Int32*)&buff[4] == *(Int32*)"IDAT")
		{
			if (ihdrFound)
			{
				chunkData = MemAlloc(UInt8, size);
				if (fd->GetRealData(ofst + 8, size, chunkData) == size)
				{
					if (mstm == 0)
					{
						imgSize = CalcImageSize(imgW, imgH, bitDepth, colorType, interlaceMeth);
						if (imgSize)
						{
							NEW_CLASS(mstm, IO::MemoryStream(imgSize, (const UTF8Char*)"Parser.FileParser.PNGParser.mstm"));
						}
						else
						{
							NEW_CLASS(mstm, IO::MemoryStream((const UTF8Char*)"Parser.FileParser.PNGParser.mstm"));
						}
						NEW_CLASS(cstm, Data::Compress::InflateStream(mstm, 2));
						NEW_CLASS(wcstm, IO::WriteCacheStream(cstm));
						wcstm->Write(chunkData, size);
					}
					else
					{
						wcstm->Write(chunkData, size);
					}
				}
				MemFree(chunkData);
			}
		}
		else if (*(Int32*)&buff[4] == *(Int32*)"fdAT")
		{
			if (ihdrFound)
			{
				chunkData = MemAlloc(UInt8, size);
				if (fd->GetRealData(ofst + 8, size, chunkData) == size)
				{
					if (mstm == 0)
					{
						if (colorType == 0)
						{
							imgSize = (((imgW * bitDepth) >> 3) + 1) * imgH;
						}
						else if (colorType == 2)
						{
							imgSize = (imgW * (UOSInt)(bitDepth >> 3) * 3 + 1) * imgH;
						}
						else if (colorType == 3)
						{
							imgSize = (((imgW * bitDepth) >> 3) + 1) * imgH;
						}
						else if (colorType == 4)
						{
							imgSize = (imgW * (UOSInt)(bitDepth >> 3) * 2 + 1) * imgH;
						}
						else if (colorType == 6)
						{
							imgSize = (imgW * (UOSInt)(bitDepth >> 3) * 4 + 1) * imgH;
						}
						else
						{
							imgSize = 0;
						}
						if (imgSize)
						{
							NEW_CLASS(mstm, IO::MemoryStream(imgSize, (const UTF8Char*)"Parser.FileParser.PNGParser.mstm"));
						}
						else
						{
							NEW_CLASS(mstm, IO::MemoryStream((const UTF8Char*)"Parser.FileParser.PNGParser.mstm"));
						}
						NEW_CLASS(cstm, Data::Compress::InflateStream(mstm, 2));
						NEW_CLASS(wcstm, IO::WriteCacheStream(cstm));
						wcstm->Write(&chunkData[4], size - 4);
					}
					else
					{
						wcstm->Write(&chunkData[4], size - 4);
					}
				}
				MemFree(chunkData);
			}
		}
		else if (*(Int32*)&buff[4] == *(Int32*)"IEND")
		{
			if (mstm)
			{
				wcstm->Flush();
				UOSInt dataSize;
				UInt8 *dataBuff = mstm->GetBuff(&dataSize);
				if (dataSize == imgSize || imgSize != 0)
				{
					ParseImage(bitDepth, colorType, dataBuff, &info, imgList, imgDelay, imgX, imgY, imgW, imgH, interlaceMeth, palette);
				}
				DEL_CLASS(wcstm);
				wcstm = 0;
				DEL_CLASS(cstm);
				cstm = 0;
				DEL_CLASS(mstm);
				mstm = 0;
			}
		}
		ofst += size + 12;
	}
	if (mstm)
	{
		wcstm->Flush();
		UOSInt dataSize;
		UInt8 *dataBuff = mstm->GetBuff(&dataSize);
		if (dataSize == imgSize || imgSize != 0)
		{
			ParseImage(bitDepth, colorType, dataBuff, &info, imgList, imgDelay, imgX, imgY, imgW, imgH, interlaceMeth, palette);
		}
		DEL_CLASS(wcstm);
		wcstm = 0;
		DEL_CLASS(cstm);
		cstm = 0;
		DEL_CLASS(mstm);
		mstm = 0;
	}
	if (imgList->GetCount() <= 0)
	{
		DEL_CLASS(imgList);
		imgList = 0;
	}
	if (palette)
	{
		MemFree(palette);
		palette = 0;
	}
	SDEL_CLASS(wcstm);
	SDEL_CLASS(cstm);
	SDEL_CLASS(mstm);

	return imgList;
}

UOSInt Parser::FileParser::PNGParser::CalcImageSize(UInt32 imgW, UInt32 imgH, UInt8 bitDepth, UInt8 colorType, UInt8 interlaceMeth)
{
	UOSInt byteDepth = (UOSInt)bitDepth >> 3;
	UOSInt imgSize;
	if (colorType == 0)
	{
		if (interlaceMeth == 1 && bitDepth <= 8)
		{
			UOSInt linePxCnt;
			UOSInt lineCnt;
			UOSInt andMask;
			UOSInt shiftCnt;
			imgSize = 0;

			if (bitDepth == 1)
			{
				andMask = 7;
				shiftCnt = 3;
			}
			else if (bitDepth == 2)
			{
				andMask = 3;
				shiftCnt = 2;
			}
			else if (bitDepth == 4)
			{
				andMask = 1;
				shiftCnt = 1;
			}
			else if (bitDepth == 8)
			{
				andMask = 0;
				shiftCnt = 0;
			}
			else
			{
				andMask = 0;
				shiftCnt = 0;
			}

			//Pass1
			linePxCnt = (imgW + 7) >> 3;
			lineCnt = (imgH + 7) >> 3;
			if (linePxCnt & andMask)
			{
				imgSize += (2 + (linePxCnt >> shiftCnt)) * lineCnt;
			}
			else
			{
				imgSize += (1 + (linePxCnt >> shiftCnt)) * lineCnt;
			}

			//Pass2
			linePxCnt = (imgW + 3) >> 3;
			lineCnt = (imgH + 7) >> 3;
			if (linePxCnt & andMask)
			{
				imgSize += (2 + (linePxCnt >> shiftCnt)) * lineCnt;
			}
			else
			{
				imgSize += (1 + (linePxCnt >> shiftCnt)) * lineCnt;
			}

			//Pass3
			linePxCnt = (imgW + 3) >> 2;
			lineCnt = (imgH + 3) >> 3;
			if (linePxCnt & andMask)
			{
				imgSize += (2 + (linePxCnt >> shiftCnt)) * lineCnt;
			}
			else
			{
				imgSize += (1 + (linePxCnt >> shiftCnt)) * lineCnt;
			}

			//Pass4
			linePxCnt = (imgW + 1) >> 2;
			lineCnt = (imgH + 3) >> 2;
			if (linePxCnt & andMask)
			{
				imgSize += (2 + (linePxCnt >> shiftCnt)) * lineCnt;
			}
			else
			{
				imgSize += (1 + (linePxCnt >> shiftCnt)) * lineCnt;
			}

			//Pass5
			linePxCnt = (imgW + 1) >> 1;
			lineCnt = (imgH + 1) >> 2;
			if (linePxCnt & andMask)
			{
				imgSize += (2 + (linePxCnt >> shiftCnt)) * lineCnt;
			}
			else
			{
				imgSize += (1 + (linePxCnt >> shiftCnt)) * lineCnt;
			}

			//Pass6
			linePxCnt = (imgW + 0) >> 1;
			lineCnt = (imgH + 1) >> 1;
			if (linePxCnt & andMask)
			{
				imgSize += (2 + (linePxCnt >> shiftCnt)) * lineCnt;
			}
			else
			{
				imgSize += (1 + (linePxCnt >> shiftCnt)) * lineCnt;
			}

			//Pass7
			linePxCnt = imgW;
			lineCnt = (imgH + 0) >> 1;
			if (linePxCnt & andMask)
			{
				imgSize += (2 + (linePxCnt >> shiftCnt)) * lineCnt;
			}
			else
			{
				imgSize += (1 + (linePxCnt >> shiftCnt)) * lineCnt;
			}
		}
		else if (interlaceMeth == 1 && bitDepth == 16)
		{
			UOSInt linePxCnt;
			UOSInt lineCnt;
			imgSize = 0;

			//Pass1
			linePxCnt = (imgW + 7) >> 3;
			lineCnt = (imgH + 7) >> 3;
			imgSize += (1 + (linePxCnt << 1)) * lineCnt;

			//Pass2
			linePxCnt = (imgW + 3) >> 3;
			lineCnt = (imgH + 7) >> 3;
			imgSize += (1 + (linePxCnt << 1)) * lineCnt;

			//Pass3
			linePxCnt = (imgW + 3) >> 2;
			lineCnt = (imgH + 3) >> 3;
			imgSize += (1 + (linePxCnt << 1)) * lineCnt;

			//Pass4
			linePxCnt = (imgW + 1) >> 2;
			lineCnt = (imgH + 3) >> 2;
			imgSize += (1 + (linePxCnt << 1)) * lineCnt;

			//Pass5
			linePxCnt = (imgW + 1) >> 1;
			lineCnt = (imgH + 1) >> 2;
			imgSize += (1 + (linePxCnt << 1)) * lineCnt;

			//Pass6
			linePxCnt = (imgW + 0) >> 1;
			lineCnt = (imgH + 1) >> 1;
			imgSize += (1 + (linePxCnt << 1)) * lineCnt;

			//Pass7
			linePxCnt = imgW;
			lineCnt = (imgH + 0) >> 1;
			imgSize += (1 + (linePxCnt << 1)) * lineCnt;
		}
		else
		{
			imgSize = (((imgW * bitDepth) >> 3) + 1) * imgH;
		}
	}
	else if (colorType == 2)
	{
		if (interlaceMeth == 1)
		{
			UOSInt linePxCnt;
			UOSInt lineCnt;
			imgSize = 0;

			//Pass1
			linePxCnt = (imgW + 7) >> 3;
			lineCnt = (imgH + 7) >> 3;
			imgSize += (1 + (linePxCnt * byteDepth * 3)) * lineCnt;

			//Pass2
			linePxCnt = (imgW + 3) >> 3;
			lineCnt = (imgH + 7) >> 3;
			imgSize += (1 + (linePxCnt * byteDepth * 3)) * lineCnt;

			//Pass3
			linePxCnt = (imgW + 3) >> 2;
			lineCnt = (imgH + 3) >> 3;
			imgSize += (1 + (linePxCnt * byteDepth * 3)) * lineCnt;

			//Pass4
			linePxCnt = (imgW + 1) >> 2;
			lineCnt = (imgH + 3) >> 2;
			imgSize += (1 + (linePxCnt * byteDepth * 3)) * lineCnt;

			//Pass5
			linePxCnt = (imgW + 1) >> 1;
			lineCnt = (imgH + 1) >> 2;
			imgSize += (1 + (linePxCnt * byteDepth * 3)) * lineCnt;

			//Pass6
			linePxCnt = (imgW + 0) >> 1;
			lineCnt = (imgH + 1) >> 1;
			imgSize += (1 + (linePxCnt * byteDepth * 3)) * lineCnt;

			//Pass7
			linePxCnt = imgW;
			lineCnt = (imgH + 0) >> 1;
			imgSize += (1 + (linePxCnt * byteDepth * 3)) * lineCnt;
		}
		else
		{
			imgSize = (imgW * byteDepth * 3 + 1) * imgH;
		}
	}
	else if (colorType == 3)
	{
		UOSInt andMask;
		UOSInt shiftCnt;
		if (bitDepth == 1)
		{
			andMask = 7;
			shiftCnt = 3;
		}
		else if (bitDepth == 2)
		{
			andMask = 3;
			shiftCnt = 2;
		}
		else if (bitDepth == 4)
		{
			andMask = 1;
			shiftCnt = 1;
		}
		else if (bitDepth == 8)
		{
			andMask = 0;
			shiftCnt = 0;
		}
		else
		{
			andMask = 0;
			shiftCnt = 0;
		}

		if (interlaceMeth == 1 && bitDepth <= 8)
		{
			UOSInt linePxCnt;
			UOSInt lineCnt;
			imgSize = 0;


			//Pass1
			linePxCnt = (imgW + 7) >> 3;
			lineCnt = (imgH + 7) >> 3;
			if (linePxCnt > 0 && lineCnt > 0)
			{
				if (linePxCnt & andMask)
				{
					imgSize += (2 + (linePxCnt >> shiftCnt)) * lineCnt;
				}
				else
				{
					imgSize += (1 + (linePxCnt >> shiftCnt)) * lineCnt;
				}
			}

			//Pass2
			linePxCnt = (imgW + 3) >> 3;
			lineCnt = (imgH + 7) >> 3;
			if (linePxCnt > 0 && lineCnt > 0)
			{
				if (linePxCnt & andMask)
				{
					imgSize += (2 + (linePxCnt >> shiftCnt)) * lineCnt;
				}
				else
				{
					imgSize += (1 + (linePxCnt >> shiftCnt)) * lineCnt;
				}
			}

			//Pass3
			linePxCnt = (imgW + 3) >> 2;
			lineCnt = (imgH + 3) >> 3;
			if (linePxCnt > 0 && lineCnt > 0)
			{
				if (linePxCnt & andMask)
				{
					imgSize += (2 + (linePxCnt >> shiftCnt)) * lineCnt;
				}
				else
				{
					imgSize += (1 + (linePxCnt >> shiftCnt)) * lineCnt;
				}
			}

			//Pass4
			linePxCnt = (imgW + 1) >> 2;
			lineCnt = (imgH + 3) >> 2;
			if (linePxCnt > 0 && lineCnt > 0)
			{
				if (linePxCnt & andMask)
				{
					imgSize += (2 + (linePxCnt >> shiftCnt)) * lineCnt;
				}
				else
				{
					imgSize += (1 + (linePxCnt >> shiftCnt)) * lineCnt;
				}
			}

			//Pass5
			linePxCnt = (imgW + 1) >> 1;
			lineCnt = (imgH + 1) >> 2;
			if (linePxCnt > 0 && lineCnt > 0)
			{
				if (linePxCnt & andMask)
				{
					imgSize += (2 + (linePxCnt >> shiftCnt)) * lineCnt;
				}
				else
				{
					imgSize += (1 + (linePxCnt >> shiftCnt)) * lineCnt;
				}
			}

			//Pass6
			linePxCnt = (imgW + 0) >> 1;
			lineCnt = (imgH + 1) >> 1;
			if (linePxCnt > 0 && lineCnt > 0)
			{
				if (linePxCnt & andMask)
				{
					imgSize += (2 + (linePxCnt >> shiftCnt)) * lineCnt;
				}
				else
				{
					imgSize += (1 + (linePxCnt >> shiftCnt)) * lineCnt;
				}
			}

			//Pass7
			linePxCnt = imgW;
			lineCnt = (imgH + 0) >> 1;
			if (linePxCnt > 0 && lineCnt > 0)
			{
				if (linePxCnt & andMask)
				{
					imgSize += (2 + (linePxCnt >> shiftCnt)) * lineCnt;
				}
				else
				{
					imgSize += (1 + (linePxCnt >> shiftCnt)) * lineCnt;
				}
			}
		}
		else if (bitDepth <= 8)
		{
			if (imgW & andMask)
			{
				imgSize = (2 + (imgW >> shiftCnt)) * imgH;
			}
			else
			{
				imgSize = (1 + (imgW >> shiftCnt)) * imgH;
			}
		}
		else
		{
			imgSize = (((imgW * bitDepth) >> 3) + 1) * imgH;
		}
	}
	else if (colorType == 4)
	{
		if (interlaceMeth == 1)
		{
			UOSInt linePxCnt;
			UOSInt lineCnt;
			imgSize = 0;

			//Pass1
			linePxCnt = (imgW + 7) >> 3;
			lineCnt = (imgH + 7) >> 3;
			if (linePxCnt > 0)
				imgSize += (1 + (linePxCnt * byteDepth * 2)) * lineCnt;

			//Pass2
			linePxCnt = (imgW + 3) >> 3;
			lineCnt = (imgH + 7) >> 3;
			if (linePxCnt > 0)
				imgSize += (1 + (linePxCnt * byteDepth * 2)) * lineCnt;

			//Pass3
			linePxCnt = (imgW + 3) >> 2;
			lineCnt = (imgH + 3) >> 3;
			if (linePxCnt > 0)
				imgSize += (1 + (linePxCnt * byteDepth * 2)) * lineCnt;

			//Pass4
			linePxCnt = (imgW + 1) >> 2;
			lineCnt = (imgH + 3) >> 2;
			if (linePxCnt > 0)
				imgSize += (1 + (linePxCnt * byteDepth * 2)) * lineCnt;

			//Pass5
			linePxCnt = (imgW + 1) >> 1;
			lineCnt = (imgH + 1) >> 2;
			if (linePxCnt > 0)
				imgSize += (1 + (linePxCnt * byteDepth * 2)) * lineCnt;

			//Pass6
			linePxCnt = (imgW + 0) >> 1;
			lineCnt = (imgH + 1) >> 1;
			if (linePxCnt > 0)
				imgSize += (1 + (linePxCnt * byteDepth * 2)) * lineCnt;

			//Pass7
			linePxCnt = imgW;
			lineCnt = (imgH + 0) >> 1;
			if (linePxCnt > 0)
				imgSize += (1 + (linePxCnt * byteDepth * 2)) * lineCnt;
		}
		else
		{
			imgSize = (imgW * byteDepth * 2 + 1) * imgH;
		}
	}
	else if (colorType == 6)
	{
		if (interlaceMeth == 1)
		{
			UOSInt linePxCnt;
			UOSInt lineCnt;
			imgSize = 0;

			//Pass1
			linePxCnt = (imgW + 7) >> 3;
			lineCnt = (imgH + 7) >> 3;
			if (linePxCnt > 0)
				imgSize += (1 + (linePxCnt * byteDepth * 4)) * lineCnt;

			//Pass2
			linePxCnt = (imgW + 3) >> 3;
			lineCnt = (imgH + 7) >> 3;
			if (linePxCnt > 0)
				imgSize += (1 + (linePxCnt * byteDepth * 4)) * lineCnt;

			//Pass3
			linePxCnt = (imgW + 3) >> 2;
			lineCnt = (imgH + 3) >> 3;
			if (linePxCnt > 0)
				imgSize += (1 + (linePxCnt * byteDepth * 4)) * lineCnt;

			//Pass4
			linePxCnt = (imgW + 1) >> 2;
			lineCnt = (imgH + 3) >> 2;
			if (linePxCnt > 0)
				imgSize += (1 + (linePxCnt * byteDepth * 4)) * lineCnt;

			//Pass5
			linePxCnt = (imgW + 1) >> 1;
			lineCnt = (imgH + 1) >> 2;
			if (linePxCnt > 0)
				imgSize += (1 + (linePxCnt * byteDepth * 4)) * lineCnt;

			//Pass6
			linePxCnt = (imgW + 0) >> 1;
			lineCnt = (imgH + 1) >> 1;
			if (linePxCnt > 0)
				imgSize += (1 + (linePxCnt * byteDepth * 4)) * lineCnt;

			//Pass7
			linePxCnt = imgW;
			lineCnt = (imgH + 0) >> 1;
			if (linePxCnt > 0)
				imgSize += (1 + (linePxCnt * byteDepth * 4)) * lineCnt;
		}
		else
		{
			imgSize = (imgW * byteDepth * 4 + 1) * imgH;
		}
	}
	else
	{
		imgSize = 0;
	}
	return imgSize;
}