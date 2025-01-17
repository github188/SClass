#include "Stdafx.h"
#include "MyMemory.h"
#include "Core/Core.h"
#include "IO/ConsoleWriter.h"
#include "Manage/HiResClock.h"
#include "Math/Math.h"
#include "Text/MyString.h"
#include "Text/MyStringFloat.h"
#include "Text/StringBuilderUTF8.h"

Int32 MyMain(Core::IProgControl *progCtrl)
{
	UTF8Char sbuff2[256];
	Manage::HiResClock *clk;
	OSInt i;
	OSInt j;
	Text::StringBuilderUTF8 *sb;

	NEW_CLASS(clk, Manage::HiResClock());
	clk->Start();
	i = 10000;
	while (i-- > 0)
	{
		NEW_CLASS(sb, Text::StringBuilderUTF8());
		j = 1000001000;
		while (j-- > 1000000000)
		{
			sb->Append((const UTF8Char*)"1234567890");
		}
		DEL_CLASS(sb);
	}
	Double t1 = clk->GetTimeDiff();
	IO::ConsoleWriter *console;
	NEW_CLASS(console, IO::ConsoleWriter());
	Text::StrConcat(Text::StrDouble(sbuff2, t1), (const UTF8Char*)" s");
	console->WriteLine(sbuff2);
	DEL_CLASS(console);

	DEL_CLASS(clk);
	return 0;
}
