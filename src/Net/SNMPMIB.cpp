#include "Stdafx.h"
#include "MyMemory.h"
#include "IO/FileStream.h"
#include "IO/Path.h"
#include "Net/SNMPUtil.h"
#include "Net/SNMPMIB.h"
#include "Text/StringBuilderUTF8.h"
#include "Text/UTF8Reader.h"
#define DEBUGOBJ "gdStatusBytes"

OSInt Net::SNMPMIB::CalcLineSpace(const UTF8Char *txt)
{
	OSInt ret = 0;
	UTF8Char c;
	while ((c = *txt++))
	{
		if (c == ' ')
		{
			ret++;
		}
		else if (c == '\t')
		{
			ret += 4;
		}
		else
		{
			return ret;
		}
	}
	return ret;
}

void Net::SNMPMIB::ModuleAppendOID(Net::SNMPMIB::ModuleInfo *module, ObjectInfo *obj)
{
	OSInt i;
	OSInt j;
	OSInt k;
	OSInt l;
	ObjectInfo *obj2;
	if (obj->oidLen <= 0)
		return;
	i = 0;
	j = module->oidList->GetCount() - 1;
	while (i <= j)
	{
		k = (i + j) >> 1;
		obj2 = module->oidList->GetItem(k);
		l = Net::SNMPUtil::OIDCompare(obj2->oid, obj2->oidLen, obj->oid, obj->oidLen);
		if (l > 0)
		{
			j = k - 1;
		}
		else if (l < 0)
		{
			i = k + 1;
		}
		else
		{
			return;
		}
	}
	module->oidList->Insert(i, obj);
}

