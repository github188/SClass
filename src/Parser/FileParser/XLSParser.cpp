#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ByteTool.h"
#include "IO/StmData/BlockStreamData.h"
#include "Media/DrawEngine.h"
#include "Parser/FileParser/XLSParser.h"
#include "Text/MyStringW.h"
#include "Text/StringBuilderUTF8.h"
#include "Text/SpreadSheet/Workbook.h"

Parser::FileParser::XLSParser::XLSParser()
{
}

Parser::FileParser::XLSParser::~XLSParser()
{
}

Int32 Parser::FileParser::XLSParser::GetName()
{
	return *(Int32*)"XLSP";
}

void Parser::FileParser::XLSParser::PrepareSelector(IO::IFileSelector *selector, IO::ParsedObject::ParserType t)
{
	if (t == IO::ParsedObject::PT_UNKNOWN || t == IO::ParsedObject::PT_WORKBOOK)
	{
		selector->AddFilter((const UTF8Char*)"*.xls", (const UTF8Char*)"Excel Workbook File");
	}
}

IO::ParsedObject::ParserType Parser::FileParser::XLSParser::GetParserType()
{
	return IO::ParsedObject::PT_WORKBOOK;
}

IO::ParsedObject *Parser::FileParser::XLSParser::ParseFile(IO::IStreamData *fd, IO::PackageFile *pkgFile, IO::ParsedObject::ParserType targetType)
{
	UInt8 buff[4096];
	IO::ParsedObject *pobj = 0;
	IO::StmData::BlockStreamData *itemFd;
	fd->GetRealData(0, 512, buff);
	if (ReadUInt32(buff) != 0xe011cfd0 || ReadUInt32(&buff[4]) != 0xe11ab1a1)
	{
		return 0;
	}
	UOSInt i;
	UOSInt j;
	UInt8 *fat;
	UOSInt sectorSize = (UOSInt)(1 << (ReadUInt16(&buff[30])));
//	Int32 dirCnt = ReadInt32(&buff[40]);
	UInt32 fatCnt = ReadUInt32(&buff[44]);
	UInt32 dirSect = ReadUInt32(&buff[48]);
	if (fatCnt <= 0)
		return 0;

	UInt32 currSect;
	UInt64 sizeLeft;
	Data::DateTime createDt(1970, 1, 1, 0, 0, 0, 0);
	Data::DateTime modifyDt(1970, 1, 1, 0, 0, 0, 0);
	createDt.ToUTCTime();
	modifyDt.ToUTCTime();

	fat = MemAlloc(UInt8, sectorSize * fatCnt);
	i = 0;
	j = fatCnt;
	if (j > 109)
		j = 109;
	while (i < j)
	{
		fd->GetRealData(sectorSize * (ReadUInt32(&buff[76 + i * 4]) + 1), sectorSize, &fat[sectorSize * i]);
		i++;
	}

	while (true)
	{
		if (fd->GetRealData(512 + sectorSize * dirSect, sectorSize, buff) != sectorSize)
			break;
		i = 0;
		while (i < sectorSize)
		{
			if (ReadInt32(&buff[i]) == 0)
				break;

			if (ReadInt32(&buff[i + 100]) != 0 || ReadInt32(&buff[i + 104]) != 0)
			{
				createDt.SetValueFILETIME(&buff[i + 100]);
			}
			if (ReadInt32(&buff[i + 108]) != 0 || ReadInt32(&buff[i + 112]) != 0)
			{
				modifyDt.SetValueSYSTEMTIME(&buff[i + 108]);
			}

			if (Text::StrCompareICase((const WChar *)&buff[i], L"WORKBOOK") == 0)
			{
				Text::SpreadSheet::Workbook *wb;
				NEW_CLASS(wb, Text::SpreadSheet::Workbook());
				wb->SetSourceName(fd->GetFullName());
				if (createDt.GetYear() > 2000)
				{
					wb->SetCreateTime(&createDt);
				}
				if (modifyDt.GetYear() > 2000)
				{
					wb->SetModifyTime(&modifyDt);
				}

				currSect = ReadUInt32(&buff[i + 116]);
				sizeLeft = ReadUInt64(&buff[i + 120]);
				NEW_CLASS(itemFd, IO::StmData::BlockStreamData(fd));
				while (sizeLeft > 0)
				{
					if ((UInt32)currSect == 0xfffffffd)
						break;
					if (sizeLeft < sectorSize)
					{
						itemFd->Append(512 + sectorSize * currSect, (UInt32)sizeLeft);
						sizeLeft = 0;
						break;
					}
					else
					{
						itemFd->Append(512 + sectorSize * currSect, (UInt32)sectorSize);
						sizeLeft -= sectorSize;
					}
					currSect = ReadUInt32(&fat[currSect * 4]);
				}
				ParseWorkbook(itemFd, 0, 0, wb);
				DEL_CLASS(itemFd);
				pobj = wb;
				break;
			}
			i += 128;
		}
		if (i < sectorSize)
		{
			break;
		}
		dirSect = ReadUInt32(&fat[dirSect * 4]);
		if (dirSect == 0xfffffffe)
		{
			break;
		}
	}
	MemFree(fat);
	return pobj;
}

