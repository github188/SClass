#include "Stdafx.h"
#include "Manage/ProcessExecution.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct
{
	int out[2];
	int in[2];
} ClassData;

UOSInt Manage::ProcessExecution::NewProcess(const UTF8Char *cmdLine)
{
	ClassData *clsData = MemAlloc(ClassData, 1);
	clsData->in[0] = 0;
	clsData->in[1] = 0;
	clsData->out[0] = 0;
	clsData->out[1] = 0;
	this->clsData = clsData;
	UTF8Char progName[64];
	UTF8Char *progBuff = 0;
	const UTF8Char *cptr = cmdLine;
	Data::ArrayList<UTF8Char *> args;
	Bool argStart = false;

	UOSInt cmdLen = Text::StrCharCnt(cmdLine);
	UTF8Char *pptr;
	if (cmdLen >= 64)
	{
		progBuff = MemAlloc(UTF8Char, cmdLen + 1);
		pptr = progBuff;
	}
	else
	{
		pptr = progName;
	}
	Bool isQuote = false;
	UTF8Char c;
	args.Add(pptr);
	while ((c = *cptr++) != 0)
	{
		if (c == '"')
			isQuote = !isQuote;
		else if (!isQuote && c == ' ')
		{
			if (!argStart)
			{
				*pptr++ = 0;
				argStart = true;
			}
		}
		else
		{
			if (argStart)
			{
				args.Add(pptr);
				argStart = false;
			}
			*pptr++ = c;
		}
	}
	*pptr = 0;
	args.Add(0);
	UOSInt argc;
	UTF8Char **arr = args.GetArray(&argc);

	if (pipe(clsData->in) != 0)
	{
		printf("ProcessExecution: Error in creating in pipes\r\n");
		return 0;
	}
	if (pipe(clsData->out) != 0)
	{
		printf("ProcessExecution: Error in creating out pipes\r\n");
		close(clsData->in[0]);
		close(clsData->in[1]);
		return 0;
	}
	pid_t pid = fork();
	int ret = -1;
	if (pid == 0)
	{
		dup2(clsData->in[0], STDIN_FILENO);
		dup2(clsData->out[1], STDOUT_FILENO);
		ret = execvp((Char*)arr[0], (Char**)arr);
		exit(ret);
	}
	return (UOSInt)pid;
}

Manage::ProcessExecution::ProcessExecution(const UTF8Char *cmdLine) : Process(NewProcess(cmdLine), false), IO::Stream(cmdLine)
{
}

Manage::ProcessExecution::~ProcessExecution()
{
	ClassData *clsData = (ClassData*)this->clsData;
	this->Close();
	MemFree(clsData);
}

UOSInt Manage::ProcessExecution::Read(UInt8 *buff, UOSInt size)
{
	ClassData *clsData = (ClassData*)this->clsData;
	if (clsData->in[0] == 0)
		return 0;
	OSInt readSize = read(clsData->out[0], buff, size);
	if (readSize >= 0)
	{
		return (UOSInt)readSize;
	}
	else
	{
		return 0;
	}
}

UOSInt Manage::ProcessExecution::Write(const UInt8 *buff, UOSInt size)
{
	ClassData *clsData = (ClassData*)this->clsData;
	if (clsData->in[0] == 0)
		return 0;
	OSInt readSize = write(clsData->in[1], buff, size);
	if (readSize >= 0)
	{
		return (UOSInt)readSize;
	}
	else
	{
		return 0;
	}
}

Int32 Manage::ProcessExecution::Flush()
{
	return 0;
}

void Manage::ProcessExecution::Close()
{
	ClassData *clsData = (ClassData*)this->clsData;
	if (this->IsRunning())
	{
		this->Kill();
	}
	if (clsData->in[0] != 0)
	{
		close(clsData->in[0]);
		close(clsData->in[1]);
		close(clsData->out[0]);
		close(clsData->out[1]);
		clsData->in[0] = 0;
		clsData->in[1] = 0;
		clsData->out[0] = 0;
		clsData->out[1] = 0;
	}
}

Bool Manage::ProcessExecution::Recover()
{
	return false;
}