Bool Net::SNMPMIB::ParseObjectOID(ModuleInfo *module, ObjectInfo *obj, const UTF8Char *s, Text::StringBuilderUTF *errMessage)
{
	const UTF8Char *oriS = s;
	UTF8Char c;
	const UTF8Char *oidName;
	OSInt oidNameLen;
	const UTF8Char *oidNextLev;
	OSInt oidNextLen;
	Bool isFirst = false;
	Text::StringBuilderUTF8 sb;
	while (true)
	{
		c = *s++;
		if (c == ' ')
		{

		}
		else if (c == '{')
		{
			break;
		}
		else
		{
			errMessage->Append((const UTF8Char*)"OID Format error: \"");
			errMessage->Append(oriS);
			errMessage->Append((const UTF8Char*)"\"");
			return false;
		}
	}
	while (true)
	{
		c = *s++;
		if (c == ' ')
		{

		}
		else if (c == '}' || c == 0)
		{
			errMessage->Append((const UTF8Char*)"OID Format error: \"");
			errMessage->Append(oriS);
			errMessage->Append((const UTF8Char*)"\"");
			return false;
		}
		else
		{
			oidName = s - 1;
			break;
		}
	}
	while (true)
	{
		c = *s++;
		if (c == ' ')
		{
			oidNameLen = s - oidName - 1;
			break;
		}
		else if (c == '}' || c == 0)
		{
			errMessage->Append((const UTF8Char*)"OID Format error: \"");
			errMessage->Append(oriS);
			errMessage->Append((const UTF8Char*)"\"");
			return false;
		}
	}
	sb.ClearStr();
	sb.AppendC(oidName, oidNameLen);
	if (sb.Equals((const UTF8Char*)"iso"))
	{
		obj->oid[0] = 40;
		obj->oidLen = 1;
		isFirst = true;
	}
	else if (sb.Equals((const UTF8Char*)"iso(1)"))
	{
		obj->oid[0] = 40;
		obj->oidLen = 1;
		isFirst = true;
	}
	else if (sb.Equals((const UTF8Char*)"0"))
	{
		obj->oid[0] = 0;
		obj->oidLen = 1;
		isFirst = true;
	}
	else
	{
		OSInt i = module->objKeys->SortedIndexOf(sb.ToString());
		ObjectInfo *obj2;
		if (i < 0)
		{
//			i = this->globalModule.objKeys->SortedIndexOf(sb.ToString());
//			if (i < 0)
//			{
				errMessage->Append((const UTF8Char*)"OID Name \"");
				errMessage->Append(sb.ToString());
				errMessage->Append((const UTF8Char*)"\" not found");
				return false;
//			}
//			else
//			{
//				obj2 = this->globalModule.objValues->GetItem(i);
//			}
		}
		else
		{
			obj2 = module->objValues->GetItem(i);
		}
		if (obj2->oidLen == 0)
		{
			if (obj2->typeVal != 0)
			{
				if (!ParseObjectOID(module, obj2, obj2->typeVal, errMessage))
				{
					return false;
				}
			}
		}
		if (obj2->oidLen == 0)
		{
			errMessage->Append((const UTF8Char*)"OID Name \"");
			errMessage->Append(sb.ToString());
			errMessage->Append((const UTF8Char*)"\" is not OID");
			return false;
		}
		MemCopyNO(obj->oid, obj2->oid, obj2->oidLen);
		obj->oidLen = obj2->oidLen;
	}
	
	while (true)
	{
		while (true)
		{
			c = *s++;
			if (c == ' ')
			{

			}
			else if (c == '}')
			{
				while (true)
				{
					c = *s++;
					if (c == ' ')
					{
					}
					else if (c == 0)
					{
						return true;
					}
					else
					{
						errMessage->Append((const UTF8Char*)"OID Format error: \"");
						errMessage->Append(oriS);
						errMessage->Append((const UTF8Char*)"\"");
						return false;
					}
				}
			}
			else
			{
				oidNextLev = s - 1;
				break;
			}
		}
		while (true)
		{
			c = *s++;
			if (c == ' ' || c == '}')
			{
				oidNextLen = s - oidNextLev - 1;
				if (c == '}')
				{
					s--;
				}
				break;
			}
		}

		UInt32 v;
		OSInt i;
		OSInt j;
		sb.ClearStr();
		sb.AppendC(oidNextLev, oidNextLen);
		i = sb.IndexOf('(');
		j = sb.IndexOf(')');
		if (i >= 0 && j > i)
		{
			sb.SetSubstr(i + 1);
			sb.TrimToLength(j - i - 1);
			if (!sb.ToUInt32(&v))
			{
				errMessage->Append((const UTF8Char*)"OID Format error: \"");
				errMessage->Append(oriS);
				errMessage->Append((const UTF8Char*)"\"");
				return false;
			}
		}
		else
		{
			if (!sb.ToUInt32(&v))
			{
				errMessage->Append((const UTF8Char*)"OID Format error: \"");
				errMessage->Append(oriS);
				errMessage->Append((const UTF8Char*)"\"");
				return false;
			}
		}
		
		if (v < 128)
		{
			if (isFirst)
			{
				obj->oid[0] += (UInt8)v;
				isFirst = false;
			}
			else
			{
				obj->oid[obj->oidLen] = (UInt8)v;
				obj->oidLen = obj->oidLen + 1;
			}
			
		}
		else if (v < 0x4000)
		{
			obj->oid[obj->oidLen] = (UInt8)(0x80 | (v >> 7));
			obj->oid[obj->oidLen + 1] = (UInt8)(v & 0x7f);
			obj->oidLen = obj->oidLen + 2;
		}
		else if (v < 0x200000)
		{
			obj->oid[obj->oidLen] = (UInt8)(0x80 | (v >> 14));
			obj->oid[obj->oidLen + 1] = (UInt8)(0x80 | ((v >> 7) & 0x7f));
			obj->oid[obj->oidLen + 2] = (UInt8)(v & 0x7f);
			obj->oidLen = obj->oidLen + 3;
		}
		else if (v < 0x10000000)
		{
			obj->oid[obj->oidLen] = (UInt8)(0x80 | (v >> 21));
			obj->oid[obj->oidLen + 1] = (UInt8)(0x80 | ((v >> 14) & 0x7f));
			obj->oid[obj->oidLen + 2] = (UInt8)(0x80 | ((v >> 7) & 0x7f));
			obj->oid[obj->oidLen + 3] = (UInt8)(v & 0x7f);
			obj->oidLen = obj->oidLen + 4;
		}
		else
		{
			obj->oid[obj->oidLen] = (UInt8)(0x80 | (v >> 28));
			obj->oid[obj->oidLen + 1] = (UInt8)(0x80 | ((v >> 21) & 0x7f));
			obj->oid[obj->oidLen + 2] = (UInt8)(0x80 | ((v >> 14) & 0x7f));
			obj->oid[obj->oidLen + 3] = (UInt8)(0x80 | ((v >> 7) & 0x7f));
			obj->oid[obj->oidLen + 4] = (UInt8)(v & 0x7f);
			obj->oidLen = obj->oidLen + 5;
		}
	}

	return true;
}