Bool Parser::FileParser::XLSParser::ParseWorkbook(IO::IStreamData *fd, UInt64 ofst, UInt64 ofstRef, Text::SpreadSheet::Workbook *wb)
{
	Bool eofFound = false;
	Bool bofFound = false;
	Text::StringBuilderUTF8 *sb;
	WorkbookStatus status;
	Parser::FileParser::XLSParser::FontInfo *font;
	Text::SpreadSheet::CellStyle *style;
	UInt8 *readBuff;
	UOSInt readBuffSize;
	UOSInt readSize;
	UOSInt i;
	UInt16 recNo;
	UInt16 recLeng;
	UInt64 currOfst = ofst;
	const UTF8Char *fmt;
	readBuff = MemAlloc(UInt8, 1048576);
	readBuffSize = 0;
	NEW_CLASS(status.sst, Data::ArrayList<const UTF8Char*>());
	NEW_CLASS(status.wsList, Data::ArrayList<WorksheetStatus*>());
	NEW_CLASS(status.fontList, Data::ArrayList<FontInfo*>());
	NEW_CLASS(status.formatMap, Data::Int32Map<const UTF8Char *>());
	Text::SpreadSheet::Workbook::GetDefPalette(status.palette);

	status.formatMap->Put(0x1, Text::StrCopyNew((const UTF8Char*)"0"));
	status.formatMap->Put(0x2, Text::StrCopyNew((const UTF8Char*)"0.00"));
	status.formatMap->Put(0x3, Text::StrCopyNew((const UTF8Char*)"#,##0"));
	status.formatMap->Put(0x4, Text::StrCopyNew((const UTF8Char*)"#,##0.00"));
	status.formatMap->Put(0x5, Text::StrCopyNew((const UTF8Char*)"($#,##0_);($#,##0)"));
	status.formatMap->Put(0x6, Text::StrCopyNew((const UTF8Char*)"($#,##0_);[Red]($#,##0)"));
	status.formatMap->Put(0x7, Text::StrCopyNew((const UTF8Char*)"($#,##0.00_);($#,##0.00)"));
	status.formatMap->Put(0x8, Text::StrCopyNew((const UTF8Char*)"($#,##0.00_);[Red]($#,##0.00)"));
	status.formatMap->Put(0x9, Text::StrCopyNew((const UTF8Char*)"0%"));
	status.formatMap->Put(0xa, Text::StrCopyNew((const UTF8Char*)"0.00%"));
	status.formatMap->Put(0xb, Text::StrCopyNew((const UTF8Char*)"0.00E+00"));
	status.formatMap->Put(0xc, Text::StrCopyNew((const UTF8Char*)"# ?/?"));
	status.formatMap->Put(0xd, Text::StrCopyNew((const UTF8Char*)"# \?\?/\?\?"));
	status.formatMap->Put(0xe, Text::StrCopyNew((const UTF8Char*)"m/d/yy"));
	status.formatMap->Put(0xf, Text::StrCopyNew((const UTF8Char*)"d-mmm-yy"));

	status.formatMap->Put(0x10, Text::StrCopyNew((const UTF8Char*)"d-mmm"));
	status.formatMap->Put(0x11, Text::StrCopyNew((const UTF8Char*)"mmm-yy"));
	status.formatMap->Put(0x12, Text::StrCopyNew((const UTF8Char*)"h:mm AM/PM"));
	status.formatMap->Put(0x13, Text::StrCopyNew((const UTF8Char*)"h:mm:ss AM/PM"));
	status.formatMap->Put(0x14, Text::StrCopyNew((const UTF8Char*)"h:mm"));
	status.formatMap->Put(0x15, Text::StrCopyNew((const UTF8Char*)"h:mm:ss"));
	status.formatMap->Put(0x16, Text::StrCopyNew((const UTF8Char*)"m/d/yy h:mm"));

	status.formatMap->Put(0x25, Text::StrCopyNew((const UTF8Char*)"(#,##0_);(#,##0)"));
	status.formatMap->Put(0x26, Text::StrCopyNew((const UTF8Char*)"(#,##0_);[Red](#,##0)"));
	status.formatMap->Put(0x27, Text::StrCopyNew((const UTF8Char*)"(#,##0.00_);(#,##0.00)"));
	status.formatMap->Put(0x28, Text::StrCopyNew((const UTF8Char*)"(#,##0.00_);[Red](#,##0.00)"));
	status.formatMap->Put(0x29, Text::StrCopyNew((const UTF8Char*)"_(* #,##0_);_(* (#,##0);_(* \"-\"_);_(@_)"));
	status.formatMap->Put(0x2a, Text::StrCopyNew((const UTF8Char*)"_($* #,##0_);_($* (#,##0);_($* \"-\"_);_(@_)"));
	status.formatMap->Put(0x2b, Text::StrCopyNew((const UTF8Char*)"_(* #,##0.00_);_(* (#,##0.00);_(* \"-\"??_);_(@_)"));
	status.formatMap->Put(0x2c, Text::StrCopyNew((const UTF8Char*)"_($* #,##0.00_);_($* (#,##0.00);_($* \"-\"??_);_(@_)"));
	status.formatMap->Put(0x2d, Text::StrCopyNew((const UTF8Char*)"mm:ss"));
	status.formatMap->Put(0x2e, Text::StrCopyNew((const UTF8Char*)"[h]:mm:ss"));
	status.formatMap->Put(0x2f, Text::StrCopyNew((const UTF8Char*)"mm:ss.0"));

	status.formatMap->Put(0x30, Text::StrCopyNew((const UTF8Char*)"##0.0E+0"));
	status.formatMap->Put(0x31, Text::StrCopyNew((const UTF8Char*)"@"));

	NEW_CLASS(sb, Text::StringBuilderUTF8());
	while (!eofFound)
	{
		readSize = fd->GetRealData(currOfst, 1048576 - readBuffSize, &readBuff[readBuffSize]);
		if (readSize <= 0)
			break;

		if (!bofFound && ReadUInt16(readBuff) != 0x809)
			break;
		readBuffSize += readSize;
		currOfst += readSize;
		i = 0;
		while (i < readBuffSize)
		{
			if (i + 4 > readBuffSize)
				break;
			
			recNo = ReadUInt16(&readBuff[i]);
			recLeng = ReadUInt16(&readBuff[i + 2]);
			if (i + 4 + recLeng > readBuffSize)
				break;
			switch (recNo)
			{
			case 0xa: //EOF
				eofFound = true;
				break;
			case 0xe: //PRECISION
				break;
			case 0x12: //PROTECT
				break;
			case 0x13: //PASSWORD
				break;
			case 0x17: //EXTERNSHEET
				break;
			case 0x18: //LABEL
				//////////////////////////
				break;
			case 0x19: //WINDOWPROTECT
				break;
			case 0x22: //1904
				break;
			case 0x31: //Undefined: Font Style
				{
					font = MemAlloc(Parser::FileParser::XLSParser::FontInfo, 1);
					font->height = ReadInt16(&readBuff[i + 4]);
					font->grbit = ReadInt16(&readBuff[i + 6]);
					font->icv = ReadInt16(&readBuff[i + 8]);
					font->bls = ReadInt16(&readBuff[i + 10]);
					font->sss = ReadInt16(&readBuff[i + 12]);
					font->uls = readBuff[i + 14];
					font->bFamily = readBuff[i + 15];
					font->bCharSet = readBuff[i + 16];
					sb->ClearStr();
					ReadUStringB(&readBuff[i + 18], sb);
					font->fontName = Text::StrCopyNew(sb->ToString());
					if (status.fontList->GetCount() == 4)
					{
						status.fontList->Add(0);
					}
					status.fontList->Add(font);

					Bool isBold = false;
					if (font->bls == 0x2bc)
						isBold = true;
					Text::SpreadSheet::WorkbookFont *f = wb->NewFont(font->fontName, font->height * 0.05, isBold);
					if (font->icv >= 64)
					{
					}
					else
					{
						f->SetColor(status.palette[font->icv - 8]);
					}
				}
				break;
			case 0x3d: //WINDOW1
				wb->SetWindowTopX(ReadInt16(&readBuff[i + 4]));
				wb->SetWindowTopY(ReadInt16(&readBuff[i + 6]));
				wb->SetWindowWidth(ReadInt16(&readBuff[i + 8]));
				wb->SetWindowHeight(ReadInt16(&readBuff[i + 10]));
				wb->SetActiveSheet(ReadUInt16(&readBuff[i + 14]));
				break;
			case 0x40: //BACKUP
				break;
			case 0x42: //CODEPAGE
				break;
			case 0x5c: //WRITEACCESS
				sb->ClearStr();
				ReadUString(&readBuff[i + 4], sb);
				wb->SetAuthor(sb->ToString());
				break;
			case 0x85: //BOUNDSHEET
				{
					sb->ClearStr();
					if (readBuff[i + 11] & 1)
					{
						const UTF16Char *wptr = Text::StrCopyNewC((UTF16Char*)&readBuff[i + 12], readBuff[i + 10]);
						const UTF8Char *csptr = Text::StrToUTF8New(wptr);
						sb->Append(csptr);
						Text::StrDelNew(csptr);
						Text::StrDelNew(wptr);
					}
					else
					{
						sb->AppendC((UTF8Char*)&readBuff[i + 12], readBuff[i + 10]);
					}
					WorksheetStatus *wsStatus = MemAlloc(WorksheetStatus, 1);
					wsStatus->ws = wb->AddWorksheet(sb->ToString());
					wsStatus->ofst = ofstRef + ReadUInt32(&readBuff[i + 4]);
					status.wsList->Add(wsStatus);
				}
				break;
			case 0x8c: //COUNTRY
				break;
			case 0x8d: //HIDEOBJb
				break;
			case 0x92: //PALETTE
				{
					UOSInt j = ReadUInt16(&readBuff[i + 4]);
					UOSInt k;
					k = 0;
					while (k < j)
					{
						status.palette[k] = 0xff000000 | (UInt32)(readBuff[i + 6 + k * 4] << 16) | (UInt32)(readBuff[i + 7 + k * 4] << 8) | (UInt32)(readBuff[i + 8 + k * 4]);
						k++;
					}
					wb->SetPalette(status.palette);
				}
				break;
			case 0x9c: //FNGROUPCOUNT
				break;
			case 0xc1: //MMS
				break;
			case 0xd3: //OBPROJ
				break;
			case 0xda: //BOOKBOOL
				break;
			case 0xe0: //XF
				{
					style = wb->NewCellStyle();
					UOSInt j = ReadUInt16(&readBuff[i + 4]);
					if (j)
					{
						font = status.fontList->GetItem(j);
						if (font)
						{
							style->SetFont(wb->GetFont(j));
						}
					}
					j = ReadUInt16(&readBuff[i + 6]);
					if (j)
					{
						style->SetDataFormat(status.formatMap->Get((Int32)j));
					}
					Text::SpreadSheet::CellStyle::BorderStyle borderLeft;
					Text::SpreadSheet::CellStyle::BorderStyle borderRight;
					Text::SpreadSheet::CellStyle::BorderStyle borderTop;
					Text::SpreadSheet::CellStyle::BorderStyle borderBottom;
					Int16 icv;
					borderLeft.borderType = Text::SpreadSheet::CellStyle::BT_NONE;
					borderTop.borderType = Text::SpreadSheet::CellStyle::BT_NONE;
					borderRight.borderType = Text::SpreadSheet::CellStyle::BT_NONE;
					borderBottom.borderType = Text::SpreadSheet::CellStyle::BT_NONE;
					Int32 tmpV = ReadInt16(&readBuff[i + 14]);
					if (tmpV != 0)
					{
						tmpV = ReadInt16(&readBuff[i + 14]);
					}
					borderLeft.borderType = (Text::SpreadSheet::CellStyle::BorderType)(tmpV & 0xf);
					borderRight.borderType = (Text::SpreadSheet::CellStyle::BorderType)((tmpV & 0xf0) >> 4);
					borderTop.borderType = (Text::SpreadSheet::CellStyle::BorderType)((tmpV & 0xf00) >> 8);
					borderBottom.borderType = (Text::SpreadSheet::CellStyle::BorderType)((tmpV & 0xf000) >> 12);
					tmpV = ReadInt16(&readBuff[i + 16]);
					icv = (tmpV & 0x7f);
					if (icv >= 64)
					{
						borderLeft.borderColor = 0;
					}
					else
					{
						borderLeft.borderColor = status.palette[icv - 8];
					}
					icv = (tmpV & 0x3f80) >> 7;
					if (icv >= 64)
					{
						borderRight.borderColor = 0;
					}
					else
					{
						borderRight.borderColor = status.palette[icv - 8];
					}
					tmpV = ReadInt16(&readBuff[i + 18]);
					icv = (tmpV & 0x7f);
					if (icv >= 64)
					{
						borderTop.borderColor = 0;
					}
					else
					{
						borderTop.borderColor = status.palette[icv - 8];
					}
					icv = (tmpV & 0x3f80) >> 7;
					if (icv >= 64)
					{
						borderBottom.borderColor = 0;
					}
					else
					{
						borderBottom.borderColor = status.palette[icv - 8];
					}
					if (borderLeft.borderType != Text::SpreadSheet::CellStyle::BT_NONE)
					{
						style->SetBorderLeft(&borderLeft);
					}
					if (borderRight.borderType != Text::SpreadSheet::CellStyle::BT_NONE)
					{
						style->SetBorderRight(&borderRight);
					}
					if (borderTop.borderType != Text::SpreadSheet::CellStyle::BT_NONE)
					{
						style->SetBorderTop(&borderTop);
					}
					if (borderBottom.borderType != Text::SpreadSheet::CellStyle::BT_NONE)
					{
						style->SetBorderBottom(&borderBottom);
					}

					icv = (ReadInt16(&readBuff[i + 22]) & 0x7f);
					if (icv >= 64)
					{
					}
					else
					{
						style->SetFillColor(status.palette[icv - 8], (Text::SpreadSheet::CellStyle::FillPattern)(readBuff[i + 21] >> 2));
					}
					j = ReadUInt16(&readBuff[i + 10]);
					switch (j & 7)
					{
					case 0:
						style->SetHAlign(Text::SpreadSheet::CellStyle::HALIGN_UNKNOWN);
						break;
					case 1:
						style->SetHAlign(Text::SpreadSheet::CellStyle::HALIGN_LEFT);
						break;
					case 2:
						style->SetHAlign(Text::SpreadSheet::CellStyle::HALIGN_CENTER);
						break;
					case 3:
						style->SetHAlign(Text::SpreadSheet::CellStyle::HALIGN_RIGHT);
						break;
					case 4:
						style->SetHAlign(Text::SpreadSheet::CellStyle::HALIGN_FILL);
						break;
					case 5:
						style->SetHAlign(Text::SpreadSheet::CellStyle::HALIGN_JUSTIFY);
						break;
					case 6:
						style->SetHAlign(Text::SpreadSheet::CellStyle::HALIGN_UNKNOWN);
						break;
					}
					switch ((j & 0x70) >> 4)
					{
					case 0:
						style->SetVAlign(Text::SpreadSheet::CellStyle::VALIGN_TOP);
						break;
					case 1:
						style->SetVAlign(Text::SpreadSheet::CellStyle::VALIGN_CENTER);
						break;
					case 2:
						style->SetVAlign(Text::SpreadSheet::CellStyle::VALIGN_BOTTOM);
						break;
					case 3:
						style->SetVAlign(Text::SpreadSheet::CellStyle::VALIGN_JUSTIFY);
						break;
					default:
						style->SetVAlign(Text::SpreadSheet::CellStyle::VALIGN_UNKNOWN);
						break;
					}
					style->SetWordWrap((j & 0x8) != 0);
				}
				break;
			case 0xe1: //INTERFACEHDR
				break;
			case 0xe2: //INTERFACEEND
				break;
			case 0xeb: //MSODRAWINGGROUP
				break;
			case 0xfc: //SST
				{
					UOSInt j = i + 12;
					UOSInt k;
					UInt32 nStr = ReadUInt32(&readBuff[i + 8]);
					while (nStr-- > 0)
					{
						if (j >= i + 4 + recLeng)
						{
							i += (UOSInt)(4 + recLeng);
							recNo = ReadUInt16(&readBuff[i]);
							recLeng = ReadUInt16(&readBuff[i + 2]);
							if (i + 4 + recLeng > readBuffSize)
							{
								if (i >= readBuffSize)
								{
									readBuffSize = 0;
								}
								else
								{
									MemCopyO(readBuff, &readBuff[i], readBuffSize - i);
									readBuffSize -= i;
								}
								readSize = fd->GetRealData(currOfst, 1048576 - readBuffSize, &readBuff[readBuffSize]);
								if (readSize <= 0)
									break;

								readBuffSize += readSize;
								currOfst += readSize;
								i = 0;
							}
							if (recNo != 0x3c)
								break;
							j = i + 4;
						}
						sb->ClearStr();
						k = ReadUString(&readBuff[j], sb);
						if (j + k > i + 4 + recLeng)
						{
							UInt32 charCnt = ReadUInt16(&readBuff[j]);
							sb->ClearStr();
							k = ReadUStringPartial(&readBuff[j + 2], i + 4 + recLeng - j - 2, &charCnt, sb);
							while (true)
							{
								i += (UOSInt)(4 + recLeng);
								recNo = ReadUInt16(&readBuff[i]);
								recLeng = ReadUInt16(&readBuff[i + 2]);
								if (i + 4 + recLeng > readBuffSize)
								{
									MemCopyO(readBuff, &readBuff[i], readBuffSize - i);
									readBuffSize -= i;
									readSize = fd->GetRealData(currOfst, 1048576 - readBuffSize, &readBuff[readBuffSize]);
									if (readSize <= 0)
										break;

									readBuffSize += readSize;
									currOfst += readSize;
								}
								if (recNo != 0x3c)
									break;
								j = i + 4;
								k = ReadUStringPartial(&readBuff[i + 4], i + 4 + recLeng - j, &charCnt, sb);
								if (charCnt <= 0)
									break;
							}
							j += k;
							status.sst->Add(Text::StrCopyNew(sb->ToString()));
						}
						else
						{
							j += k;
							status.sst->Add(Text::StrCopyNew(sb->ToString()));
						}
					}
					j = 0;
				}
				break;
			case 0xff: //EXTSST
				break;
			case 0x13d: //TABID
				break;
			case 0x160: //USESELFS
				break;
			case 0x161: //DSF
				break;
			case 0x1ae: //SUPBOOK
				break;
			case 0x1af: //PROT4REV
				break;
			case 0x1b7: //REFRESHALL
				break;
			case 0x1ba: //???
				break;
			case 0x1bc: //PROT4REVPASS
				break;
			case 0x1c0: //EXCEL9FILE
				break;
			case 0x1c1: //RECALCID
				break;
			case 0x293: //STYLE
				{

				}
				break;
			case 0x41e: //FORMAT
				{
					UInt16 ifmt = ReadUInt16(&readBuff[i + 4]);
					sb->ClearStr();
					ReadUString(&readBuff[i + 6], sb);
					fmt = status.formatMap->Put(ifmt, Text::StrCopyNew(sb->ToString()));
					if (fmt)
					{
						Text::StrDelNew(fmt);
					}
				}
				break;
			case 0x809: //BOF
				bofFound = true;
				break;
			case 0x863: //BOOKEXT
				break;
			case 0x87c: //XFCRC
				break;
			case 0x87d: //XFEXT
				{
					UInt16 ixfe = ReadUInt16(&readBuff[i + 18]);
					Int16 cexts = ReadInt16(&readBuff[i + 22]);
					UInt32 j;
					Text::SpreadSheet::CellStyle *style = wb->GetStyle(ixfe);
					if (style)
					{
						j = 24;
						while (cexts-- > 0 && j < recLeng)
						{
							switch (ReadInt16(&readBuff[i + j]))
							{
							case 0: //xfextRGBForeColor
								break;
							case 1: //xfextRGBBackColor
								break;
							case 4: //xfextForeColor
								break;
							case 5: //xfextBackColor
								break;
							case 6: //xfextGradientTint
								break;
							case 7: //xfextBorderColorTop
								break;
							case 8: //xfextBorderColorBottom
								break;
							case 9: //xfextBorderColorLeft
								break;
							case 10: //xfextBorderColorRight
								break;
							case 11: //xfextBorderColorDiag
								break;
							case 13: //xfextTextColor
								{
/*									Int16 xclrType = ReadInt16(&readBuff[i + j + 4]);
									if (xclrType == 1)
									{
										Int16 icv = ReadInt32(&readBuff[i + j + 8]);
										if (icv >= 64)
										{
										}
										else
										{
											style->SetFontColor(status.palette[icv - 8]);
										}
									}
									else if (xclrType == 2)
									{
										style->SetFontColor(ReadInt32(&readBuff[i + j + 8]));
									}
									else if (xclrType == 3)
									{
									}*/
								}
								break;
							case 14: //xfextFontScheme
								break;
							case 15: //xfextIndent
								break;
							}
							j += ReadUInt16(&readBuff[i + j + 2]);
						}
					}
				}
				break;
			case 0x88c: //COMPAT12
				break;
			case 0x88e:	//TABLESTYLES
				break;
			case 0x892: //STYLEEXT
				/////////////////////////////
				break;
			case 0x896: //THEME
				break;
			case 0x897: //GUIDTYPELIB
				break;
			case 0x89a: //MTRSETTINGS
				break;
			case 0x89b: //COMPRESSPICTURES
				break;
			case 0x8a3: //FORCEFULLCALCULATION
				break;
			default:
				readSize = 0;
				break;
			}
			i += (UOSInt)(4 + recLeng);
			if (eofFound)
				break;
		}
		if (i >= readBuffSize)
		{
			readBuffSize = 0;
		}
		else
		{
			MemCopyO(readBuff, &readBuff[i], readBuffSize - i);
			readBuffSize -= i;
		}
	}
	style = wb->GetDefaultStyle();
	font = status.fontList->GetItem(0);
	if (font)
	{
		style->SetFont(wb->GetFont(0));
	}

	i = status.wsList->GetCount();
	while (i-- > 0)
	{
		WorksheetStatus *wsStatus = status.wsList->GetItem(i);
		ParseWorksheet(fd, wsStatus->ofst, wb, wsStatus->ws, &status);
		MemFree(wsStatus);
	}
	DEL_CLASS(status.wsList);
	i = status.sst->GetCount();
	while (i-- > 0)
	{
		Text::StrDelNew(status.sst->GetItem(i));
	}
	DEL_CLASS(status.sst);
	i = status.fontList->GetCount();
	while (i-- > 0)
	{
		font = status.fontList->GetItem(i);
		if (font)
		{
			Text::StrDelNew(font->fontName);
			MemFree(font);
		}
	}
	DEL_CLASS(status.fontList);
	Data::ArrayList<const UTF8Char *> *formatList = status.formatMap->GetValues();
	i = formatList->GetCount();
	while (i-- > 0)
	{
		const UTF8Char *fmt = formatList->GetItem(i);
		if (fmt)
		{
			Text::StrDelNew(fmt);
		}
	}
	DEL_CLASS(status.formatMap);
	MemFree(readBuff);
	DEL_CLASS(sb);
	return eofFound;
}

