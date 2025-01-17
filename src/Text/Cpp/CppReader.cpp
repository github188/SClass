#include "Stdafx.h"
#include "Text/CharUtil.h"
#include "Text/Cpp/CppReader.h"

Bool Text::Cpp::CppReader::ReadLineInner(Text::StringBuilderUTF8 *sb)
{
	UOSInt initSize = sb->GetLength();
	if (!this->reader->ReadLine(sb, 512))
	{
		return false;
	}

	UOSInt i;
	UOSInt j;
	UOSInt k;
	if (this->escapeType == ET_MULTILINE_COMMENT)
	{
		i = sb->IndexOf((const UTF8Char*)"*/", initSize);
		if (i == INVALID_INDEX)
		{
			sb->TrimToLength(initSize);
			return true;
		}
		sb->RemoveChars(initSize, (UOSInt)i + 2 - initSize);
		this->escapeType = ET_NONE;
	}
	else if (this->escapeType == ET_STRING)
	{
		i = sb->IndexOf((const UTF8Char*)"\"", initSize);
		if (i == INVALID_INDEX)
		{
			return true;
		}
		initSize = i + 1;
		this->escapeType = ET_NONE;
	}
	while (true)
	{
		j = sb->IndexOf((const UTF8Char*)"/*", initSize);
		k = sb->IndexOf((const UTF8Char*)"\"", initSize);
		if (j == INVALID_INDEX && k == INVALID_INDEX)
		{
			break;
		}

		if (j != INVALID_INDEX && (k == INVALID_INDEX || k > j))
		{
			i = sb->IndexOf((const UTF8Char*)"*/", j + 2);
			if (i != INVALID_INDEX)
			{
				sb->RemoveChars(j, (i - j + 2));
			}
			else
			{
				sb->TrimToLength(j);
				this->escapeType = ET_MULTILINE_COMMENT;
				break;
			}
		}
		else
		{
			i = sb->IndexOf((const UTF8Char*)"\"", k + 1);
			if (i != INVALID_INDEX)
			{
				initSize = i + 1;
			}
			else
			{
				this->escapeType = ET_STRING;
				break;
			}
		}
	}
	sb->TrimRight();
	return true;
}

Bool Text::Cpp::CppReader::ReadWord(Text::StringBuilderUTF *sb, Bool move)
{
	while (this->currOfst >= this->sbLine->GetCharCnt())
	{
		this->sbLine->ClearStr();
		if (!ReadLineInner(this->sbLine))
		{
			return false;
		}
		this->sbLine->Trim();
		this->currOfst = 0;
	}
	UTF8Char *sptr = this->sbLine->ToString();
	while (Text::CharUtil::IsWS(&sptr[this->currOfst]))
	{
		this->currOfst++;
	}

	if ((sptr[this->currOfst] == '>' && sptr[this->currOfst + 1] == '>') ||
		(sptr[this->currOfst] == '<' && sptr[this->currOfst + 1] == '<'))
	{
		if (sptr[this->currOfst + 2] == '=')
		{
			sb->AppendC(&sptr[this->currOfst], 3);
			if (move)
				this->currOfst += 3;
		}
		else
		{
			sb->AppendC(&sptr[this->currOfst], 2);
			if (move)
				this->currOfst += 2;
		}
		return true;
	}
	else if ((sptr[this->currOfst] == '&' && sptr[this->currOfst + 1] == '&') ||
		(sptr[this->currOfst] == '|' && sptr[this->currOfst + 1] == '|'))
	{
		sb->AppendC(&sptr[this->currOfst], 2);
		if (move)
			this->currOfst += 2;
		return true;
	}
	else if (Text::StrIndexOf((const UTF8Char*)"=><+*-/^!!%", sptr[this->currOfst]) != INVALID_INDEX)
	{
		if (sptr[this->currOfst + 1] == '=')
		{
			sb->AppendC(&sptr[this->currOfst], 2);
			if (move)
				this->currOfst += 2;
			return true;
		}
		else
		{
			sb->AppendChar(sptr[this->currOfst], 1);
			if (move)
				this->currOfst += 1;
			return true;
		}
	}
	else if (Text::StrIndexOf((const UTF8Char*)"{}[](),;", sptr[this->currOfst]) != INVALID_INDEX)
	{
		sb->AppendChar(sptr[this->currOfst], 1);
		if (move)
			this->currOfst += 1;
		return true;
	}
	else if (Text::CharUtil::IsAlphaNumeric(sptr[this->currOfst]))
	{
		UOSInt i = this->currOfst;
		while (Text::CharUtil::IsAlphaNumeric(sptr[i]) || sptr[i] == '-' || sptr[i] == '_')
		{
			i++;
		}
		sb->AppendC(&sptr[this->currOfst], i - this->currOfst);
		if (move)
		{
			this->currOfst = i;
		}
		return true;
	}
	else if (sptr[this->currOfst] == '"')
	{
		UOSInt j = this->currOfst + 1;
		while (true)
		{
			if (sptr[j] == 0 || (sptr[j] == '\\' && sptr[j + 1] == 0))
			{
				reader->GetLastLineBreak(this->sbLine);
				if (!ReadLineInner(this->sbLine))
				{
					return false;
				}
				sptr = this->sbLine->ToString();
			}
			else if (sptr[j] == '\\')
			{
				j += 2;
			}
			else if (sptr[j] == '\"')
			{
				j++;
				break;
			}
			else
			{
				j++;
			}
		}
		sb->AppendC(&sptr[this->currOfst], j - this->currOfst);
		this->currOfst = j;
		return true;
	}
	else
	{
		return false;
	}
}

Text::Cpp::CppReader::CppReader(IO::Stream *stm)
{
	NEW_CLASS(this->reader, Text::UTF8Reader(stm));
	NEW_CLASS(this->sbLine, Text::StringBuilderUTF8());
	this->currOfst = 0;
	this->escapeType = ET_NONE;
}

Text::Cpp::CppReader::~CppReader()
{
	DEL_CLASS(this->sbLine);
	DEL_CLASS(this->reader);
}

Bool Text::Cpp::CppReader::PeekWord(Text::StringBuilderUTF *sb)
{
	return ReadWord(sb, false);
}

Bool Text::Cpp::CppReader::NextWord(Text::StringBuilderUTF *sb)
{
	return ReadWord(sb, true);
}

Bool Text::Cpp::CppReader::ReadLine(Text::StringBuilderUTF8 *sb)
{
	if (this->currOfst >= this->sbLine->GetCharCnt())
	{
		return ReadLineInner(sb);
	}
	else
	{
		sb->Append(this->sbLine->ToString() + this->currOfst);
		this->currOfst = this->sbLine->GetCharCnt();
		return true;
	}
}

Bool Text::Cpp::CppReader::GetLastLineBreak(Text::StringBuilderUTF *sb)
{
	return this->reader->GetLastLineBreak(sb);
}