Bool Net::SNMPMIB::ParseObjectBegin(Text::UTF8Reader *reader, ObjectInfo *obj, Text::StringBuilderUTF *errMessage)
{
	Text::StringBuilderUTF8 sb;
	OSInt i;
	while (true)
	{
		sb.ClearStr();
		if (!reader->ReadLine(&sb, 512))
		{
			errMessage->Append((const UTF8Char*)"Object end not found");
			return false;
		}

		i = sb.IndexOf((const UTF8Char*)"--");
		if (i >= 0)
		{
			sb.RemoveChars(sb.GetLength() - i);
		}
		sb.TrimRight();
		if (sb.GetLength() > 0)
		{
			if (sb.EndsWith((const UTF8Char*)"BEGIN"))
			{
				errMessage->Append((const UTF8Char*)"Nested begin found");
				return false;
			}
			else if (sb.EndsWith((const UTF8Char*)"END"))
			{
				return true;
			}
		}
	}
}

Bool Net::SNMPMIB::ParseModule(Text::UTF8Reader *reader, ModuleInfo *module, Text::StringBuilderUTF *errMessage)
{
	Text::StringBuilderUTF8 sb;
	OSInt i;
	OSInt j;
	OSInt lineSpace;
	ObjectInfo *obj;
	ObjectInfo *currObj = 0;
	Text::StringBuilderUTF8 sbObjValName;
	Text::StringBuilderUTF8 sbObjValCont;
	OSInt objLineSpace = 0;
	Bool objIsEqual = false;
	Bool objIsBrk = false;
	Bool succ;
	Bool isQuotedText = false;
	
	while (true)
	{
		sb.ClearStr();
		if (!reader->ReadLine(&sb, 512))
		{
			errMessage->Append((const UTF8Char*)"Module end not found");
			return false;
		}

		i = sb.IndexOf((const UTF8Char*)"--");
		if (i >= 0)
		{
			sb.RemoveChars(sb.GetLength() - i);
		}
		sb.TrimRight();
		if (currObj && currObj->objectName && Text::StrEquals(currObj->objectName, (const UTF8Char*)"PSSEQStringEntry"))
		{
			i = 0;
		}

		if (sb.GetLength() > 0)
		{
			if (sb.EndsWith((const UTF8Char*)"BEGIN"))
			{
				succ = ParseObjectBegin(reader, 0, errMessage);
				if (!succ)
				{
					return succ;
				}
				currObj = 0;
			}
			else if (sb.EndsWith((const UTF8Char*)"END"))
			{
				Data::ArrayList<ObjectInfo*> *objList = module->objValues;
				ObjectInfo *obj;
				i = 0;
				j = objList->GetCount();
				while (i < j)
				{
					obj = objList->GetItem(i);
					if (obj->typeName && obj->typeVal && obj->oidLen == 0 && !Text::StrEquals(obj->typeName, (const UTF8Char*)"TRAP-TYPE") && !Text::StrEquals(obj->typeVal, (const UTF8Char*)"Imported Value"))
					{
						succ = this->ParseObjectOID(module, obj, obj->typeVal, errMessage);
						if (!succ)
						{
							return false;
						}
						ModuleAppendOID(module, obj);
						ModuleAppendOID(&this->globalModule, obj);
					}
					else if (obj->oidLen > 0)
					{
						ModuleAppendOID(module, obj);
						ModuleAppendOID(&this->globalModule, obj);
					}
					i++;
				}
				return true;
			}
			else if (isQuotedText)
			{
				if (currObj)
				{
					sbObjValCont.Append(sb.ToString());
				}
				if (sb.EndsWith('"'))
				{
					isQuotedText = false;
					if (currObj)
					{
						if (sbObjValName.GetLength() > 0 && sbObjValCont.GetLength() > 0)
						{
							currObj->valName->Add(Text::StrCopyNew(sbObjValName.ToString()));
							currObj->valCont->Add(Text::StrCopyNew(sbObjValCont.ToString()));
						}
						sbObjValName.ClearStr();
						sbObjValCont.ClearStr();
					}
				}
				else
				{
					reader->GetLastLineBreak(&sbObjValCont);
				}
				
			}
			else
			{
				lineSpace = CalcLineSpace(sb.ToString());
				sb.Trim();
				if (currObj && (objIsBrk || objIsEqual || (lineSpace > objLineSpace && sb.ToString()[0] >= 'A' && sb.ToString()[0] <= 'Z') || sb.StartsWith((const UTF8Char*)"::=") || sb.StartsWith((const UTF8Char*)"{") || sb.StartsWith((const UTF8Char*)"\"")))
				{
					if (objIsBrk)
					{
						if (objIsEqual)
						{
							Text::StringBuilderUTF8 sbTmp;
							sbTmp.Append(currObj->typeVal);
							sbTmp.AppendChar(' ', 1);
							sbTmp.Append(sb.ToString());
							Text::StrDelNew(currObj->typeVal);
							currObj->typeVal = Text::StrCopyNew(sbTmp.ToString());
						}
						else
						{
							sbObjValCont.Append(sb.ToString());
						}
						
						if (sb.EndsWith((const UTF8Char*)"}"))
						{
							objIsBrk = false;
							objIsEqual = false;
							if (sbObjValName.GetLength() > 0 && sbObjValCont.GetLength() > 0)
							{
								currObj->valName->Add(Text::StrCopyNew(sbObjValName.ToString()));
								currObj->valCont->Add(Text::StrCopyNew(sbObjValCont.ToString()));
							}
							sbObjValName.ClearStr();
							sbObjValCont.ClearStr();
							if (currObj->typeName == 0)
							{
								UTF8Char *typeVal = (UTF8Char*)currObj->typeVal;
								if (typeVal)
								{
									OSInt strLen = Text::StrCharCnt(typeVal);
									OSInt wsCnt = 0;
									while (strLen-- > 0)
									{
										if (typeVal[strLen] == '\r' || typeVal[strLen] == '\n' || typeVal[strLen] == '\t' || typeVal[strLen] == ' ')
										{
											typeVal[strLen] = ' ';
											wsCnt++;
										}
										else if (wsCnt > 0)
										{
											if (wsCnt > 1)
											{
												Text::StrLTrim(&typeVal[strLen + 2]);
											}
											wsCnt = 0;
										}
									}
								}
								currObj = 0;
							}
						}
					}
					else if (objIsEqual)
					{
						if (sb.StartsWith((const UTF8Char*)"[") && sb.EndsWith((const UTF8Char*)"]"))
						{

						}
						else
						{
							currObj->typeVal = Text::StrCopyNew(sb.ToString());
							i = Text::StrIndexOf(currObj->typeVal, '{');
							j = Text::StrIndexOf(currObj->typeVal, '}');
							if (i >= 0)
							{
								if (j > i)
								{
									currObj = 0;
									objIsEqual = false;
								}
								else
								{
									objIsBrk = true;
								}
							}
							else
							{
								currObj = 0;
								objIsEqual = false;
							}
						}
					}
					else if (sb.StartsWith((const UTF8Char*)"::="))
					{
						if (sbObjValName.GetLength() > 0 && sbObjValCont.GetLength() > 0)
						{
							currObj->valName->Add(Text::StrCopyNew(sbObjValName.ToString()));
							currObj->valCont->Add(Text::StrCopyNew(sbObjValCont.ToString()));
						}
						sbObjValName.ClearStr();
						sbObjValCont.ClearStr();

						i = 3;
						while (sb.ToString()[i] == ' ' || sb.ToString()[i] == '\t')
						{
							i++;
						}
						currObj->typeVal = Text::StrCopyNew(sb.ToString() + i);
						if (currObj->typeName != 0 && Text::StrEndsWith(currObj->typeVal, (const UTF8Char*)"}"))
						{
							currObj = 0;
							objIsBrk = false;
							objIsEqual = false;
						}							
						else if (Text::StrEndsWith(currObj->typeVal, (const UTF8Char*)"{"))
						{
							objIsBrk = true;
							objIsEqual = true;
						}
					}
					else if (sb.StartsWith((const UTF8Char*)"{"))
					{
						sbObjValCont.Append(sb.ToString());
						if (sb.EndsWith((const UTF8Char*)"}"))
						{
							objIsBrk = false;
							objIsEqual = false;
							if (sbObjValName.GetLength() > 0 && sbObjValCont.GetLength() > 0)
							{
								currObj->valName->Add(Text::StrCopyNew(sbObjValName.ToString()));
								currObj->valCont->Add(Text::StrCopyNew(sbObjValCont.ToString()));
							}
							sbObjValName.ClearStr();
							sbObjValCont.ClearStr();
						}
						else
						{
							objIsBrk = true;
							objIsEqual = false;
						}
					}
					else if (sb.StartsWith((const UTF8Char*)"\""))
					{
						sbObjValCont.Append(sb.ToString());
						if (sb.GetLength() > 1 && sb.EndsWith((const UTF8Char*)"\""))
						{
							if (sbObjValName.GetLength() > 0 && sbObjValCont.GetLength() > 0)
							{
								currObj->valName->Add(Text::StrCopyNew(sbObjValName.ToString()));
								currObj->valCont->Add(Text::StrCopyNew(sbObjValCont.ToString()));
							}
							sbObjValName.ClearStr();
							sbObjValCont.ClearStr();
						}
						else
						{
							isQuotedText = true;
						}
					}
					else
					{
						Bool proc = false;
						if (sbObjValName.GetLength() > 0 && sbObjValCont.GetLength() > 0)
						{
							if (sb.StartsWith((const UTF8Char*)"{") || sb.StartsWith((const UTF8Char*)"\""))
							{
								sbObjValCont.AppendChar(' ', 1);
								sbObjValCont.Append(sb.ToString());
								proc = true;
							}
							else
							{
								currObj->valName->Add(Text::StrCopyNew(sbObjValName.ToString()));
								currObj->valCont->Add(Text::StrCopyNew(sbObjValCont.ToString()));
								sbObjValName.ClearStr();
								sbObjValCont.ClearStr();
							}
						}
						else if (sbObjValName.GetLength() > 0)
						{
							sbObjValCont.Append(sb.ToString());
							proc = true;
						}

						if (!proc)
						{
							i = sb.IndexOf(' ');
							j = sb.IndexOf('\t');
							if (i < 0)
							{
								i = j;
							}
							else if (j >= 0 && j < i)
							{
								i = j;
							}
							
							if (i < 0)
							{
								sbObjValName.Append(sb.ToString());
							}
							else
							{
								sbObjValName.AppendC(sb.ToString(), i);
								sbObjValCont.Append(sb.ToString() + i + 1);
								sbObjValCont.Trim();
							}
							
						}

						if ((i = sb.IndexOf('\"')) >= 0)
						{
							j = sb.IndexOf((const UTF8Char*)"\"", i + 1);
							if (j < 0)
							{
								reader->GetLastLineBreak(&sbObjValCont);
								isQuotedText = true;
							}
						}
						else if ((i = sb.IndexOf((const UTF8Char*)"{")) >= 0)
						{
							j = sb.IndexOf((const UTF8Char*)"}");
							if (j > i)
							{
								if (sbObjValName.GetLength() > 0 && sbObjValCont.GetLength() > 0)
								{
									currObj->valName->Add(Text::StrCopyNew(sbObjValName.ToString()));
									currObj->valCont->Add(Text::StrCopyNew(sbObjValCont.ToString()));
								}
								sbObjValName.ClearStr();
								sbObjValCont.ClearStr();
							}
							else
							{
								objIsBrk = true;
								objIsEqual = false;
							}
						}
					}
				}
				else if (sb.StartsWith((const UTF8Char*)"IMPORTS"))
				{
					Bool isEnd = false;
					Text::StringBuilderUTF8 impObjNames;
					Net::SNMPMIB::ModuleInfo *impModule;
					Net::SNMPMIB::ObjectInfo *impObj;
					Net::SNMPMIB::ObjectInfo *impObj2;
					UTF8Char *impSarr[2];
					OSInt impCnt;
					OSInt impInd;
					sb.SetSubstr(7);
					sb.Trim();

					while (!isEnd)
					{
						if (sb.EndsWith(';'))
						{
							isEnd = true;
							sb.RemoveChars(1);
						}
						i = sb.IndexOf((const UTF8Char*)"FROM ");
						if (i >= 0)
						{
							impObjNames.AppendC(sb.ToString(), i);
							impObjNames.TrimRight();
							if ((impModule = this->moduleMap->Get(sb.ToString() + i + 5)) != 0)
							{
							
							}
							else
							{
								UTF8Char sbuff[512];
								Text::StrConcat(sbuff, module->moduleFileName);
								j = Text::StrLastIndexOf(sbuff, IO::Path::PATH_SEPERATOR);
								Text::StrConcat(&sbuff[j + 1], sb.ToString() + i + 5);
								succ = LoadFile(sbuff, errMessage);
								if (!succ)
								{
									return false;
								}
								impModule = this->moduleMap->Get(sb.ToString() + i + 5);
							}
							
							if (impModule == 0)
							{
								errMessage->Append((const UTF8Char*)"IMPORTS module ");
								errMessage->Append(sb.ToString() + i + 5);
								errMessage->Append((const UTF8Char *)" not found");
								return false;
							}
							impSarr[1] = impObjNames.ToString();
							while (true)
							{
								impCnt = Text::StrSplitTrim(impSarr, 2, impSarr[1], ',');
								impInd = impModule->objKeys->SortedIndexOf(impSarr[0]);
								if (impInd >= 0)
								{
									impObj = impModule->objValues->GetItem(impInd);
									impObj2 = MemAlloc(ObjectInfo, 1);
									impObj2->objectName = Text::StrCopyNew(impSarr[0]);
									if (impObj->typeName)
									{
										impObj2->typeName = Text::StrCopyNew(impObj->typeName);
									}
									else
									{
										impObj2->typeName = 0;
									}
									impObj2->typeVal = Text::StrCopyNew((const UTF8Char*)"Imported Value");
									impObj2->oidLen = impObj->oidLen;
									if (impObj->oidLen > 0)
									{
										MemCopyNO(impObj2->oid, impObj->oid, impObj->oidLen);
									}
									NEW_CLASS(impObj2->valName, Data::ArrayList<const UTF8Char*>());
									NEW_CLASS(impObj2->valCont, Data::ArrayList<const UTF8Char*>());
									j = impObj->valName->GetCount();
									impInd = 0;
									while (impInd < j)
									{
										impObj2->valName->Add(Text::StrCopyNew(impObj->valName->GetItem(impInd)));
										impObj2->valCont->Add(Text::StrCopyNew(impObj->valCont->GetItem(impInd)));
										impInd++;
									}
									
									impInd = module->objKeys->SortedInsert(impObj2->objectName);
									module->objValues->Insert(impInd, impObj2);
								}
								else
								{
									errMessage->Append((const UTF8Char*)"IMPORTS object ");
									errMessage->Append(impSarr[0]);
									errMessage->Append((const UTF8Char*)" in module ");
									errMessage->Append(sb.ToString() + i + 5);
									errMessage->Append((const UTF8Char *)" not found");
									return false;
								}
								if (impCnt != 2)
								{
									break;
								}
							}
							impObjNames.ClearStr();
						}
						else
						{
							impObjNames.Append(sb.ToString());
						}
						if (isEnd)
						{
							break;
						}
						sb.ClearStr();
						if (!reader->ReadLine(&sb, 512))
						{
							errMessage->Append((const UTF8Char*)"IMPORTS end not found");
							return false;
						}

						i = sb.IndexOf((const UTF8Char*)"--");
						if (i >= 0)
						{
							sb.RemoveChars(sb.GetLength() - i);
						}
						sb.Trim();
					}
				}
				else if (sb.StartsWith((const UTF8Char*)"EXPORTS"))
				{
					while (true)
					{
						if (sb.EndsWith(';'))
						{
							break;
						}

						sb.ClearStr();
						if (!reader->ReadLine(&sb, 512))
						{
							errMessage->Append((const UTF8Char*)"EXPORTS end not found");
							return false;
						}

						i = sb.IndexOf((const UTF8Char*)"--");
						if (i >= 0)
						{
							sb.RemoveChars(sb.GetLength() - i);
						}
						sb.Trim();
					}
				}
				else
				{
					if (currObj)
					{
						if (sbObjValName.GetLength() > 0 && sbObjValCont.GetLength() > 0)
						{
							currObj->valName->Add(Text::StrCopyNew(sbObjValName.ToString()));
							currObj->valCont->Add(Text::StrCopyNew(sbObjValCont.ToString()));
						}
						sbObjValName.ClearStr();
						sbObjValCont.ClearStr();
					}
					currObj = 0;
					i = sb.IndexOf((const UTF8Char*)"::=");
					if (i == 0)
					{
						errMessage->Append((const UTF8Char*)"::= found at non object location");
						return false;
					}
					if (i < 0)
					{
						i = sb.IndexOf(' ');
						j = sb.IndexOf('\t');
						if (i < 0)
						{
							i = j;
						}
						else if (j >= 0 && j < i)
						{
							i = j;
						}
						if (i >= 0)
						{
							i = -1;
						}
						else
						{
							i = sb.GetLength();
							if (!reader->ReadLine(&sb, 512))
							{
								errMessage->Append((const UTF8Char*)"Unknown format: ");
								errMessage->Append(sb.ToString());
								return false;
							}
							j = sb.IndexOf((const UTF8Char*)"--");
							if (j >= 0)
							{
								sb.TrimToLength(j);
							}
							sb.TrimRight();
							if (sb.ToString()[i] == ' ' || sb.ToString()[i] == '\t')
							{
								i = sb.IndexOf((const UTF8Char*)"::=");
							}
							else
							{
								errMessage->Append((const UTF8Char*)"Unknown format: ");
								errMessage->Append(sb.ToString());
								return false;
							}
						}
					}
					if (i >= 0)
					{
						j = i;
						OSInt k;
						while (sb.ToString()[j - 1] == ' ' || sb.ToString()[j - 1] == '\t')
						{
							j--;
						}
						k = sb.IndexOf(' ');
						OSInt l = sb.IndexOf('\t');
						if (k < 0)
						{
							k = l;
						}
						else if (l >= 0 && l < k)
						{
							k = l;
						}
						obj = MemAlloc(ObjectInfo, 1);
						obj->objectName = Text::StrCopyNewC(sb.ToString(), k);
						if (j > k)
						{
							while (sb.ToString()[k] == ' ' || sb.ToString()[k] == '\t')
							{
								k++;
							}
							obj->typeName = Text::StrCopyNewC(sb.ToString() + k, j - k);
						}
						else
						{
							obj->typeName = 0;
						}
						obj->typeVal = 0;
						obj->oidLen = 0;
						NEW_CLASS(obj->valName, Data::ArrayList<const UTF8Char*>());
						NEW_CLASS(obj->valCont, Data::ArrayList<const UTF8Char*>());
						k = module->objKeys->SortedInsert(obj->objectName);
						module->objValues->Insert(k, obj);
						k = this->globalModule.objKeys->SortedInsert(obj->objectName);
						this->globalModule.objValues->Insert(k, obj);
						if (Text::StrEquals(obj->objectName, (const UTF8Char*)DEBUGOBJ))
						{
							currObj = 0;
						}

						if (sb.EndsWith((const UTF8Char*)"::="))
						{
							currObj = obj;
							objLineSpace = lineSpace;
							objIsEqual = true;
							sbObjValName.ClearStr();
							sbObjValCont.ClearStr();
						}
						else
						{
							i += 3;
							while (sb.ToString()[i] == ' ' || sb.ToString()[i] == '\t')
							{
								i++;
							}
							obj->typeVal = Text::StrCopyNew(sb.ToString() + i);
							currObj = obj;
							sbObjValName.ClearStr();
							sbObjValCont.ClearStr();
							objLineSpace = lineSpace;
							objIsEqual = false;
							objIsBrk = false;
							if (Text::StrEndsWith(obj->typeVal, (const UTF8Char*)"{"))
							{
								objIsBrk = true;
								objIsEqual = true;
							}
							else if (Text::StrEndsWith(obj->typeVal, (const UTF8Char*)"}"))
							{
								currObj = 0;
							}
						}
					}
					else
					{
						i = sb.IndexOf(' ');
						j = sb.IndexOf('\t');
						if (i < 0)
						{
							i = j;
						}
						else if (j >= 0 && j < i)
						{
							i = j;
						}
						if (i < 0)
						{
							i = sb.GetLength();
							if (!reader->ReadLine(&sb, 512))
							{
								errMessage->Append((const UTF8Char*)"Unknown format: ");
								errMessage->Append(sb.ToString());
								return false;
							}
							j = sb.IndexOf((const UTF8Char*)"--");
							if (j >= 0)
							{
								sb.TrimToLength(j);
							}
							sb.TrimRight();
							if (sb.ToString()[i] == ' ' || sb.ToString()[i] == '\t')
							{
							}
							else
							{
								errMessage->Append((const UTF8Char*)"Unknown format: ");
								errMessage->Append(sb.ToString());
								return false;
							}
						}

						obj = MemAlloc(ObjectInfo, 1);
						obj->objectName = Text::StrCopyNewC(sb.ToString(), i);
						while (sb.ToString()[i] == ' ' || sb.ToString()[i] == '\t')
						{
							i++;
						}
						obj->typeName = Text::StrCopyNew(sb.ToString() + i);
						obj->typeVal = 0;
						obj->oidLen = 0;
						NEW_CLASS(obj->valName, Data::ArrayList<const UTF8Char*>());
						NEW_CLASS(obj->valCont, Data::ArrayList<const UTF8Char*>());
						i = module->objKeys->SortedInsert(obj->objectName);
						module->objValues->Insert(i, obj);
						i = this->globalModule.objKeys->SortedInsert(obj->objectName);
						this->globalModule.objValues->Insert(i, obj);
						if (Text::StrEquals(obj->objectName, (const UTF8Char*)DEBUGOBJ))
						{
							currObj = obj;
						}

						currObj = obj;
						objLineSpace = lineSpace;
						objIsEqual = false;
						sbObjValName.ClearStr();
						sbObjValCont.ClearStr();
					}
				}
			}
		}
	}
}