Bool Parser::FileParser::XLSParser::ParseWorksheet(IO::IStreamData *fd, UInt64 ofst, Text::SpreadSheet::Workbook *wb, Text::SpreadSheet::Worksheet *ws, WorkbookStatus *status)
{
	Bool eofFound = false;
	Bool bofFound = false;
	UInt8 *readBuff;
	UOSInt readBuffSize;
	UOSInt readSize;
	UOSInt i;
	UInt16 recNo;
	UInt16 recLeng;
	UInt64 currOfst = ofst;
	readBuff = MemAlloc(UInt8, 1048576);
	readBuffSize = 0;
	while (!eofFound)
	{
		readSize = fd->GetRealData(currOfst, 1048576 - readBuffSize, &readBuff[readBuffSize]);
		if (readSize <= 0)
			break;
		readBuffSize += readSize;
		currOfst += readSize;
		i = 0;
		while (i < readBuffSize)
		{
			if (i + 4 > readBuffSize)
				break;
			
			if (!bofFound && ReadUInt16(readBuff) != 0x809)
				break;
			recNo = ReadUInt16(&readBuff[i]);
			recLeng = ReadUInt16(&readBuff[i + 2]);
			if (i + 4 + recLeng > readBuffSize)
				break;
			switch (recNo)
			{
			case 0x6: ////////
				break;
			case 0xa: //EOF
				eofFound = true;
				break;
			case 0xc: //CALCCOUNT
				break;
			case 0xd: //CALCMODE
				break;
			case 0xf: //REFMODE
				break;
			case 0x10: //DELTA
				break;
			case 0x11: //ITERATION
				break;
			case 0x12: //PROTECT
				break;
			case 0x13: //PASSWORD
				break;
			case 0x14: //HEADER
				break;
			case 0x15: //FOOTER
				break;
			case 0x1b: //HORIZONTALPAGEBREAKS
				/////////////////////////////////
				break;
			case 0x1c: //NOTE
				/////////////////////////////////
				break;
			case 0x1d: //SELECTION
				break;
			case 0x26: //LEFTMARGIN
				ws->SetMarginLeft(ReadDouble(&readBuff[i + 4]));
				break;
			case 0x27: //RIGHTMARGIN
				ws->SetMarginRight(ReadDouble(&readBuff[i + 4]));
				break;
			case 0x28: //TOPMARGIN
				ws->SetMarginTop(ReadDouble(&readBuff[i + 4]));
				break;
			case 0x29: //BOTTOMMARGIN
				ws->SetMarginBottom(ReadDouble(&readBuff[i + 4]));
				break;
			case 0x2a: //PRINTHEADERS
				break;
			case 0x2b: //PRINTGRIDLINES
				break;
			case 0x3c: //CONTINUE
				break;
			case 0x3d: //WINDOW1
				break;
			case 0x41: //PANE
				break;
			case 0x4d: //PLS
				break;
			case 0x55: //DEFCOLWIDTH
				break;
			case 0x5d: //OBJ
				break;
			case 0x5f: //SAVERECALC
				break;
			case 0x63: //OBJPROTECT
				break;
			case 0x7d: //COLINFO
				{
					UInt16 colStart = (UInt16)(ReadUInt16(&readBuff[i + 4]) + 1);
					UInt16 colEnd = (UInt16)(ReadUInt16(&readBuff[i + 6]) + 1);
					Double w = ReadUInt16(&readBuff[i + 8]) / 256.0 * 5.25 + 0.05;
					while (colStart <= colEnd)
					{
						ws->SetColWidth(colStart, w);
						colStart++;
					}
				}
				break;
			case 0x80: //GUTS
				break;
			case 0x81: //WSBOOL
				break;
			case 0x82: //GRIDSET
				break;
			case 0x83: //HCENTER
				break;
			case 0x84: //VCENTER
				break;
			case 0x9d: //AUTOFILTERINFO
				break;
			case 0xa1: //SETUP
				ws->SetMarginHeader(ReadDouble(&readBuff[i + 20]));
				ws->SetMarginFooter(ReadDouble(&readBuff[i + 28]));
				break;
			case 0xbd: //MULRK
				{
					UInt16 row = (UInt16)(ReadUInt16(&readBuff[i + 4]) + 1);
					UInt16 colStart = (UInt16)(ReadUInt16(&readBuff[i + 6]) + 1);
					UInt16 colEnd = (UInt16)(ReadUInt16(&readBuff[i + recLeng + 2]) + 1);
					UOSInt currI = i + 8;
					while (colStart <= colEnd)
					{
						ws->SetCellDouble(row, colStart, ParseRKNumber(ReadInt32(&readBuff[currI + 2])));
						ws->SetCellStyle(row, colStart, wb->GetStyle(ReadUInt16(&readBuff[currI])));

						currI += 6;
						colStart++;
					}
				}
				break;
			case 0xbe: //MULBLANK
				{
					UInt16 row = (UInt16)(ReadUInt16(&readBuff[i + 4]) + 1);
					UInt16 colStart = (UInt16)(ReadUInt16(&readBuff[i + 6]) + 1);
					UInt16 colEnd = (UInt16)(ReadUInt16(&readBuff[i + recLeng + 2]) + 1);
					UOSInt currI = i + 8;
					while (colStart <= colEnd)
					{
						ws->SetCellStyle(row, colStart, wb->GetStyle(ReadUInt16(&readBuff[currI])));

						currI += 2;
						colStart++;
					}
				}
				break;
			case 0xd7: //DBCELL
				break;
			case 0xdd: //SCENPROTECT
				break;
			case 0xe5: //MERGECELLS
				{
					UInt16 cnt = (UInt16)(ReadUInt16(&readBuff[i + 4]));
					UInt16 v = 0;
					UOSInt vofst = i + 6;
					while (v < cnt)
					{
						ws->MergeCells((UInt32)(ReadUInt16(&readBuff[vofst]) + 1), (UInt32)(ReadUInt16(&readBuff[vofst + 4]) + 1), (UInt32)ReadUInt16(&readBuff[vofst + 2]) - ReadUInt16(&readBuff[vofst]) + 1, (UInt32)ReadUInt16(&readBuff[vofst + 6]) - ReadUInt16(&readBuff[vofst + 4]) + 1);
						vofst += 8;
						v++;
					}
				}
				break;
			case 0xec: //MSODRAWING
				break;
			case 0xed: //MSODRAWINGSELECTION
				break;
			case 0xef: ///////////////////////////////////
				break;
			case 0xfd: //LABELSST
				ws->SetCellString((UInt32)(ReadUInt16(&readBuff[i + 4]) + 1), (UInt32)(ReadUInt16(&readBuff[i + 6]) + 1), status->sst->GetItem(ReadUInt32(&readBuff[i + 10])));
				ws->SetCellStyle((UInt32)(ReadUInt16(&readBuff[i + 4]) + 1), (UInt32)(ReadUInt16(&readBuff[i + 6]) + 1), wb->GetStyle(ReadUInt16(&readBuff[i + 8])));
				break;
			case 0x1b6: //TXO
				break;
			case 0x1c2: //???
				break;
			case 0x200: //DIMENSIONS
				break;
			case 0x201: //BLANK
				ws->SetCellStyle((UInt32)(ReadUInt16(&readBuff[i + 4]) + 1), (UInt32)(ReadUInt16(&readBuff[i + 6]) + 1), wb->GetStyle(ReadUInt16(&readBuff[i + 8])));
				break;
			case 0x203: //NUMBER
				ws->SetCellDouble((UInt32)(ReadUInt16(&readBuff[i + 4]) + 1), (UInt32)(ReadUInt16(&readBuff[i + 6]) + 1), ReadDouble(&readBuff[i + 10]));
				ws->SetCellStyle((UInt32)(ReadUInt16(&readBuff[i + 4]) + 1), (UInt32)(ReadUInt16(&readBuff[i + 6]) + 1), wb->GetStyle(ReadUInt16(&readBuff[i + 8])));
				break;
			case 0x207: //STRING

				break;
			case 0x208: //ROW
				ws->SetRowHeight((UInt32)(ReadUInt16(&readBuff[i + 4]) + 1), ReadInt16(&readBuff[i + 10]) * 0.05);
				break;
			case 0x20b: //INDEX
				break;
			case 0x225: //DEFAULTROWHEIGHT
				break;
			case 0x23e: //WINDOW2
				ws->SetOptions(ReadUInt16(&readBuff[i + 4]));
				ws->SetZoom(ReadUInt16(&readBuff[i + 16]));
				break;
			case 0x27e: // RK? ////////////////////////////
				ws->SetCellDouble((UInt32)(ReadUInt16(&readBuff[i + 4]) + 1), (UInt32)(ReadUInt16(&readBuff[i + 6]) + 1), ParseRKNumber(ReadInt32(&readBuff[i + 10])));
				ws->SetCellStyle((UInt32)(ReadUInt16(&readBuff[i + 4]) + 1), (UInt32)(ReadUInt16(&readBuff[i + 6]) + 1), wb->GetStyle(ReadUInt16(&readBuff[i + 8])));
				break;
			case 0x4bc: //SHRFMLA?
				break;
			case 0x809: //BOF
				bofFound = true;
				break;
			default:
				readSize = 0;
				break;
			}
			i += (UOSInt)(4 + recLeng);
			if (eofFound)
				break;
		}
		if (i >= readBuffSize)
		{
			readBuffSize = 0;
		}
		else
		{
			MemCopyO(readBuff, &readBuff[i], readBuffSize - i);
			readBuffSize -= i;
		}
	}
	MemFree(readBuff);
	return eofFound;
}

