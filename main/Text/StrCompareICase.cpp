#include "Stdafx.h"
#include "MyMemory.h"
#include "Core/Core.h"
#include "IO/ConsoleWriter.h"
#include "Manage/HiResClock.h"
#include "Text/MyString.h"
#include "Text/MyStringFloat.h"
#include "Text/StringBuilderUTF8.h"

Int32 MyMain(Core::IProgControl *progCtrl)
{
	Double spd1;
	Double spd2;
	OSInt i;
	OSInt val;
	OSInt val2;
	Manage::HiResClock *clk;
	Text::StringBuilderUTF8 sb;
	IO::ConsoleWriter console;

	NEW_CLASS(clk, Manage::HiResClock());
	i = 10000000;
	clk->Start();
	while (i-- > 0)
	{
		val = Text::StrCompareICase("aBcDe1FgHiJk", "AbCdE1fGhIjK");
	}
	spd1 = clk->GetTimeDiff();
	sb.ClearStr();
	sb.Append((const UTF8Char*)"Compare value1 = ");
	sb.AppendOSInt(val);
	sb.Append((const UTF8Char*)", t = ");
	Text::SBAppendF64(&sb, spd1);
	console.WriteLine(sb.ToString());

	i = 10000000;
	clk->Start();
	while (i-- > 0)
	{
		val2 = Text::StrCompare("aBcDe1FgHiJk", "AbCdE1fGhIjK");
	}
	spd2 = clk->GetTimeDiff();
	sb.ClearStr();
	sb.Append((const UTF8Char*)"Compare value2 = ");
	sb.AppendOSInt(val2);
	sb.Append((const UTF8Char*)", t = ");
	Text::SBAppendF64(&sb, spd2);
	console.WriteLine(sb.ToString());

	DEL_CLASS(clk);

	return 0;
}