Net::SNMPMIB::SNMPMIB()
{
	NEW_CLASS(this->moduleMap, Data::StringUTF8Map<ModuleInfo *>());
	NEW_CLASS(this->globalModule.objKeys, Data::ArrayListStrUTF8());
	NEW_CLASS(this->globalModule.objValues, Data::ArrayList<ObjectInfo*>());
	NEW_CLASS(this->globalModule.oidList, Data::ArrayList<ObjectInfo*>());
}

Net::SNMPMIB::~SNMPMIB()
{
	this->UnloadAll();
	DEL_CLASS(this->moduleMap);
	DEL_CLASS(this->globalModule.objKeys);
	DEL_CLASS(this->globalModule.objValues);
	DEL_CLASS(this->globalModule.oidList);
}

Net::SNMPMIB::ModuleInfo *Net::SNMPMIB::GetGlobalModule()
{
	return &this->globalModule;	
}

Net::SNMPMIB::ModuleInfo *Net::SNMPMIB::GetModuleByFileName(const UTF8Char *fileName)
{
	Data::ArrayList<ModuleInfo*> *moduleList = this->moduleMap->GetValues();
	ModuleInfo *module;
	OSInt i = moduleList->GetCount();
	while (i-- > 0)
	{
		module = moduleList->GetItem(i);
		if (module->moduleFileName && Text::StrEquals(module->moduleFileName, fileName))
			return module;
	}
	return 0;
}

