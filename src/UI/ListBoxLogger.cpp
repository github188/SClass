#include "Stdafx.h"
#include "MyMemory.h"
#include "Text/MyString.h"
#include "Text/StringBuilderUTF8.h"
#include "UI/ListBoxLogger.h"

void __stdcall UI::ListBoxLogger::TimerTick(void *userObj)
{
	UI::ListBoxLogger *me = (UI::ListBoxLogger*)userObj;

	if (me->logCnt > 0)
	{
		OSInt curr;
		OSInt cnt;
		OSInt i;
		me->mut->Lock();
		cnt = me->logCnt;
		curr = me->logIndex - cnt;
		if (curr < 0)
		{
			curr += me->maxLog;
		}
		i = 0;
		while (i < cnt)
		{
			me->tmpLogArr[i] = me->logArr[curr];
			me->logArr[curr] = 0;
			i++;
			curr = (curr + 1) % me->maxLog;
		}
		me->logCnt = 0;
		me->mut->Unlock();

//		this->lb->BeginUpdate();
		if (me->reverse)
		{
			if (cnt >= me->maxLog)
			{
				me->lb->ClearItems();
			}
			else
			{
				OSInt lbCnt = me->lb->GetCount();
				while (lbCnt + cnt > me->maxLog)
				{
					lbCnt--;
					me->lb->RemoveItem(lbCnt);
				}
			}
			
			i = 0;
			while (cnt-- > 0)
			{
				me->lb->InsertItem(i, me->tmpLogArr[cnt], 0);
				Text::StrDelNew(me->tmpLogArr[cnt]);
				i++;
			}
		}
		else
		{
			if (cnt >= me->maxLog)
			{
				me->lb->ClearItems();
			}
			else
			{
				OSInt lbCnt = me->lb->GetCount();
				while (lbCnt + cnt > me->maxLog)
				{
					me->lb->RemoveItem(0);
					lbCnt--;
				}
			}
			i = 0;
			while (i < cnt)
			{
				me->lb->AddItem(me->tmpLogArr[i], 0);
				Text::StrDelNew(me->tmpLogArr[i]);
				i++;
			}
		}
//		this->lb->EndUpdate();
	}
}

UI::ListBoxLogger::ListBoxLogger(UI::GUIForm *frm, UI::GUIListBox *lb, Int32 maxLog, Bool reverse)
{
	OSInt i;
	this->lb = lb;
	this->maxLog = maxLog;
	this->reverse = reverse;
	this->frm = frm;
	this->logArr = MemAlloc(const UTF8Char *, this->maxLog);
	this->tmpLogArr = MemAlloc(const UTF8Char *, this->maxLog);
	this->logIndex = 0;
	this->logCnt = 0;
	this->timeFormat = 0;
	i = this->maxLog;
	while (i-- > 0)
	{
		this->logArr[i] = 0;
	}
	NEW_CLASS(this->mut, Sync::Mutex());
	this->tmr = frm->AddTimer(500, TimerTick, this);
}

UI::ListBoxLogger::~ListBoxLogger()
{
	this->frm->RemoveTimer(this->tmr);

	OSInt i = this->maxLog;
	while (i-- > 0)
	{
		if (this->logArr[i])
		{
			Text::StrDelNew(this->logArr[i]);
		}
	}
	MemFree(this->logArr);
	MemFree(this->tmpLogArr);
	SDEL_TEXT(this->timeFormat);
	DEL_CLASS(this->mut);
	
}

void UI::ListBoxLogger::LogClosed()
{
}

void UI::ListBoxLogger::LogAdded(Data::DateTime *logTime, const UTF8Char *logMsg, IO::ILogHandler::LogLevel logLev)
{
	Text::StringBuilderUTF8 sb;
	UTF8Char sbuff[64];
	this->mut->Lock();
	if (this->timeFormat)
	{
		logTime->ToString(sbuff, this->timeFormat);
		sb.Append(sbuff);
	}
	else
	{
		sb.AppendDate(logTime);
	}
	sb.AppendChar('\t', 1);
	sb.Append(logMsg);
	if (this->logCnt < this->maxLog)
	{
		this->logCnt++;
	}
	if (this->logArr[this->logIndex])
	{
		Text::StrDelNew(this->logArr[this->logIndex]);
	}
	this->logArr[this->logIndex] = Text::StrCopyNew(sb.ToString());
	this->logIndex = (this->logIndex + 1) % this->maxLog;
	this->mut->Unlock();
}

void UI::ListBoxLogger::SetTimeFormat(const Char *timeFormat)
{
	this->mut->Lock();
	SDEL_TEXT(this->timeFormat);
	this->timeFormat = Text::StrCopyNew(timeFormat);
	this->mut->Unlock();
}