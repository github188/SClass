#include "Stdafx.h"
#include "DB/SDFFile.h"
#include "Text/StringBuilderW.h"

DB::SDFFile::SDFFile(const UTF8Char *fileName, Version ver, IO::LogTool *log, const UTF8Char *password) : DB::OLEDBConn(log)
{
	Text::StringBuilderW sb;
	switch (ver)
	{
	case VER_30:
		sb.Append((const UTF8Char*)"Provider=Microsoft.SQLSERVER.CE.OLEDB.3.0;Data Source=");
		break;
	case VER_35:
		sb.Append((const UTF8Char*)"Provider=Microsoft.SQLSERVER.CE.OLEDB.3.5;Data Source=");
		break;
	case VER_40:
		sb.Append((const UTF8Char*)"Provider=Microsoft.SQLSERVER.CE.OLEDB.4.0;Data Source=");
		break;
	}
	sb.Append(fileName);
	if (password)
	{
		sb.Append((const UTF8Char*)";Password=");
		sb.Append(password);
	}
	this->Init(sb.ToString());
}
