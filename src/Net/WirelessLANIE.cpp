#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/ByteTool.h"
#include "Net/MACInfo.h"
#include "Net/WirelessLANIE.h"
#include "Text/MyString.h"
#include "Text/MyStringFloat.h"

Net::WirelessLANIE::WirelessLANIE(const UInt8 *ieBuff)
{
	this->ieBuff = MemAlloc(UInt8, ieBuff[1] + 2);
	MemCopyNO(this->ieBuff, ieBuff, ieBuff[1] + 2);
}

Net::WirelessLANIE::~WirelessLANIE()
{
	MemFree(this->ieBuff);
}

const UInt8 *Net::WirelessLANIE::GetIEBuff()
{
	return this->ieBuff;
}

void Net::WirelessLANIE::ToString(const UInt8 *ieBuff, Text::StringBuilderUTF *sb)
{
	UInt8 cmd = ieBuff[0];
	UInt8 size = ieBuff[1];
	UInt8 i;
	Bool found;
	switch (cmd)
	{
	case 0:
		sb->Append((const UTF8Char*)"Service Set Identity (SSID) = ");
		found = false;
		i = 0;
		while (i < size)
		{
			if (ieBuff[2 + i] == 0)
			{
				found = true;
				break;
			}
			i++;
		}
		if (!found)
		{
			sb->AppendC(&ieBuff[2], size);
		}
		else
		{
			sb->AppendChar('(', 1);
			sb->AppendHexBuff(&ieBuff[2], size, ' ', Text::LBT_NONE);
			sb->AppendChar(')', 1);
		}
		return;
	case 1:
		sb->Append((const UTF8Char*)"Supported Rates:");
		i = 0;
		while (i < size)
		{
			sb->Append((const UTF8Char *)"\r\n\t");
			if (ieBuff[2 + i] & 0x80)
			{
				sb->AppendChar('*', 1);
			}
			switch (ieBuff[2 + i] & 0x7F)
			{
			case 2:
				sb->Append((const UTF8Char*)"1Mbps");
				break;
			case 4:
				sb->Append((const UTF8Char*)"2Mbps");
				break;
			case 11:
				sb->Append((const UTF8Char*)"5.5Mbps");
				break;
			case 12:
				sb->Append((const UTF8Char*)"6Mbps");
				break;
			case 18:
				sb->Append((const UTF8Char*)"9Mbps");
				break;
			case 22:
				sb->Append((const UTF8Char*)"11Mbps");
				break;
			case 24:
				sb->Append((const UTF8Char*)"12Mbps");
				break;
			case 36:
				sb->Append((const UTF8Char*)"18Mbps");
				break;
			case 44:
				sb->Append((const UTF8Char*)"22Mbps");
				break;
			case 48:
				sb->Append((const UTF8Char*)"24Mbps");
				break;
			case 66:
				sb->Append((const UTF8Char*)"33Mbps");
				break;
			case 72:
				sb->Append((const UTF8Char*)"36Mbps");
				break;
			case 96:
				sb->Append((const UTF8Char*)"48Mbps");
				break;
			case 108:
				sb->Append((const UTF8Char*)"54Mbps");
				break;
			default:
				sb->AppendHex8(ieBuff[2 + i]);
				break;
			}
			i++;
		}
		return;
	case 2:
		if (size == 5)
		{
			sb->Append((const UTF8Char*)"FH Parameter Set:");
			sb->Append((const UTF8Char*)"\r\n\tDwell Time = ");
			sb->AppendU16(ReadUInt16(&ieBuff[2]));
			sb->Append((const UTF8Char*)"\r\n\tHop Set = ");
			sb->AppendU16(ieBuff[4]);
			sb->Append((const UTF8Char*)"\r\n\tHop Pattern = ");
			sb->AppendU16(ieBuff[5]);
			sb->Append((const UTF8Char*)"\r\n\tHop Index = ");
			sb->AppendU16(ieBuff[6]);
		}
		else
		{
			sb->Append((const UTF8Char*)"FH Parameter Set - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		return;
	case 3:
		if (size == 1)
		{
			sb->Append((const UTF8Char*)"DS Parameter Set:");
			sb->Append((const UTF8Char*)"\r\n\tCurrent channel = ");
			sb->AppendU16(ieBuff[2]);
		}
		else
		{
			sb->Append((const UTF8Char*)"DS Parameter Set - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		return;
	case 4:
		sb->Append((const UTF8Char*)"CF Parameter Set - ");
		sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		return;
	case 5:
		if (size >= 3)
		{
			sb->Append((const UTF8Char*)"Traffic Indication Map (TIM):");
			sb->Append((const UTF8Char*)"\r\n\tDTIM Count = ");
			sb->AppendU16(ieBuff[2]);
			sb->Append((const UTF8Char*)"\r\n\tDTIM Period = ");
			sb->AppendU16(ieBuff[3]);
			sb->Append((const UTF8Char*)"\r\n\tBitmap Control = 0x");
			sb->AppendHex8(ieBuff[4]);
			if (size > 3)
			{
				sb->Append((const UTF8Char*)"\r\n\tPartial Virtual Bitmap = ");
				sb->AppendHexBuff(&ieBuff[5], size - 3, 0, Text::LBT_NONE);
			}
		}
		else
		{
			sb->Append((const UTF8Char*)"Traffic Indication Map (TIM) - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		return;
	case 6:
		if (size == 2)
		{
			sb->Append((const UTF8Char*)"IBSS Parameter Set:");
			sb->Append((const UTF8Char*)"\r\n\tATIM window = ");
			sb->AppendU16(ReadUInt16(&ieBuff[2]));
		}
		else
		{
			sb->Append((const UTF8Char*)"IBSS Parameter Set - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		return;
	case 7:
		if (size >= 3)
		{
			sb->Append((const UTF8Char*)"Country:");
			sb->Append((const UTF8Char*)"\r\n\tCountry string = ");
			sb->AppendC(&ieBuff[2], 3);
			i = 3;
			while (i <= size - 3)
			{
				sb->Append((const UTF8Char*)"\r\n\tFirst channel number = ");
				sb->AppendU16(ieBuff[2 + i]);
				sb->Append((const UTF8Char*)"\r\n\tNumber of channels = ");
				sb->AppendU16(ieBuff[3 + i]);
				sb->Append((const UTF8Char*)"\r\n\tMax transmit power = ");
				sb->AppendU16(ieBuff[4 + i]);
				i += 3;
			}
		}
		else
		{
			sb->Append((const UTF8Char*)"Country - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		return;
	case 8:
		sb->Append((const UTF8Char*)"Hopping Pattern Parameters - ");
		sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		return;
	case 9:
		sb->Append((const UTF8Char*)"Hopping Pattern Table - ");
		sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		return;
	case 10:
		sb->Append((const UTF8Char*)"Request - ");
		sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		return;
	case 0x0B:
		if (size == 5)
		{
			sb->Append((const UTF8Char*)"BSS Load:");
			sb->Append((const UTF8Char*)"\r\n\tStation Count = ");
			sb->AppendU16(ReadUInt16(&ieBuff[2]));
			sb->Append((const UTF8Char*)"\r\n\tChannel Utilization = ");
			Text::SBAppendF64(sb, ieBuff[4] / 2.55);
			sb->Append((const UTF8Char*)"%\r\n\tAvailable Admission Capacity = ");
			sb->AppendU16(ReadUInt16(&ieBuff[5]));
		}
		else
		{
			sb->Append((const UTF8Char*)"BSS Load - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		return;
	case 0x10:
		sb->Append((const UTF8Char*)"Challenge text - ");
		sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		return;
	case 0x20:
		if (size == 1)
		{
			sb->Append((const UTF8Char*)"Power Constraint:");
			sb->Append((const UTF8Char*)"\r\n\tLocal Power Constraint = ");
			sb->AppendU16(ieBuff[2]);
		}
		else
		{
			sb->Append((const UTF8Char*)"Power Constraint - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		return;
	case 0x21:
		sb->Append((const UTF8Char*)"Power Capability - ");
		sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		return;
	case 0x22:
		sb->Append((const UTF8Char*)"Transmit Power Control (TPC) Request - ");
		sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		return;
	case 0x23:
		if (size == 2)
		{
			sb->Append((const UTF8Char*)"TPC Report:");
			sb->Append((const UTF8Char*)"\r\n\tTransmit power = ");
			sb->AppendU16(ieBuff[2]);
			sb->Append((const UTF8Char*)"dBm\r\n\tLink margin = ");
			sb->AppendU16(ieBuff[3]);
			sb->Append((const UTF8Char*)"dB");
		}
		else
		{
			sb->Append((const UTF8Char*)"TPC Report - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		return;
	case 0x24:
		sb->Append((const UTF8Char*)"Supported Channels - ");
		sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		return;
	case 0x25:
		sb->Append((const UTF8Char*)"Channel Switch Announcement - ");
		sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		return;
	case 0x26:
		sb->Append((const UTF8Char*)"Measurement Request - ");
		sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		return;
	case 0x27:
		sb->Append((const UTF8Char*)"Measurement Report - ");
		sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		return;
	case 0x28:
		sb->Append((const UTF8Char*)"Quiet - ");
		sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		return;
	case 0x29:
		sb->Append((const UTF8Char*)"IBSS DFS - ");
		sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		return;
	case 0x2A:
		if (size == 1)
		{
			sb->Append((const UTF8Char*)"ERP information = 0x");
			sb->AppendHex8(ieBuff[2]);
			sb->Append((const UTF8Char*)"\r\n\tNon Erp Present = ");
			sb->AppendU16(ieBuff[2] & 1);
			sb->Append((const UTF8Char*)"\r\n\tUse Protection = ");
			sb->AppendU16((ieBuff[2] >> 1) & 1);
			sb->Append((const UTF8Char*)"\r\n\tBarker Preamble Mode = ");
			sb->AppendU16((ieBuff[2] >> 2) & 1);
		}
		else
		{
			sb->Append((const UTF8Char*)"ERP information - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		return;
	case 0x2D:
		if (size == 26)
		{
			UInt16 v16;
			sb->Append((const UTF8Char*)"HT Capabilities:");
			sb->Append((const UTF8Char*)"\r\n\tHT Capabilities Info = 0x");
			sb->AppendHex16(v16 = ReadUInt16(&ieBuff[2]));
			sb->Append((const UTF8Char*)"\r\n\t\tLdpc = ");
			sb->AppendU16((v16 >> 0) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tSupported Channel Width = ");
			sb->AppendU16((v16 >> 1) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tSm Power Save = ");
			sb->AppendU16((v16 >> 2) & 3);
			sb->Append((const UTF8Char*)"\r\n\t\tGreen Field = ");
			sb->AppendU16((v16 >> 4) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tShort Guard Interval 20 = ");
			sb->AppendU16((v16 >> 5) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tShort Guard Interval 40 = ");
			sb->AppendU16((v16 >> 6) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tTx STBC = ");
			sb->AppendU16((v16 >> 7) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tRx STBC = ");
			sb->AppendU16((v16 >> 8) & 3);
			sb->Append((const UTF8Char*)"\r\n\t\tHt Delayed Block Ack = ");
			sb->AppendU16((v16 >> 10) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tMax Amsdu Length = ");
			sb->AppendU16((v16 >> 11) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tDss Mode 40 = ");
			sb->AppendU16((v16 >> 12) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tPsmp Support = ");
			sb->AppendU16((v16 >> 13) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tForty Mhz Intolerant = ");
			sb->AppendU16((v16 >> 14) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tLsig Protection Support = ");
			sb->AppendU16((v16 >> 15) & 1);
			sb->Append((const UTF8Char*)"\r\n\tAmpdu Parameters = ");
			sb->AppendU16(ieBuff[4]);
			sb->Append((const UTF8Char*)"\r\n\t\tMax Ampdu Length Exponent = ");
			sb->AppendU16((ieBuff[4] >> 0) & 3);
			sb->Append((const UTF8Char*)"\r\n\t\tMin Mpdu Start Space = ");
			sb->AppendU16((ieBuff[4] >> 2) & 0x1b);
			sb->Append((const UTF8Char*)"\r\n\tSupported Mcs Set 1 = ");
			sb->AppendI64(ReadInt64(&ieBuff[5]));
			sb->Append((const UTF8Char*)"\r\n\tSupported Mcs Set 2 = ");
			sb->AppendI64(ReadInt64(&ieBuff[13]));
			sb->Append((const UTF8Char*)"\r\n\tExtended HT Capabilities = ");
			sb->AppendI16(ReadInt16(&ieBuff[21]));
			sb->Append((const UTF8Char*)"\r\n\tTx Bf Capabilities = ");
			sb->AppendI32(ReadInt32(&ieBuff[23]));
			sb->Append((const UTF8Char*)"\r\n\tAntenna Selection Capabilities = ");
			sb->AppendU16(ieBuff[27]);
		}
		else
		{
			sb->Append((const UTF8Char*)"HT Capabilities - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		return;
	case 0x2F:
		if (size == 1)
		{
			sb->Append((const UTF8Char*)"ERP D4.0 = 0x");
			sb->AppendHex8(ieBuff[2]);
			sb->Append((const UTF8Char*)"\r\n\tNon Erp Present = ");
			sb->AppendU16(ieBuff[2] & 1);
			sb->Append((const UTF8Char*)"\r\n\tUse Protection = ");
			sb->AppendU16((ieBuff[2] >> 1) & 1);
			sb->Append((const UTF8Char*)"\r\n\tBarker Preamble Mode = ");
			sb->AppendU16((ieBuff[2] >> 2) & 1);
		}
		else
		{
			sb->Append((const UTF8Char*)"ERP D4.0 - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		return;
	case 0x30:
		if (size >= 14)
		{
			sb->Append((const UTF8Char*)"Robust Security Network:");
			sb->Append((const UTF8Char*)"\r\n\tVersion = ");
			sb->AppendU16(ReadUInt16(&ieBuff[2]));
			sb->Append((const UTF8Char*)"\r\n\tGroup cipher suite OUI = ");
			sb->AppendHexBuff(&ieBuff[4], 3, '-', Text::LBT_NONE);
			sb->Append((const UTF8Char*)" (");
			sb->Append((const UTF8Char*)Net::MACInfo::GetMACInfoOUI(&ieBuff[4])->name);
			sb->Append((const UTF8Char*)")");
			sb->Append((const UTF8Char*)"\r\n\tGroup cipher suite Type = ");
			sb->AppendU16(ieBuff[7]);
			if (ieBuff[4] == 0x00 && ieBuff[5] == 0x0F && ieBuff[6] == 0xAC)
			{
				switch (ieBuff[7])
				{
				case 0:
					sb->Append((const UTF8Char*)" (Use the group cipher suite)");
					break;
				case 1:
					sb->Append((const UTF8Char*)" (WEP-40)");
					break;
				case 2:
					sb->Append((const UTF8Char*)" (TKIP)");
					break;
				case 3:
					sb->Append((const UTF8Char*)" (Reserved)");
					break;
				case 4:
					sb->Append((const UTF8Char*)" (CCMP)");
					break;
				case 5:
					sb->Append((const UTF8Char*)" (WEP-104)");
					break;
				}
			}
			UInt32 cnt = ReadUInt16(&ieBuff[8]);
			sb->Append((const UTF8Char*)"\r\n\tPairwise Cipher Suites Count = ");
			sb->AppendU32(cnt);
			UInt32 j;
			i = 8;
			j = 0;
			while (i <= size - 4 && j < cnt)
			{
				sb->Append((const UTF8Char*)"\r\n\tPairwise cipher suite OUI = ");
				sb->AppendHexBuff(&ieBuff[i + 2], 3, '-', Text::LBT_NONE);
				sb->Append((const UTF8Char*)" (");
				sb->Append((const UTF8Char*)Net::MACInfo::GetMACInfoOUI(&ieBuff[i + 2])->name);
				sb->Append((const UTF8Char*)")");
				sb->Append((const UTF8Char*)"\r\n\tPairwise cipher suite Type = ");
				sb->AppendU16(ieBuff[i + 5]);
				if (ieBuff[i + 2] == 0x00 && ieBuff[i + 3] == 0x0F && ieBuff[i + 4] == 0xAC)
				{
					switch (ieBuff[i + 5])
					{
					case 0:
						sb->Append((const UTF8Char*)" (Use the group cipher suite)");
						break;
					case 1:
						sb->Append((const UTF8Char*)" (WEP-40)");
						break;
					case 2:
						sb->Append((const UTF8Char*)" (TKIP)");
						break;
					case 3:
						sb->Append((const UTF8Char*)" (Reserved)");
						break;
					case 4:
						sb->Append((const UTF8Char*)" (CCMP)");
						break;
					case 5:
						sb->Append((const UTF8Char*)" (WEP-104)");
						break;
					}
				}
				j++;
				i += 4;
			}
			if (i <= size - 4)
			{
				cnt = ReadUInt16(&ieBuff[i + 2]);
				sb->Append((const UTF8Char*)"\r\n\tAuthentication Suites Count = ");
				sb->AppendU32(cnt);
				i += 2;
				j = 0;
				while (i <= size - 4 && j < cnt)
				{
					sb->Append((const UTF8Char*)"\r\n\tAuthentication Suites OUI = ");
					sb->AppendHexBuff(&ieBuff[i + 2], 3, '-', Text::LBT_NONE);
					sb->Append((const UTF8Char*)" (");
					sb->Append((const UTF8Char*)Net::MACInfo::GetMACInfoOUI(&ieBuff[i + 2])->name);
					sb->Append((const UTF8Char*)")");
					sb->Append((const UTF8Char*)"\r\n\tAuthentication Suites Type = ");
					sb->AppendU16(ieBuff[i + 5]);
					if (ieBuff[i + 2] == 0x00 && ieBuff[i + 3] == 0x0F && ieBuff[i + 4] == 0xAC)
					{
						switch (ieBuff[i + 5])
						{
						case 1:
							sb->Append((const UTF8Char*)" (802.1X or PMK caching)");
							break;
						case 2:
							sb->Append((const UTF8Char*)" (Pre-Shared Key/PSK)");
							break;
						}
					}
					j++;
					i += 4;
				}
			}
			if (i <= size - 2)
			{
				sb->Append((const UTF8Char*)"\r\n\tRSN Capabilties = 0x");
				sb->AppendHex16(ReadUInt16(&ieBuff[i + 2]));
				i += 2;
			}
			if (i < size)
			{
				sb->Append((const UTF8Char*)"\r\n\tUnknown = ");
				sb->AppendHexBuff(&ieBuff[i + 2], size - i, 0, Text::LBT_NONE);
				i += 2;
			}
		}
		else
		{
			sb->Append((const UTF8Char*)"Robust Security Network - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		return;
	case 0x32:
		sb->Append((const UTF8Char*)"Extended Supported Rates:");
		i = 0;
		while (i < size)
		{
			sb->Append((const UTF8Char *)"\r\n\t");
			if (ieBuff[2 + i] & 0x80)
			{
				sb->AppendChar('*', 1);
			}
			switch (ieBuff[2 + i] & 0x7F)
			{
			case 2:
				sb->Append((const UTF8Char*)"1Mbps");
				break;
			case 4:
				sb->Append((const UTF8Char*)"2Mbps");
				break;
			case 11:
				sb->Append((const UTF8Char*)"5.5Mbps");
				break;
			case 12:
				sb->Append((const UTF8Char*)"6Mbps");
				break;
			case 18:
				sb->Append((const UTF8Char*)"9Mbps");
				break;
			case 22:
				sb->Append((const UTF8Char*)"11Mbps");
				break;
			case 24:
				sb->Append((const UTF8Char*)"12Mbps");
				break;
			case 36:
				sb->Append((const UTF8Char*)"18Mbps");
				break;
			case 44:
				sb->Append((const UTF8Char*)"22Mbps");
				break;
			case 48:
				sb->Append((const UTF8Char*)"24Mbps");
				break;
			case 66:
				sb->Append((const UTF8Char*)"33Mbps");
				break;
			case 72:
				sb->Append((const UTF8Char*)"36Mbps");
				break;
			case 96:
				sb->Append((const UTF8Char*)"48Mbps");
				break;
			case 108:
				sb->Append((const UTF8Char*)"54Mbps");
				break;
			default:
				sb->AppendHex8(ieBuff[2 + i]);
				break;
			}
			i++;
		}
		return;
	case 0x3D:
		if (size == 22)
		{
			UInt16 v16;
			sb->Append((const UTF8Char*)"HT Operation:");
			sb->Append((const UTF8Char*)"\r\n\tPrimaryChannel = ");
			sb->AppendU16(ieBuff[2]);
			sb->Append((const UTF8Char*)"\r\n\tInformation Subset 1 = 0x");
			sb->AppendHex8(ieBuff[3]);
			sb->Append((const UTF8Char*)"\r\n\t\tSecondary Channel Offset = ");
			sb->AppendU16(ieBuff[3] & 3);
			sb->Append((const UTF8Char*)"\r\n\t\tSta Channel Width = ");
			sb->AppendU16((ieBuff[3] >> 2) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tRifs Mode = ");
			sb->AppendU16((ieBuff[3] >> 3) & 1);
			sb->Append((const UTF8Char*)"\r\n\tInformation Subset 2 = 0x");
			sb->AppendHex16(v16 = ReadUInt16(&ieBuff[4]));
			sb->Append((const UTF8Char*)"\r\n\t\tHT Protection = ");
			sb->AppendU16(v16 & 3);
			sb->Append((const UTF8Char*)"\r\n\t\tNon Gf Ht Stas Present = ");
			sb->AppendU16((v16 >> 2) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tObss Non Ht Stas Present = ");
			sb->AppendU16((v16 >> 4) & 1);
			sb->Append((const UTF8Char*)"\r\n\tInformation Subset 3 = 0x");
			sb->AppendHex16(v16 = ReadUInt16(&ieBuff[6]));
			sb->Append((const UTF8Char*)"\r\n\t\tDual Beacon = ");
			sb->AppendU16((v16 >> 6) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tDual Cts Protection = ");
			sb->AppendU16((v16 >> 7) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tSTBC Beacon = ");
			sb->AppendU16((v16 >> 8) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tL-Sig Txop Protection Full Support = ");
			sb->AppendU16((v16 >> 9) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tPCO Active = ");
			sb->AppendU16((v16 >> 10) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tPCO Phase = ");
			sb->AppendU16((v16 >> 11) & 1);
			sb->Append((const UTF8Char*)"\r\n\tBasic Mcs Set 1 = 0x");
			sb->AppendHex64(ReadUInt64(&ieBuff[8]));
			sb->Append((const UTF8Char*)"\r\n\tBasic Mcs Set 2 = 0x");
			sb->AppendHex64(ReadUInt64(&ieBuff[16]));
		}
		else
		{
			sb->Append((const UTF8Char*)"HT Operation - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		return;
	case 0x46:
		if (size == 3 || size == 5)
		{
			sb->Append((const UTF8Char*)"Mobility Domain:");
			sb->Append((const UTF8Char*)"\r\n\tMobility Domain Identifier = 0x");
			sb->AppendHex16(ReadUInt16(&ieBuff[2]));
			sb->Append((const UTF8Char*)"\r\n\tFT Capability and Policy = 0x");
			sb->AppendHex8(ieBuff[4]);
			sb->Append((const UTF8Char*)"\r\n\t\tFast BSS Transition over DS = ");
			sb->AppendU16((ieBuff[4] >> 0) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tResource Request Protocol capability = ");
			sb->AppendU16((ieBuff[4] >> 1) & 1);
		}
		else
		{
			sb->Append((const UTF8Char*)"Mobility Domain - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		return;
	case 0x4A:
		if (size == 14)
		{
			sb->Append((const UTF8Char*)"Overlapping BSS Scan Parameters:");
			sb->Append((const UTF8Char*)"\r\n\tPassive dwell = ");
			sb->AppendU16(ReadUInt16(&ieBuff[2]));
			sb->Append((const UTF8Char*)"TUs\r\n\tActive dwell = ");
			sb->AppendU16(ReadUInt16(&ieBuff[4]));
			sb->Append((const UTF8Char*)"TUs\r\n\tChannel width trigger scan interval = ");
			sb->AppendU16(ReadUInt16(&ieBuff[6]));
			sb->Append((const UTF8Char*)"s\r\n\tScan passive total per channel = ");
			sb->AppendU16(ReadUInt16(&ieBuff[8]));
			sb->Append((const UTF8Char*)"TUs\r\n\tScan active total per channel = ");
			sb->AppendU16(ReadUInt16(&ieBuff[10]));
			sb->Append((const UTF8Char*)"TUs\r\n\tBSS width channel transition delay factor = ");
			sb->AppendU16(ReadUInt16(&ieBuff[12]));
			sb->Append((const UTF8Char*)"\r\n\tOBSS Scan Activity Threshold = ");
			Text::SBAppendF64(sb, ReadUInt16(&ieBuff[14]) * 0.01);
			sb->Append((const UTF8Char*)"%");
		}
		else
		{
			sb->Append((const UTF8Char*)"Overlapping BSS Scan Parameters - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		return;
	case 0x7F:
		if (size == 8)
		{
			sb->Append((const UTF8Char*)"Extended Capabilities = ");
			sb->AppendHexBuff(&ieBuff[2], 8, ' ', Text::LBT_NONE);
			sb->Append((const UTF8Char*)"\r\n\t20 40 Bss Coexistence Management Support = ");
			sb->AppendU16((ieBuff[2] >> 0) & 1);
			sb->Append((const UTF8Char*)"\r\n\tExtended Channel Switching = ");
			sb->AppendU16((ieBuff[2] >> 2) & 1);
			sb->Append((const UTF8Char*)"\r\n\tPsmp Capability = ");
			sb->AppendU16((ieBuff[2] >> 4) & 1);
			sb->Append((const UTF8Char*)"\r\n\tSpsmp Support = ");
			sb->AppendU16((ieBuff[2] >> 6) & 1);
			sb->Append((const UTF8Char*)"\r\n\tEvent = ");
			sb->AppendU16((ieBuff[2] >> 7) & 1);
			sb->Append((const UTF8Char*)"\r\n\tDiagnostics = ");
			sb->AppendU16((ieBuff[3] >> 0) & 1);
			sb->Append((const UTF8Char*)"\r\n\tMulticast Diagnostics = ");
			sb->AppendU16((ieBuff[3] >> 1) & 1);
			sb->Append((const UTF8Char*)"\r\n\tLocation Tracking = ");
			sb->AppendU16((ieBuff[3] >> 2) & 1);
			sb->Append((const UTF8Char*)"\r\n\tFms = ");
			sb->AppendU16((ieBuff[3] >> 3) & 1);
			sb->Append((const UTF8Char*)"\r\n\tProxy Arp Service = ");
			sb->AppendU16((ieBuff[3] >> 4) & 1);
			sb->Append((const UTF8Char*)"\r\n\tCollocated Interference Reporting = ");
			sb->AppendU16((ieBuff[3] >> 5) & 1);
			sb->Append((const UTF8Char*)"\r\n\tCivic Location = ");
			sb->AppendU16((ieBuff[3] >> 6) & 1);
			sb->Append((const UTF8Char*)"\r\n\tGeospatial Location = ");
			sb->AppendU16((ieBuff[3] >> 7) & 1);
			sb->Append((const UTF8Char*)"\r\n\tTfs = ");
			sb->AppendU16((ieBuff[4] >> 0) & 1);
			sb->Append((const UTF8Char*)"\r\n\tWnm Sleep Mode = ");
			sb->AppendU16((ieBuff[4] >> 1) & 1);
			sb->Append((const UTF8Char*)"\r\n\tTim Broadcast = ");
			sb->AppendU16((ieBuff[4] >> 2) & 1);
			sb->Append((const UTF8Char*)"\r\n\tBss Transition = ");
			sb->AppendU16((ieBuff[4] >> 3) & 1);
			sb->Append((const UTF8Char*)"\r\n\tQos Traffic Capability = ");
			sb->AppendU16((ieBuff[4] >> 4) & 1);
			sb->Append((const UTF8Char*)"\r\n\tAc Station Count = ");
			sb->AppendU16((ieBuff[4] >> 5) & 1);
			sb->Append((const UTF8Char*)"\r\n\tMultiple Bssid = ");
			sb->AppendU16((ieBuff[4] >> 6) & 1);
			sb->Append((const UTF8Char*)"\r\n\tTiming Measurement = ");
			sb->AppendU16((ieBuff[4] >> 7) & 1);
			sb->Append((const UTF8Char*)"\r\n\tChannel Usage = ");
			sb->AppendU16((ieBuff[5] >> 0) & 1);
			sb->Append((const UTF8Char*)"\r\n\tSsid List = ");
			sb->AppendU16((ieBuff[5] >> 1) & 1);
			sb->Append((const UTF8Char*)"\r\n\tDms = ");
			sb->AppendU16((ieBuff[5] >> 2) & 1);
			sb->Append((const UTF8Char*)"\r\n\tUtc Tsf Offset = ");
			sb->AppendU16((ieBuff[5] >> 3) & 1);
			sb->Append((const UTF8Char*)"\r\n\tTpu Buffer Sta Support = ");
			sb->AppendU16((ieBuff[5] >> 4) & 1);
			sb->Append((const UTF8Char*)"\r\n\tTdls Peer Psm Support = ");
			sb->AppendU16((ieBuff[5] >> 5) & 1);
			sb->Append((const UTF8Char*)"\r\n\tTdls Channel Switching = ");
			sb->AppendU16((ieBuff[5] >> 6) & 1);
			sb->Append((const UTF8Char*)"\r\n\tInterworking = ");
			sb->AppendU16((ieBuff[5] >> 7) & 1);
			sb->Append((const UTF8Char*)"\r\n\tQos Map = ");
			sb->AppendU16((ieBuff[6] >> 0) & 1);
			sb->Append((const UTF8Char*)"\r\n\tEbr = ");
			sb->AppendU16((ieBuff[6] >> 1) & 1);
			sb->Append((const UTF8Char*)"\r\n\tSspn Interface = ");
			sb->AppendU16((ieBuff[6] >> 2) & 1);
			sb->Append((const UTF8Char*)"\r\n\tMsgcf Capability = ");
			sb->AppendU16((ieBuff[6] >> 4) & 1);
			sb->Append((const UTF8Char*)"\r\n\tTdls Support = ");
			sb->AppendU16((ieBuff[6] >> 5) & 1);
			sb->Append((const UTF8Char*)"\r\n\tTdls Prohibited = ");
			sb->AppendU16((ieBuff[6] >> 6) & 1);
			sb->Append((const UTF8Char*)"\r\n\tTdls Channel Switching Prohibited = ");
			sb->AppendU16((ieBuff[6] >> 7) & 1);
			sb->Append((const UTF8Char*)"\r\n\tReject Unadmitted Frame = ");
			sb->AppendU16((ieBuff[7] >> 0) & 1);
			sb->Append((const UTF8Char*)"\r\n\tService Interval Granularity = ");
			sb->AppendU16((ieBuff[7] >> 1) & 7);
			sb->Append((const UTF8Char*)"\r\n\tIdentifier Location = ");
			sb->AppendU16((ieBuff[7] >> 4) & 1);
			sb->Append((const UTF8Char*)"\r\n\tUapsd Coexistence = ");
			sb->AppendU16((ieBuff[7] >> 5) & 1);
			sb->Append((const UTF8Char*)"\r\n\tWnm Notification = ");
			sb->AppendU16((ieBuff[7] >> 6) & 1);
			sb->Append((const UTF8Char*)"\r\n\tQab Capability = ");
			sb->AppendU16((ieBuff[7] >> 7) & 1);
			sb->Append((const UTF8Char*)"\r\n\tUtf8 Ssid = ");
			sb->AppendU16((ieBuff[8] >> 0) & 1);
			sb->Append((const UTF8Char*)"\r\n\tQmf Activated = ");
			sb->AppendU16((ieBuff[8] >> 1) & 1);
			sb->Append((const UTF8Char*)"\r\n\tQmf Reconfiguration Activated = ");
			sb->AppendU16((ieBuff[8] >> 2) & 1);
			sb->Append((const UTF8Char*)"\r\n\tRobust Av Streaming = ");
			sb->AppendU16((ieBuff[8] >> 3) & 1);
			sb->Append((const UTF8Char*)"\r\n\tAdvanced Gcr = ");
			sb->AppendU16((ieBuff[8] >> 4) & 1);
			sb->Append((const UTF8Char*)"\r\n\tMesh Gcr = ");
			sb->AppendU16((ieBuff[8] >> 5) & 1);
			sb->Append((const UTF8Char*)"\r\n\tScs = ");
			sb->AppendU16((ieBuff[8] >> 6) & 1);
			sb->Append((const UTF8Char*)"\r\n\tQload Report = ");
			sb->AppendU16((ieBuff[8] >> 7) & 1);
			sb->Append((const UTF8Char*)"\r\n\tAlternate Edca = ");
			sb->AppendU16((ieBuff[9] >> 0) & 1);
			sb->Append((const UTF8Char*)"\r\n\tUnprotected Txop Negotiation = ");
			sb->AppendU16((ieBuff[9] >> 1) & 1);
			sb->Append((const UTF8Char*)"\r\n\tProtected Txop Negotiation = ");
			sb->AppendU16((ieBuff[9] >> 2) & 1);
			sb->Append((const UTF8Char*)"\r\n\tProtected Qload Report = ");
			sb->AppendU16((ieBuff[9] >> 3) & 1);
			sb->Append((const UTF8Char*)"\r\n\tTdls Wider Bandwidth = ");
			sb->AppendU16((ieBuff[9] >> 4) & 1);
			sb->Append((const UTF8Char*)"\r\n\tOperating Mode Notification = ");
			sb->AppendU16((ieBuff[9] >> 5) & 1);
			sb->Append((const UTF8Char*)"\r\n\tMax Number Of Msdus In Amsdu = ");
			sb->AppendU16((ieBuff[9] >> 6) & 3);
		}
		else
		{
			sb->Append((const UTF8Char*)"Extended Capabilities - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		return;
	case 0xBF:
		if (size == 12)
		{
			UInt32 v32;
			sb->Append((const UTF8Char*)"VHT Capabilities:");
			sb->Append((const UTF8Char*)"\r\n\tVHT Capabilities Info = 0x");
			sb->AppendHex32(v32 = ReadUInt32(&ieBuff[2]));
			sb->Append((const UTF8Char*)"\r\n\t\tMax Mpdu Length = ");
			sb->AppendU16((v32 >> 0) & 3);
			sb->Append((const UTF8Char*)"\r\n\t\tSupported Channel Width Set = ");
			sb->AppendU16((v32 >> 2) & 3);
			sb->Append((const UTF8Char*)"\r\n\t\tRx Ldpc = ");
			sb->AppendU16((v32 >> 4) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tShort Guard Interval For 80Mhz = ");
			sb->AppendU16((v32 >> 5) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tShort Guard Interval For 160Mhz = ");
			sb->AppendU16((v32 >> 6) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tTx STBC = ");
			sb->AppendU16((v32 >> 7) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tRx STBC = ");
			sb->AppendU16((v32 >> 8) & 7);
			sb->Append((const UTF8Char*)"\r\n\t\tSu Beamformer Capable = ");
			sb->AppendU16((v32 >> 11) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tSu Beamformee Capable = ");
			sb->AppendU16((v32 >> 12) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tBeamformee Sts Capable = ");
			sb->AppendU16((v32 >> 13) & 7);
			sb->Append((const UTF8Char*)"\r\n\t\tNumber Of Sounding Dimensions = ");
			sb->AppendU16((v32 >> 16) & 7);
			sb->Append((const UTF8Char*)"\r\n\t\tMu Beamformer Capable = ");
			sb->AppendU16((v32 >> 19) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tMu Beamformee Capable = ");
			sb->AppendU16((v32 >> 20) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tVht Txop Ps = ");
			sb->AppendU16((v32 >> 21) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tHtc Vht Capable = ");
			sb->AppendU16((v32 >> 22) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tMax Ampdu Length Exponent = ");
			sb->AppendU16((v32 >> 23) & 7);
			sb->Append((const UTF8Char*)"\r\n\t\tVht Link Adaptation Capable = ");
			sb->AppendU16((v32 >> 26) & 3);
			sb->Append((const UTF8Char*)"\r\n\t\tRx Antenna Pattern Consistency = ");
			sb->AppendU16((v32 >> 28) & 1);
			sb->Append((const UTF8Char*)"\r\n\t\tTx Antenna Pattern Consistency = ");
			sb->AppendU16((v32 >> 29) & 1);
			sb->Append((const UTF8Char*)"\r\n\tSupported Mcs And Nss Set = 0x");
			sb->AppendHex64(ReadUInt64(&ieBuff[6]));
		}
		else
		{
			sb->Append((const UTF8Char*)"VHT Capabilities - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		
		return;
	case 0xC0:
		if (size == 5)
		{
			sb->Append((const UTF8Char*)"VHT Operation:");
			sb->Append((const UTF8Char*)"\r\n\tChannelWidth = ");
			sb->AppendU16(ieBuff[2]);
			sb->Append((const UTF8Char*)"\r\n\tChannel Center Frequency Segment 0 = ");
			sb->AppendU16(ieBuff[3]);
			sb->Append((const UTF8Char*)"\r\n\tChannel Center Frequency Segment 1 = ");
			sb->AppendU16(ieBuff[4]);
			sb->Append((const UTF8Char*)"\r\n\tBasic Vht Mcs And Nss Set = ");
			sb->AppendU16(ReadUInt16(&ieBuff[5]));
		}
		else
		{
			sb->Append((const UTF8Char*)"VHT Operation - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		return;
	case 0xC3:
		if (size == 4)
		{
			sb->Append((const UTF8Char*)"VHT Transmit Power Envelope:");
			sb->Append((const UTF8Char*)"\r\n\tNumber of Segments = ");
			sb->AppendU16(ieBuff[2]);
			sb->Append((const UTF8Char*)"\r\n\tChannel Center Frequency Segment = ");
			sb->AppendU16(ieBuff[3]);
			sb->Append((const UTF8Char*)"\r\n\tSegment Segment Width = ");
			sb->AppendU16(ieBuff[4]);
			sb->Append((const UTF8Char*)"\r\n\tMaximum Transmit Power = ");
			sb->AppendU16(ieBuff[5]);
		}
		else
		{
			sb->Append((const UTF8Char*)"VHT Transmit Power Envelope - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
		}
		return;
	case 0xDD:
		if (size < 4)
		{
			sb->Append((const UTF8Char*)"Vendor Specific - ");
			sb->AppendHexBuff(&ieBuff[2], size, 0, Text::LBT_NONE);
			return;
		}
		sb->Append((const UTF8Char*)"Vendor Specific:");
		sb->Append((const UTF8Char*)"\r\n\tOUI = ");
		sb->AppendHexBuff(&ieBuff[2], 3, '-', Text::LBT_NONE);
		sb->Append((const UTF8Char*)" (");
		sb->Append((const UTF8Char*)Net::MACInfo::GetMACInfoOUI(&ieBuff[2])->name);
		sb->Append((const UTF8Char*)")");
		sb->Append((const UTF8Char*)"\r\n\tOUI Type = ");
		sb->AppendU16(ieBuff[5]);
		Bool succ = false;
		UInt32 v32 = ReadMUInt32(&ieBuff[2]);
		if (v32 == 0x0050F201)
		{
			sb->Append((const UTF8Char*)" (WPA Information Element)");
		}
		else if (v32 == 0x0050F202)
		{
			sb->Append((const UTF8Char*)" (WMM/WME: Parameter Element)");
			if (ieBuff[6] == 1 && size >= 8)
			{
				succ = true;
				sb->Append((const UTF8Char*)"\r\n\tWME Subtype = 1 (Parameter Element)");
				sb->Append((const UTF8Char*)"\r\n\tWME Version = ");
				sb->AppendU16(ieBuff[7]);
				sb->Append((const UTF8Char*)"\r\n\tWME QoS Info = 0x");
				sb->AppendHex8(ieBuff[8]);
				sb->Append((const UTF8Char*)"\r\n\t\tU-APSD = ");
				sb->AppendU16(ieBuff[8] >> 7);
				sb->Append((const UTF8Char*)"\r\n\t\tParameter Set Count = ");
				sb->AppendU16(ieBuff[8] & 15);
				sb->Append((const UTF8Char*)"\r\n\tReserved = ");
				sb->AppendU16(ieBuff[9]);
				i = 8;
				while (i <= size - 4)
				{
					sb->Append((const UTF8Char*)"\r\n\tACI Type = ");
					sb->AppendU16((ieBuff[i + 2] >> 5) & 3);
					switch ((ieBuff[i + 2] >> 5) & 3)
					{
					case 0:
						sb->Append((const UTF8Char*)" (Best Effort)");
						break;
					case 1:
						sb->Append((const UTF8Char*)" (Background)");
						break;
					case 2:
						sb->Append((const UTF8Char*)" (Video)");
						break;
					case 3:
						sb->Append((const UTF8Char*)" (Voice)");
						break;
					}
					sb->Append((const UTF8Char*)"\r\n\tAdmission Control Mandatory = ");
					sb->AppendU16((ieBuff[i + 2] >> 4) & 1);
					sb->Append((const UTF8Char*)"\r\n\tAIFSN = ");
					sb->AppendU16((ieBuff[i + 2] >> 0) & 15);
					sb->Append((const UTF8Char*)"\r\n\tECW Min = ");
					sb->AppendU16((ieBuff[i + 3] >> 0) & 15);
					sb->Append((const UTF8Char*)"\r\n\tECW Max = ");
					sb->AppendU16((ieBuff[i + 3] >> 4) & 15);
					sb->Append((const UTF8Char*)"\r\n\tTXOP Limit = ");
					sb->AppendU16(ReadUInt16(&ieBuff[i + 4]));
					i += 4;
				}
			}
		}
		else if (v32 == 0x0050F204)
		{
			UInt16 itemId;
			UInt16 itemSize;
			succ = true;
			sb->Append((const UTF8Char*)" (WPS Information Element)");
			i = 4;
			while (i <= size - 4)
			{
				itemId = ReadMUInt16(&ieBuff[i + 2]);
				itemSize = ReadMUInt16(&ieBuff[i + 4]);
				if (i + 4 + itemSize > size)
				{
					break;
				}
				switch (itemId)
				{
				case 0x1008:
					sb->Append((const UTF8Char*)"\r\n\tConfig methods = ");
					sb->AppendHexBuff(&ieBuff[i + 6], itemSize, ',', Text::LBT_NONE);
					break;
				case 0x1011:
					sb->Append((const UTF8Char*)"\r\n\tDevice Name = ");
					sb->AppendC(&ieBuff[i + 6], itemSize);
					break;
				case 0x1021:
					sb->Append((const UTF8Char*)"\r\n\tManufacture = ");
					sb->AppendC(&ieBuff[i + 6], itemSize);
					break;
				case 0x1023:
					sb->Append((const UTF8Char*)"\r\n\tModel = ");
					sb->AppendC(&ieBuff[i + 6], itemSize);
					break;
				case 0x1024:
					sb->Append((const UTF8Char*)"\r\n\tModel Number = ");
					sb->AppendC(&ieBuff[i + 6], itemSize);
					break;
				case 0x103B:
					sb->Append((const UTF8Char*)"\r\n\tResponse Type = ");
					if (itemSize == 1)
					{
						sb->AppendU16(ieBuff[i + 6]);
						switch (ieBuff[i + 6])
						{
						case 3:
							sb->Append((const UTF8Char*)" (AP)");
							break;
						}
					}
					else
					{
						sb->AppendC(&ieBuff[i + 6], itemSize);
					}
					break;
				case 0x103C:
					sb->Append((const UTF8Char*)"\r\n\tRF Bands = 0x");
					if (itemSize == 1)
					{
						sb->AppendHex8(ieBuff[i + 6]);
					}
					break;
				case 0x1042:
					sb->Append((const UTF8Char*)"\r\n\tSerial = ");
					sb->AppendC(&ieBuff[i + 6], itemSize);
					break;
				case 0x1044:
					sb->Append((const UTF8Char*)"\r\n\tWi-Fi Protected Setup State = ");
					if (itemSize == 1)
					{
						sb->AppendU16(ieBuff[i + 6]);
					}
					break;
				case 0x1047:
					sb->Append((const UTF8Char*)"\r\n\tUUID = ");
					if (itemSize == 16)
					{
						sb->AppendHexBuff(&ieBuff[i + 6], 4, 0, Text::LBT_NONE);
						sb->AppendChar('-', 1);
						sb->AppendHexBuff(&ieBuff[i + 10], 2, 0, Text::LBT_NONE);
						sb->AppendChar('-', 1);
						sb->AppendHexBuff(&ieBuff[i + 12], 2, 0, Text::LBT_NONE);
						sb->AppendChar('-', 1);
						sb->AppendHexBuff(&ieBuff[i + 14], 2, 0, Text::LBT_NONE);
						sb->AppendChar('-', 1);
						sb->AppendHexBuff(&ieBuff[i + 16], 6, 0, Text::LBT_NONE);
					}
					break;
				case 0x1049:
					sb->Append((const UTF8Char*)"\r\n\tVendor Extension:");
					if (itemSize >= 3)
					{
						sb->Append((const UTF8Char*)"\r\n\t\tVendor OUI: ");
						sb->AppendHexBuff(&ieBuff[i + 6], 3, '-', Text::LBT_NONE);
						sb->Append((const UTF8Char*)" (");
						sb->Append((const UTF8Char*)Net::MACInfo::GetMACInfoOUI(&ieBuff[i + 6])->name);
						sb->AppendChar(')', 1);
						OSInt j = 3;
						while (j < itemSize - 2)
						{
							if (j + 2 + ieBuff[i + 6 + j + 1] > itemSize)
								break;
							sb->Append((const UTF8Char*)"\r\n\t\tType=0x");
							sb->AppendHex8(ieBuff[i + 6 + j]);
							switch (ieBuff[i + 6 + j])
							{
							case 0:
								sb->Append((const UTF8Char*)" (Version2)");
								break;
							case 3:
								sb->Append((const UTF8Char*)" (Request to Enroll)");
								break;
							}
							sb->Append((const UTF8Char*)", Value=");
							sb->AppendHexBuff(&ieBuff[i + 6 + j + 2], ieBuff[i + 6 + j + 1], ' ', Text::LBT_NONE);
							j += ieBuff[i + 6 + j + 1] + 2;
						}
					}
				case 0x104A:
					sb->Append((const UTF8Char*)"\r\n\tVersion = ");
					if (itemSize == 1)
					{
						sb->AppendU16(ieBuff[i + 6] >> 4);
						sb->AppendChar('.', 1);
						sb->AppendU16(ieBuff[i + 6] & 15);
					}
					break;
				case 0x1054:
					sb->Append((const UTF8Char*)"\r\n\tPrimary Device Type = ");
					if (itemSize == 8)
					{
						sb->AppendU16(ReadMUInt16(&ieBuff[i + 6]));
						sb->AppendChar('-', 1);
						sb->AppendHexBuff(&ieBuff[i + 8], 4, 0, Text::LBT_NONE);
						sb->AppendChar('-', 1);
						sb->AppendU16(ReadMUInt16(&ieBuff[i + 12]));
					}
					break;
				default:
					sb->Append((const UTF8Char*)"\r\n\tWPS IE ");
					sb->AppendHexBuff(&ieBuff[i + 2], itemSize + 4, 0, Text::LBT_NONE);
					break;
				}
				i += 4 + itemSize; 
			}
		}
		if (!succ)
		{
			sb->Append((const UTF8Char*)"\r\n\tContent = ");
			sb->AppendHexBuff(&ieBuff[6], size - 4, 0, Text::LBT_NONE);
		}
		return;
	}
	sb->Append((const UTF8Char*)"IE ");
	sb->AppendHexBuff(ieBuff, size + 2, 0, Text::LBT_NONE);
}