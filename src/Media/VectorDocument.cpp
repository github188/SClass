#include "Stdafx.h"
#include "MyMemory.h"
#include "Media/VectorDocument.h"
#include "Text/MyString.h"

Media::VectorDocument::VectorDocument(UInt32 srid, Media::DrawEngine *refEng) : IO::ParsedObject((const UTF8Char*)"Untitled")
{
	NEW_CLASS(this->items, Data::ArrayList<Media::VectorGraph*>());
	this->currDoc = 0;
	this->srid = srid;
	this->refEng = refEng;
	this->docName = 0;
	this->author = 0;
	this->subject = 0;
	this->keywords = 0;
	this->creator = 0;
	this->producer = 0;
	this->createTimeTicks = 0;
	this->modTimeTicks = 0;
}

Media::VectorDocument::VectorDocument(UInt32 srid, const UTF8Char *name, Media::DrawEngine *refEng) : IO::ParsedObject(name)
{
	NEW_CLASS(this->items, Data::ArrayList<Media::VectorGraph*>());
	this->currDoc = 0;
	this->srid = srid;
	this->refEng = refEng;
	this->docName = Text::StrCopyNew(name);
	this->author = 0;
	this->subject = 0;
	this->keywords = 0;
	this->creator = 0;
	this->producer = 0;
	this->createTimeTicks = 0;
	this->modTimeTicks = 0;
}

Media::VectorDocument::~VectorDocument()
{
	UOSInt i;
	Media::VectorGraph *graph;
	i = this->items->GetCount();
	while (i-- > 0)
	{
		graph = this->items->GetItem(i);
		DEL_CLASS(graph);
	}
	DEL_CLASS(this->items);
	SDEL_TEXT(this->docName);
	SDEL_TEXT(this->author);
	SDEL_TEXT(this->subject);
	SDEL_TEXT(this->keywords);
	SDEL_TEXT(this->creator);
	SDEL_TEXT(this->producer);
}

Media::VectorGraph *Media::VectorDocument::AddGraph(Double width, Double height, Math::Unit::Distance::DistanceUnit unit)
{
	Media::VectorGraph *graph;
	Media::ColorProfile color(Media::ColorProfile::CPT_SRGB);
	NEW_CLASS(graph, Media::VectorGraph(this->srid, width, height, unit, this->refEng, &color));
	this->items->Add(graph);
	return graph;
}

const UTF8Char *Media::VectorDocument::GetDocName()
{
	return this->docName;
}

void Media::VectorDocument::SetCreateTime(Int64 createTimeTicks)
{
	this->createTimeTicks = createTimeTicks;
}

Int64 Media::VectorDocument::GetCreateTime()
{
	return this->createTimeTicks;
}

void Media::VectorDocument::SetModifyTime(Int64 modTimeTicks)
{
	this->modTimeTicks = modTimeTicks;
}

Int64 Media::VectorDocument::GetModifyTime()
{
	return this->modTimeTicks;
}

void Media::VectorDocument::SetAuthor(const UTF8Char *author)
{
	SDEL_TEXT(this->author);
	if (author)
	{
		this->author = Text::StrCopyNew(author);
	}
}

const UTF8Char *Media::VectorDocument::GetAuthor()
{
	return this->author;
}

void Media::VectorDocument::SetSubject(const UTF8Char *subject)
{
	SDEL_TEXT(this->subject);
	if (subject)
	{
		this->subject = Text::StrCopyNew(subject);
	}
}

const UTF8Char *Media::VectorDocument::GetSubject()
{
	return this->subject;
}

void Media::VectorDocument::SetKeywords(const UTF8Char *keywords)
{
	SDEL_TEXT(this->keywords);
	if (keywords)
	{
		this->keywords = Text::StrCopyNew(keywords);
	}
}

const UTF8Char *Media::VectorDocument::GetKeywords()
{
	return this->keywords;
}

void Media::VectorDocument::SetCreator(const UTF8Char *creator)
{
	SDEL_TEXT(this->creator);
	if (creator)
	{
		this->creator = Text::StrCopyNew(creator);
	}
}

const UTF8Char *Media::VectorDocument::GetCreator()
{
	return this->creator;
}

void Media::VectorDocument::SetProducer(const UTF8Char *producer)
{
	SDEL_TEXT(this->producer);
	if (producer)
	{
		this->producer = Text::StrCopyNew(producer);
	}
}

const UTF8Char *Media::VectorDocument::GetProducer()
{
	return this->producer;
}

UOSInt Media::VectorDocument::GetCount()
{
	return this->items->GetCount();
}

Media::VectorGraph *Media::VectorDocument::GetItem(UOSInt Index)
{
	return this->items->GetItem(Index);
}

Bool Media::VectorDocument::BeginPrint(IPrintDocument *doc)
{
	Media::VectorGraph *graph;
	Double width;
	Double height;
	this->currGraph = 0;
	this->currDoc = doc;
	graph = this->items->GetItem(this->currGraph);
	if (graph == 0)
		return false;
	if (this->docName)
		doc->SetDocName(this->docName);
	width = graph->GetVisibleWidthMM();
	height = graph->GetVisibleHeightMM();
	if (width > height)
	{
		doc->SetNextPagePaperSizeMM(height, width);
		doc->SetNextPageOrientation(Media::IPrintDocument::PO_LANDSCAPE);
	}
	else
	{
		doc->SetNextPagePaperSizeMM(width, height);
		doc->SetNextPageOrientation(Media::IPrintDocument::PO_PORTRAIT);
	}
	return true;
}

Bool Media::VectorDocument::PrintPage(Media::DrawImage *printPage)
{
	Media::VectorGraph *graph;
	Double width;
	Double height;
	graph = this->items->GetItem(this->currGraph);
	if (this->currDoc == 0 || graph == 0)
		return false;
	graph->DrawTo(printPage, 0);
	this->currGraph++;
	graph = this->items->GetItem(this->currGraph);
	if (graph == 0)
		return false;
	width = graph->GetVisibleWidthMM();
	height = graph->GetVisibleHeightMM();
	if (width > height)
	{
		this->currDoc->SetNextPagePaperSizeMM(height, width);
		this->currDoc->SetNextPageOrientation(Media::IPrintDocument::PO_LANDSCAPE);
	}
	else
	{
		this->currDoc->SetNextPagePaperSizeMM(width, height);
		this->currDoc->SetNextPageOrientation(Media::IPrintDocument::PO_PORTRAIT);
	}
	return true;
}

Bool Media::VectorDocument::EndPrint(IPrintDocument *doc)
{
	this->currDoc = 0;
	return true;
}

IO::ParsedObject::ParserType Media::VectorDocument::GetParserType()
{
	return IO::ParsedObject::PT_VECTOR_DOCUMENT;
}
