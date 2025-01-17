#include "Stdafx.h"
#include "MyMemory.h"
#include "Core/Core.h"
#include "IO/ConsoleWriter.h"
#include "Manage/Process.h"
#include "Text/StringBuilderUTF8.h"

Int32 MyMain(Core::IProgControl *progCtrl)
{
	IO::ConsoleWriter console;
	Text::StringBuilderUTF8 sb;
	console.WriteLine((const UTF8Char*)"Start Execute");
	Manage::Process::ExecuteProcess((const UTF8Char*)"getprop", &sb);
	console.WriteLine((const UTF8Char*)"End Execute");
	console.WriteLine(sb.ToString());
	return 0;
}