UOSInt Parser::FileParser::XLSParser::ReadUString(UInt8 *buff, Text::StringBuilderUTF *sb)
{
	UOSInt currOfst = 0;
	UOSInt charCnt = ReadUInt16(&buff[0]);
	UInt8 flags = buff[2];
	UOSInt fmtCnt;
	UOSInt cchExtRst;
	if ((flags & 12) == 0)
	{
		if (flags & 1)
		{
			const UTF16Char *wptr = Text::StrCopyNewC((UTF16Char*)&buff[3], charCnt);
			const UTF8Char *csptr = Text::StrToUTF8New(wptr);
			sb->Append(csptr);
			Text::StrDelNew(csptr);
			Text::StrDelNew(wptr);
			currOfst += 3 + charCnt * 2;
		}
		else
		{
			sb->AppendC((UTF8Char*)&buff[3], charCnt);
			currOfst += 3 + charCnt;
		}
	}
	else if ((flags & 12) == 4)
	{
		cchExtRst = ReadUInt32(&buff[3]);
		if (flags & 1)
		{
			const UTF16Char *wptr = Text::StrCopyNewC((UTF16Char*)&buff[7], charCnt);
			const UTF8Char *csptr = Text::StrToUTF8New(wptr);
			sb->Append(csptr);
			Text::StrDelNew(csptr);
			Text::StrDelNew(wptr);
			currOfst += 7 + charCnt * 2;
		}
		else
		{
			sb->AppendC((UTF8Char*)&buff[7], charCnt);
			currOfst += 7 + charCnt;
		}
		currOfst += cchExtRst;
	}
	else if ((flags & 12) == 8)
	{
		///////////////////////////////////////
		fmtCnt = ReadUInt16(&buff[3]);
		if (flags & 1)
		{
			const UTF16Char *wptr = Text::StrCopyNewC((UTF16Char*)&buff[5], charCnt);
			const UTF8Char *csptr = Text::StrToUTF8New(wptr);
			sb->Append(csptr);
			Text::StrDelNew(csptr);
			Text::StrDelNew(wptr);
			currOfst += 5 + charCnt * 2;
		}
		else
		{
			sb->AppendC((UTF8Char*)&buff[5], charCnt);
			currOfst += 5 + charCnt;
		}
		currOfst += fmtCnt * 4;
	}
	else if ((flags & 12) == 12)
	{
		///////////////////////////////////////
		fmtCnt = ReadUInt16(&buff[3]);
		cchExtRst = ReadUInt32(&buff[5]);
		if (flags & 1)
		{
			const UTF16Char *wptr = Text::StrCopyNewC((UTF16Char*)&buff[9], charCnt);
			const UTF8Char *csptr = Text::StrToUTF8New(wptr);
			sb->Append(csptr);
			Text::StrDelNew(csptr);
			Text::StrDelNew(wptr);
			currOfst += 9 + charCnt * 2;
		}
		else
		{
			sb->AppendC((UTF8Char*)&buff[9], charCnt);
			currOfst += 9 + charCnt;
		}
		currOfst += fmtCnt * 4;
		currOfst += cchExtRst;
	}
	return currOfst;
}