void Net::SNMPMIB::UnloadAll()
{
	Data::ArrayList<ModuleInfo*> *moduleList = this->moduleMap->GetValues();
	Data::ArrayList<ObjectInfo*> *objList;
	ObjectInfo *obj;
	ModuleInfo *module;
	OSInt i = moduleList->GetCount();
	OSInt j;
	OSInt k;
	while (i-- > 0)
	{
		module = moduleList->GetItem(i);
		objList = module->objValues;
		j = objList->GetCount();
		while (j-- > 0)
		{
			obj = objList->GetItem(j);
			Text::StrDelNew(obj->objectName);
			SDEL_TEXT(obj->typeName);
			SDEL_TEXT(obj->typeVal);
			k = obj->valName->GetCount();
			while (k-- > 0)
			{
				Text::StrDelNew(obj->valName->GetItem(k));
				Text::StrDelNew(obj->valCont->GetItem(k));
			}
			DEL_CLASS(obj->valName);
			DEL_CLASS(obj->valCont);
			MemFree(obj);
		}

		DEL_CLASS(module->objKeys);
		DEL_CLASS(module->objValues);
		SDEL_TEXT(module->moduleName);
		SDEL_TEXT(module->moduleFileName);
		DEL_CLASS(module->oidList);
		MemFree(module);
	}
	this->moduleMap->Clear();

	this->globalModule.oidList->Clear();
	this->globalModule.objKeys->Clear();
	this->globalModule.objValues->Clear();
}

Bool Net::SNMPMIB::LoadFile(const UTF8Char *fileName, Text::StringBuilderUTF *errMessage)
{
	Text::StringBuilderUTF8 sb;
	IO::FileStream *fs;
	Text::UTF8Reader *reader;
	OSInt i;
	ModuleInfo *module;
	Bool moduleFound = false;
	Bool succ;
	NEW_CLASS(fs, IO::FileStream(fileName, IO::FileStream::FILE_MODE_READONLY, IO::FileStream::FILE_SHARE_DENY_NONE, IO::FileStream::BT_NORMAL));
	if (fs->IsError())
	{
		DEL_CLASS(fs);
		errMessage->Append((const UTF8Char*)"Error in opening file ");
		errMessage->Append(fileName);
		return false;
	}
	succ = false;
	NEW_CLASS(reader, Text::UTF8Reader(fs));
	while (true)
	{
		sb.ClearStr();
		if (!reader->ReadLine(&sb, 512))
		{
			if (!moduleFound)
			{
				errMessage->Append((const UTF8Char*)"Module definition not found");
			}
			break;
		}

		i = sb.IndexOf((const UTF8Char*)"--");
		if (i >= 0)
		{
			sb.RemoveChars(sb.GetLength() - i);
		}
		sb.Trim();
		if (sb.GetLength() > 0)
		{
			if (moduleFound)
			{
				errMessage->Append((const UTF8Char*)"Object found after Module definition");
				succ = false;
				break;
			}
			i = sb.IndexOf((const UTF8Char*)" DEFINITIONS ::= BEGIN");
			if (i < 0)
			{
				succ = false;
				errMessage->Append((const UTF8Char*)"Wrong Module definition format");
				break;
			}
			sb.ToString()[i] = 0;
			if (this->moduleMap->Get(sb.ToString()))
			{
				errMessage->Append((const UTF8Char*)"Module ");
				errMessage->Append(sb.ToString());
				errMessage->Append((const UTF8Char*)" already loaded");
				break;
			}
			module = MemAlloc(ModuleInfo, 1);
			module->moduleName = Text::StrCopyNew(sb.ToString());
			module->moduleFileName = Text::StrCopyNew(fileName);
			NEW_CLASS(module->objKeys, Data::ArrayListStrUTF8());
			NEW_CLASS(module->objValues, Data::ArrayList<ObjectInfo*>());
			NEW_CLASS(module->oidList, Data::ArrayList<ObjectInfo*>());
			this->moduleMap->Put(module->moduleName, module);
			succ = ParseModule(reader, module, errMessage);
			moduleFound = true;
			if (!succ)
			{
				break;
			}
		}

	}
	DEL_CLASS(reader);
	DEL_CLASS(fs);
	return succ;
}