UOSInt Parser::FileParser::XLSParser::ReadUStringPartial(UInt8 *buff, UOSInt buffSize, UInt32 *charCnt, Text::StringBuilderUTF *sb)
{
	UOSInt currOfst = 0;
	UInt8 flags = buff[0];
	UOSInt thisCnt = *charCnt;
	UOSInt fmtCnt;
	UOSInt cchExtRst;
	if ((flags & 12) == 0)
	{
		if (flags & 1)
		{
			if (thisCnt * 2 + 1 > buffSize)
				thisCnt = (buffSize - 1) >> 1;
			const UTF16Char *wptr = Text::StrCopyNewC((UTF16Char*)&buff[1], thisCnt);
			const UTF8Char *csptr = Text::StrToUTF8New(wptr);
			sb->Append(csptr);
			Text::StrDelNew(csptr);
			Text::StrDelNew(wptr);
			currOfst += 1 + thisCnt * 2;
		}
		else
		{
			if (thisCnt + 1 > buffSize)
				thisCnt = buffSize - 1;
			sb->AppendC((const UTF8Char*)&buff[1], thisCnt);
			currOfst += 1 + thisCnt;
		}
	}
	else if ((flags & 12) == 4)
	{
		cchExtRst = ReadUInt32(&buff[1]);
		if (flags & 1)
		{
			if (thisCnt * 2 + 1 + cchExtRst > buffSize)
				thisCnt = (buffSize - 1 - cchExtRst) >> 1;
			const UTF16Char *wptr = Text::StrCopyNewC((UTF16Char*)&buff[5], thisCnt);
			const UTF8Char *csptr = Text::StrToUTF8New(wptr);
			sb->Append(csptr);
			Text::StrDelNew(csptr);
			Text::StrDelNew(wptr);
			currOfst += 5 + thisCnt * 2;
		}
		else
		{
			if (thisCnt + 1 + cchExtRst > buffSize)
				thisCnt = buffSize - 1 - cchExtRst;
			sb->AppendC((UTF8Char*)&buff[5], thisCnt);
			currOfst += 5 + thisCnt;
		}
		currOfst += cchExtRst;
	}
	else if ((flags & 12) == 8)
	{
		///////////////////////////////////////
		fmtCnt = ReadUInt16(&buff[1]);
		if (flags & 1)
		{
			if (thisCnt * 2 + 3 + (fmtCnt << 2) > buffSize)
				thisCnt = (buffSize - 3 - (fmtCnt << 2)) >> 1;
			const UTF16Char *wptr = Text::StrCopyNewC((UTF16Char*)&buff[3], thisCnt);
			const UTF8Char *csptr = Text::StrToUTF8New(wptr);
			sb->Append(csptr);
			Text::StrDelNew(csptr);
			Text::StrDelNew(wptr);
			currOfst += 3 + thisCnt * 2;
		}
		else
		{
			if (thisCnt + 3 + (fmtCnt << 2) > buffSize)
				thisCnt = buffSize - 3 - (fmtCnt << 2);
			sb->AppendC((UTF8Char*)&buff[3], thisCnt);
			currOfst += 3 + thisCnt;
		}
		currOfst += fmtCnt * 4;
	}
	else if ((flags & 12) == 12)
	{
		///////////////////////////////////////
		fmtCnt = ReadUInt16(&buff[1]);
		cchExtRst = ReadUInt32(&buff[3]);
		if (flags & 1)
		{
			if (thisCnt * 2 + 7 + (fmtCnt << 2) + cchExtRst > buffSize)
				thisCnt = (buffSize - 7 - (fmtCnt << 2) - cchExtRst) >> 1;
			const UTF16Char *wptr = Text::StrCopyNewC((UTF16Char*)&buff[7], thisCnt);
			const UTF8Char *csptr = Text::StrToUTF8New(wptr);
			sb->Append(csptr);
			Text::StrDelNew(csptr);
			Text::StrDelNew(wptr);
			currOfst += 7 + thisCnt * 2;
		}
		else
		{
			if (thisCnt + 7 + (fmtCnt << 2) + cchExtRst > buffSize)
				thisCnt = buffSize - 7 - (fmtCnt << 2) - cchExtRst;
			sb->AppendC((UTF8Char*)&buff[7], thisCnt);
			currOfst += 7 + thisCnt;
		}
		currOfst += fmtCnt * 4;
		currOfst += cchExtRst;
	}
	*charCnt = (UInt32)(*charCnt - thisCnt);
	return currOfst;
}

UOSInt Parser::FileParser::XLSParser::ReadUStringB(UInt8 *buff, Text::StringBuilderUTF *sb)
{
	UOSInt currOfst = 0;
	UOSInt charCnt = buff[0];
	UInt8 flags = buff[1];
	UOSInt fmtCnt;
	UOSInt cchExtRst;
	if ((flags & 12) == 0)
	{
		if (flags & 1)
		{
			const UTF16Char *wptr = Text::StrCopyNewC((UTF16Char*)&buff[2], charCnt);
			const UTF8Char *csptr = Text::StrToUTF8New(wptr);
			sb->Append(csptr);
			Text::StrDelNew(csptr);
			Text::StrDelNew(wptr);
			currOfst += 3 + charCnt * 2;
		}
		else
		{
			sb->AppendC((UTF8Char*)&buff[2], charCnt);
			currOfst += 3 + charCnt;
		}
	}
	else if ((flags & 12) == 4)
	{
		cchExtRst = ReadUInt32(&buff[2]);
		if (flags & 1)
		{
			const UTF16Char *wptr = Text::StrCopyNewC((UTF16Char*)&buff[6], charCnt);
			const UTF8Char *csptr = Text::StrToUTF8New(wptr);
			sb->Append(csptr);
			Text::StrDelNew(csptr);
			Text::StrDelNew(wptr);
			currOfst += 7 + charCnt * 2;
		}
		else
		{
			sb->AppendC((UTF8Char*)&buff[6], charCnt);
			currOfst += 7 + charCnt;
		}
		currOfst += cchExtRst;
	}
	else if ((flags & 12) == 8)
	{
		///////////////////////////////////////
		fmtCnt = ReadUInt16(&buff[2]);
		if (flags & 1)
		{
			const UTF16Char *wptr = Text::StrCopyNewC((UTF16Char*)&buff[4], charCnt);
			const UTF8Char *csptr = Text::StrToUTF8New(wptr);
			sb->Append(csptr);
			Text::StrDelNew(csptr);
			Text::StrDelNew(wptr);
			currOfst += 5 + charCnt * 2;
		}
		else
		{
			sb->AppendC((UTF8Char*)&buff[4], charCnt);
			currOfst += 5 + charCnt;
		}
		currOfst += fmtCnt * 4;
	}
	else if ((flags & 12) == 12)
	{
		///////////////////////////////////////
		fmtCnt = ReadUInt16(&buff[2]);
		cchExtRst = ReadUInt32(&buff[4]);
		if (flags & 1)
		{
			const UTF16Char *wptr = Text::StrCopyNewC((UTF16Char*)&buff[8], charCnt);
			const UTF8Char *csptr = Text::StrToUTF8New(wptr);
			sb->Append(csptr);
			Text::StrDelNew(csptr);
			Text::StrDelNew(wptr);
			currOfst += 9 + charCnt * 2;
		}
		else
		{
			sb->AppendC((UTF8Char*)&buff[8], charCnt);
			currOfst += 9 + charCnt;
		}
		currOfst += fmtCnt * 4;
		currOfst += cchExtRst;
	}
	return currOfst;
}

Double Parser::FileParser::XLSParser::ParseRKNumber(Int32 rkValue)
{
	UInt8 buff[8];
	if ((rkValue & 3) == 0)
	{
		WriteInt32(&buff[4], rkValue & ~3);
		WriteInt32(buff, 0);
		return ReadDouble(buff);
	}
	else if ((rkValue & 3) == 1)
	{
		WriteInt32(&buff[4], rkValue & ~3);
		WriteInt32(buff, 0);
		return ReadDouble(buff) * 0.01;
	}
	else if ((rkValue & 3) == 2)
	{
		return (rkValue >> 2);
	}
	else if ((rkValue & 3) == 3)
	{
		return (rkValue >> 2) * 0.01;
	}
	else
	{
		return 0;
	}
}
