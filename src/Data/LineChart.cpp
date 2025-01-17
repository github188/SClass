#include "Stdafx.h"
#include "MyMemory.h"
#include "Data/IChart.h"
#include "Data/LineChart.h"
#include "Text/MyString.h"
#include "Text/MyStringFloat.h"

#define DRAW_UNIT Double

Data::LineChart::LineChart(const UTF8Char *title)
{
	if (title == 0)
	{
		this->title = 0;
		this->titleLineCnt = 0;
	}
	else
	{
		this->title = Text::StrCopyNew(title);
		this->titleLineCnt = Text::StrSplitLine(this->titleLine, 3, (UTF8Char*)this->title);
	}
	this->xLabel = 0;
	this->yLabel = 0;
	this->xType = Data::LineChart::CT_NO;
	this->refTime = 0;
	this->timeZoneQHR = 0;
	this->barLength = 3.0;
	this->pointType = PT_NULL;
	this->pointSize = 0;
	this->yUnit = 0;
	Data::DateTime dt;
	dt.ToLocalTime();
	this->timeZoneQHR = dt.GetTimeZoneQHR();
	NEW_CLASS(this->rnd, Data::RandomOS());
	NEW_CLASS(xDatas, Data::ArrayList<void*>());
	NEW_CLASS(xDataCnt, Data::ArrayList<UOSInt>());
	NEW_CLASS(yCharts, Data::ArrayList<ChartData*>());
	NEW_CLASS(dataNames, Data::ArrayList<const UTF8Char*>());

	bgColor = 0xffffffff;
	boundColor = 0xff000000;
	fontColor = 0xff000000;
	gridColor = 0xffebebeb;
	refLineColor = 0xffff0000;
	this->lineThick = 1.0;
    
	fntName = Text::StrCopyNew((const UTF8Char*)"SimHei");
	fntSizePt = 12.0;
	timeFormat = Text::StrCopyNew("HH:mm");
	dateFormat = Text::StrCopyNew("yyyy/MM/dd");
	dblFormat = Text::StrCopyNew("0.00");
	minDblVal = 0.01;
	
	this->refDbl = 0;
	this->refInt = 0;
	this->refTime = 0;
	this->refExist = false;

	this->xRangeDateMin = 0;
	this->xRangeDateMax = 0;

	this->hasXRangeDate = false;
	this->hasYRangeDbl = false;
	this->hasYRangeInt = false;
}

Data::LineChart::~LineChart()
{
	UOSInt i;

	i = this->xDatas->GetCount();
	while (i-- > 0)
	{
		MemFree(this->xDatas->GetItem(i));
	}
	DEL_CLASS(this->xDatas);
	this->xDatas = 0;
	DEL_CLASS(this->xDataCnt);
	this->xDataCnt = 0;

	i = this->yCharts->GetCount();
	while (i-- > 0)
	{
		Data::LineChart::ChartData *data = (Data::LineChart::ChartData*)this->yCharts->RemoveAt(i);
		DEL_CLASS(data);
	}
	DEL_CLASS(this->yCharts);
	this->yCharts = 0;

	i = this->dataNames->GetCount();
	while (i-- > 0)
	{
		Text::StrDelNew(this->dataNames->RemoveAt(i));
	}
	DEL_CLASS(this->dataNames);
	DEL_CLASS(this->rnd);
	this->dataNames = 0;

	this->refTime = 0;
	this->xRangeDateMax = 0;
	this->xRangeDateMin = 0;

	SDEL_TEXT(this->yUnit);
	SDEL_TEXT(this->xLabel);
	SDEL_TEXT(this->yLabel);
	SDEL_TEXT(this->title);
	SDEL_TEXT(this->fntName);
	SDEL_TEXT(this->dateFormat);
	SDEL_TEXT(this->timeFormat);
	SDEL_TEXT(this->dblFormat);
}

void Data::LineChart::SetTitle(const UTF8Char *title)
{
	if (this->title)
	{
		Text::StrDelNew(this->title);
	}
	if (title == 0)
	{
		this->title = 0;
		this->titleLineCnt = 0;
	}
	else
	{
		this->title = Text::StrCopyNew(title);
		this->titleLineCnt = Text::StrSplitLine(this->titleLine, 3, (UTF8Char*)this->title);
	}
}

Bool Data::LineChart::AddXData(Data::DateTime **data, UOSInt dataCnt)
{
	if (xType == Data::LineChart::CT_NO)
	{
		Int64 *dateData = MemAlloc(Int64, dataCnt);
		UOSInt i = dataCnt;
		while (i-- > 0)
		{
			dateData[i] = data[i]->ToTicks();
		}
		
		xType = Data::LineChart::CT_DATETICK;
		xDatas->Add(dateData);
		xDataCnt->Add(dataCnt);
		return true;
	}
	else if (xType == Data::LineChart::CT_DATETICK)
	{
		Int64 *dateData = MemAlloc(Int64, dataCnt);
		UOSInt i = dataCnt;
		while (i-- > 0)
		{
			dateData[i] = data[i]->ToTicks();
		}

		xDatas->Add(dateData);
		xDataCnt->Add(dataCnt);
		return true;
	}
	else
	{
		return false;
	}
}

Bool Data::LineChart::AddXData(Double *data, UOSInt dataCnt)
{
	if (xType == Data::LineChart::CT_NO)
	{
		Double *dblData = MemAlloc(Double, dataCnt);
		MemCopyNO(dblData, data, sizeof(Double) * dataCnt);
		xType = Data::LineChart::CT_DOUBLE;
		xDatas->Add(dblData);
		xDataCnt->Add(dataCnt);
		return true;
	}
	else if (xType == Data::LineChart::CT_DOUBLE)
	{
		Double *dblData = MemAlloc(Double, dataCnt);
		MemCopyNO(dblData, data, sizeof(Double) * dataCnt);
		xDatas->Add(dblData);
		xDataCnt->Add(dataCnt);
		return true;
	}
	else
	{
		return false;
	}
}

Bool Data::LineChart::AddXData(Int32 *data, UOSInt dataCnt)
{
	if (xType == Data::LineChart::CT_NO)
	{
		Int32 *iData = MemAlloc(Int32, dataCnt);
		MemCopyNO(iData, data, sizeof(Int32) * dataCnt);
		xType = Data::LineChart::CT_INTEGER;
		xDatas->Add(iData);
		xDataCnt->Add(dataCnt);
		return true;
	}
	else if (xType == Data::LineChart::CT_INTEGER)
	{
		Int32 *iData = MemAlloc(Int32, dataCnt);
		MemCopyNO(iData, data, sizeof(Int32) * dataCnt);
		xDatas->Add(iData);
		xDataCnt->Add(dataCnt);
		return true;
	}
	else
	{
		return false;
	}
}

Bool Data::LineChart::AddXDataDate(Int64 *data, UOSInt dataCnt)
{
	if (xType == Data::LineChart::CT_NO)
	{
		Int64 *newData = MemAlloc(Int64, dataCnt);
		MemCopyNO(newData, data, sizeof(Int64) * dataCnt);
		xType = Data::LineChart::CT_DATETICK;
		xDatas->Add(newData);
		xDataCnt->Add(dataCnt);
		return true;
	}
	else if (xType == Data::LineChart::CT_DATETICK)
	{
		Int64 *newData = MemAlloc(Int64, dataCnt);
		MemCopyNO(newData, data, sizeof(Int64) * dataCnt);
		xDatas->Add(newData);
		xDataCnt->Add(dataCnt);
		return true;
	}
	else
	{
		return false;
	}
}

void Data::LineChart::SetXAxis(const UTF8Char *name)
{
	if (this->xLabel)
	{
		Text::StrDelNew(this->xLabel);
	}
	this->xLabel = Text::StrCopyNew(name);
}

void Data::LineChart::SetDateFormat(const Char *format)
{
	if (this->dateFormat)
	{
		Text::StrDelNew(this->dateFormat);
	}
	this->dateFormat = Text::StrCopyNew(format);
}

void Data::LineChart::SetTimeFormat(const Char *format)
{
	SDEL_TEXT(this->timeFormat);
	if (format)
	{
		this->timeFormat = Text::StrCopyNew(format);
	}
}

void Data::LineChart::SetDblFormat(const Char *format)
{
	if (this->dblFormat)
	{
		Text::StrDelNew(this->dblFormat);
	}
	this->dblFormat = Text::StrCopyNew(format);
	UOSInt i = Text::StrIndexOf(format, ".");
	if (i == INVALID_INDEX)
	{
		this->minDblVal = 1.0;
	}
	else
	{
		i = Text::StrCharCnt(format) - i - 1;
		this->minDblVal = 1.0;
		while (i-- > 0)
		{
			this->minDblVal = this->minDblVal * 0.1;
		}
	}
}

void Data::LineChart::SetFontHeightPt(Double ptSize)
{
	if (ptSize > 0)
		fntSizePt = ptSize;
}

void Data::LineChart::SetFontName(const UTF8Char *name)
{
	if (this->fntName)
	{
		Text::StrDelNew(this->fntName);
	}
	this->fntName = Text::StrCopyNew(name);
}

void Data::LineChart::SetYRefVal(Int32 refVal, UInt32 col)
{
	this->refInt = refVal;
	this->refLineColor = col;
	this->refExist = true;
}

void Data::LineChart::SetYRefVal(Double refVal, UInt32 col)
{
	this->refDbl = refVal;
	this->refLineColor = col;
	this->refExist = true;
}

void Data::LineChart::SetYRefVal(Data::DateTime *refVal, UInt32 col)
{
	this->refTime = refVal->ToTicks();
	this->refLineColor = col;
	this->refExist = true;
}

void Data::LineChart::SetYRefType(Data::LineChart::RefType refType)
{
	this->refType = refType;
}

void Data::LineChart::SetYUnit(const UTF8Char *yUnit)
{
	SDEL_TEXT(this->yUnit);
	if (yUnit)
	{
		this->yUnit = Text::StrCopyNew(yUnit);
	}
}

void Data::LineChart::SetLineThick(Double lineThick)
{
	this->lineThick = lineThick;
}

void Data::LineChart::SetTimeZoneQHR(Int8 timeZoneQHR)
{
	this->timeZoneQHR = timeZoneQHR;
}

void Data::LineChart::SetBarLength(Double barLength)
{
	this->barLength = barLength;
}

void Data::LineChart::SetPointType(PointType pointType, Double pointSize)
{
	this->pointType = pointType;
	this->pointSize = pointSize;
}

UInt32 Data::LineChart::GetRndColor()
{
	UInt32 r;
	UInt32 g;
	UInt32 b;
	
	r = (UInt32)(64 + (this->rnd->NextInt15() % 192));
	g = (UInt32)(64 + (this->rnd->NextInt15() % 192));
	b = 512 - r - g;
	if (b < 0)
		b = 0;
	else if (b > 255)
		b = 255;
	return 0xff000000 | (r << 16) | (g << 8) | b;
}

void Data::LineChart::AddYDataDate(const UTF8Char *name, Int64 *value, UOSInt valCnt, UInt32 lineColor, Data::LineChart::LineStyle lineStyle)
{
	Int64 *newVals;
	newVals = MemAlloc(Int64, valCnt);
	MemCopyNO(newVals, value, sizeof(Int64) * valCnt);
	dataNames->Add(Text::StrCopyNew(name));
	yCharts->Add(new Data::LineChart::ChartData(newVals, valCnt, Data::LineChart::CT_DATETICK, lineColor, lineStyle));
}

void Data::LineChart::AddYData(const UTF8Char *name, Int32 *value, UOSInt valCnt, UInt32 lineColor, Data::LineChart::LineStyle lineStyle)
{
	Int32 *newVals;
	newVals = MemAlloc(Int32, valCnt);
	MemCopyNO(newVals, value, sizeof(Int32) * valCnt);
	dataNames->Add(Text::StrCopyNew(name));
	yCharts->Add(new Data::LineChart::ChartData(newVals, valCnt, Data::LineChart::CT_INTEGER, lineColor, lineStyle));
}

void Data::LineChart::AddYData(const UTF8Char *name, Double *value, UOSInt valCnt, UInt32 lineColor, Data::LineChart::LineStyle lineStyle)
{
	Double *newVals;
	newVals = MemAlloc(Double, valCnt);
	MemCopyNO(newVals, value, sizeof(Double) * valCnt);
	dataNames->Add(Text::StrCopyNew(name));
	yCharts->Add(new Data::LineChart::ChartData(newVals, valCnt, Data::LineChart::CT_DOUBLE, lineColor, lineStyle));
}

void Data::LineChart::SetYAxis(const UTF8Char *name)
{
	if (this->yLabel)
	{
		Text::StrDelNew(this->yLabel);
	}
	this->yLabel = Text::StrCopyNew(name);
}

void Data::LineChart::SetXRangeDate(Data::DateTime *xVal)
{
	if (hasXRangeDate)
	{
		if (this->xRangeDateMin > xVal->ToTicks())
		{
			this->xRangeDateMin = xVal->ToTicks();
		}
		if (this->xRangeDateMax < xVal->ToTicks())
		{
			this->xRangeDateMax = xVal->ToTicks();
		}
	}
	else
	{
		this->xRangeDateMin = xVal->ToTicks();
		this->xRangeDateMax = xVal->ToTicks();
		this->hasXRangeDate = true;
	}
}

void Data::LineChart::SetYRangeInt(Int32 yVal)
{
	if (hasYRangeInt)
	{
		if (yRangeIntMin > yVal)
		{
			yRangeIntMin = yVal;
		}
		if (yRangeIntMax < yVal)
		{
			yRangeIntMax = yVal;
		}
	}
	else
	{
		yRangeIntMin = yVal;
		yRangeIntMax = yVal;
		hasYRangeInt = true;
	}
}

void Data::LineChart::SetYRangeDbl(Double yVal)
{
	if (hasYRangeDbl)
	{
		if (yRangeDblMin > yVal)
		{
			yRangeDblMin = yVal;
		}
		if (yRangeDblMax < yVal)
		{
			yRangeDblMax = yVal;
		}
	}
	else
	{
		yRangeDblMin = yVal;
		yRangeDblMax = yVal;
		hasYRangeDbl = true;
	}
}

/*void Data::LineChart::SetStyle(Data::LineChart::LineStyle style)
{
	this->style = style;
}*/

void Data::LineChart::Plot(Media::DrawImage *img, Double x, Double y, Double width, Double height)
{
	if (height == 0 || width == 0)
		return;
	if (this->yCharts->GetCount() == 0)
	{
		if (!this->hasXRangeDate || this->xRangeDateMax == this->xRangeDateMin)
			return;

		Bool found = false;
		if (this->hasYRangeDbl && this->yRangeDblMax != this->yRangeDblMin)
			found = true;
		if (this->hasYRangeInt && this->yRangeIntMax != this->yRangeIntMin)
			found = true;
		if (!found)
			return;
	}

	Media::DrawFont *fnt;
	Double fntH;
	Bool customX = false;

//	Int32 xAxisPos;
	Int32 xMaxInt = 0;
	Int32 xMinInt = 0;
	Double xMaxDbl = 0;
	Double xMinDbl = 0;
	Int64 xMaxDate = 0;
	Int64 xMinDate = 0;
//	Int32 yAxis1Pos;
	Data::LineChart::ChartType yAxis1Type;
//	Int32 yAxis2Pos;
	Data::LineChart::ChartType yAxis2Type;
	Int32 y1MaxInt = 0;
	Int32 y1MinInt = 0;
	Int32 y2MaxInt = 0;
	Int32 y2MinInt = 0;
	Double y1MaxDbl = 0;
	Double y1MinDbl = 0;
	Double y2MaxDbl = 0;
	Double y2MinDbl = 0;
	Int64 y1MaxDate = 0;
	Int64 y1MinDate = 0;
	Int64 y2MaxDate = 0;
	Int64 y2MinDate = 0;

	Double barLeng = this->barLength;
	Double xLeng;
	Double y1Leng;
	Double y2Leng;
	Bool y2show;
	UTF8Char sbuff[256];
	UTF8Char *sptr;
	const UTF8Char *csptr;

	UOSInt i;
	UOSInt j;
	Data::DateTime dt1;
	Data::DateTime dt2;

	Media::DrawBrush *bgBrush = img->NewBrushARGB(bgColor);
	Media::DrawPen *boundPen = img->NewPenARGB(boundColor, this->lineThick, 0, 0);
	Media::DrawBrush *fontBrush = img->NewBrushARGB(fontColor);
	Media::DrawPen *gridPen = img->NewPenARGB(gridColor, this->lineThick, 0, 0);
	Media::DrawPen *refLinePen = img->NewPenARGB(refLineColor, this->lineThick, 0, 0);

	fnt = img->NewFontPt(fntName, (DRAW_UNIT)fntSizePt, Media::DrawEngine::DFS_ANTIALIAS, 0);
	img->DrawRect((DRAW_UNIT)x, (DRAW_UNIT)y, (DRAW_UNIT)width, (DRAW_UNIT)height, 0, bgBrush);

	Double rcSize[2];
	rcSize[0] = 0;
	rcSize[1] = 0;
	img->GetTextSizeC(fnt, (const UTF8Char*)"AA", 2, rcSize);
	fntH = rcSize[1];
	if (this->titleLineCnt > 0)
	{
		i = 0;
		while (i < this->titleLineCnt)
		{
			img->GetTextSize(fnt, this->titleLine[i], rcSize);
			img->DrawString((x + (width / 2) - (rcSize[0] * 0.5)), y, this->titleLine[i], fnt, fontBrush);
			y += fntH;
			height -= fntH;
			i++;
		}
	}


	if (xType == Data::LineChart::CT_NO || xDatas->GetCount() == 0)
	{
		if (this->yCharts->GetCount() == 0)
		{
			if (this->hasXRangeDate)
			{
				xType = Data::LineChart::CT_DATETICK;
				xMaxDate = this->xRangeDateMax;
				xMinDate = this->xRangeDateMin;
			}
		}
		else
		{
			xType = Data::LineChart::CT_INTEGER;
			UOSInt dataLeng;
			Data::LineChart::ChartData *data = ((Data::LineChart::ChartData*)yCharts->GetItem(0));
			Int32 *tmpData = MemAlloc(Int32, dataLeng = (i = data->dataCnt));
			while (i-- > 0)
				tmpData[i] = (Int32)i;
			this->xDatas->Add(tmpData);
			this->xDataCnt->Add(dataLeng);
			xMaxInt = (Int32)dataLeng - 1;
			xMinInt = 0;
			if (data->dataCnt == 2)
				xMaxInt = 1;
			customX = true;
		}
	}
	else if (xType == Data::LineChart::CT_DATETICK)
	{
		Int64 *tmpdata;
		UOSInt tmpdataCnt;
		xMaxDate = ((Int64*)xDatas->GetItem(0))[0];
		xMinDate = ((Int64*)xDatas->GetItem(0))[0];
		i = 0;
		while (i < this->xDatas->GetCount())
		{
			tmpdata = (Int64*)xDatas->GetItem(i);
			tmpdataCnt = xDataCnt->GetItem(i);
			j = 0;
			while (j < tmpdataCnt)
			{
				if (tmpdata[j] > xMaxDate)
					xMaxDate = tmpdata[j];

				if (tmpdata[j] < xMinDate)
					xMinDate = tmpdata[j];
				j++;
			}
			i++;
		}
		if (xMaxDate == xMinDate)
		{
			xMaxDate += 3600000;
		}
	}
	else if (xType == Data::LineChart::CT_DOUBLE)
	{
		Double *tmpdata;
		UOSInt tmpdataCnt;
		xMinDbl = xMaxDbl = ((Double*)xDatas->GetItem(0))[0];
		i = 0;
		while (i < this->xDatas->GetCount())
		{
			tmpdata = (Double*)xDatas->GetItem(i);
			tmpdataCnt = xDataCnt->GetItem(i);
			j = 0;
			while (j < tmpdataCnt)
			{
				if (tmpdata[j] > xMaxDbl)
					xMaxDbl = tmpdata[j];
				if (tmpdata[j] < xMinDbl)
					xMinDbl = tmpdata[j];
				j++;
			}
			i++;
		}
		if (xMaxDbl == xMinDbl)
		{
			xMaxDbl = xMaxDbl + 1;
		}
	}
	else if (xType == Data::LineChart::CT_INTEGER)
	{
		Int32 *tmpdata;
		UOSInt tmpdataCnt;
		xMinInt = xMaxInt = ((Int32*)xDatas->GetItem(0))[0];
		i = 0;
		while (i < this->xDatas->GetCount())
		{
			tmpdata = (Int32*)xDatas->GetItem(i);
			tmpdataCnt = xDataCnt->GetItem(i);
			j = 0;
			while (j < tmpdataCnt)
			{
				if (tmpdata[j] > xMaxInt)
					xMaxInt = tmpdata[j];
				if (tmpdata[j] < xMinInt)
					xMinInt = tmpdata[j];
				j++;
			}
			i++;
		}
		if (xMaxInt == xMinInt)
			xMaxInt = xMaxInt + 1;
	}



	yAxis1Type = Data::LineChart::CT_NO;
	yAxis2Type = Data::LineChart::CT_NO;
	if (this->yCharts->GetCount() > 0)
	{
		i = 0;
		while (i < this->yCharts->GetCount())
		{
			Data::LineChart::ChartData *data = (Data::LineChart::ChartData*)this->yCharts->GetItem(i);
			if (data->dataType == Data::LineChart::CT_INTEGER)
			{
				Int32 *datas = (Int32*)data->data;
				if (yAxis1Type == Data::LineChart::CT_NO)
				{
					if (this->hasYRangeInt)
					{
						y1MinInt = this->yRangeIntMin;
						y1MaxInt = this->yRangeIntMax;
					}
					else if (refExist)
					{
						y1MinInt = y1MaxInt = refInt;
					}
					else
					{
						y1MaxInt = datas[0];
						y1MinInt = datas[0];
					}
					yAxis1Type = Data::LineChart::CT_INTEGER;
					j = 0;
					while (j < data->dataCnt)
					{
						if (datas[j] > y1MaxInt)
							y1MaxInt = datas[j];
						if (datas[j] < y1MinInt)
							y1MinInt = datas[j];
						j++;
					}
					if (y1MaxInt == y1MinInt)
						y1MaxInt = y1MinInt + 1;
				}
				else if (yAxis1Type == Data::LineChart::CT_INTEGER)
				{
					j = 0;
					while (j < data->dataCnt)
					{
						if (datas[j]> y1MaxInt)
							y1MaxInt = datas[j];
						if (datas[j] < y1MinInt)
							y1MinInt = datas[j];
						j++;
					}
					if (y1MaxInt == y1MinInt)
						y1MaxInt = y1MinInt + 1;
				}
				else if (yAxis2Type == Data::LineChart::CT_NO)
				{
					yAxis2Type = Data::LineChart::CT_INTEGER;
					if (this->hasYRangeInt)
					{
						y2MinInt = this->yRangeIntMin;
						y2MaxInt = this->yRangeIntMax;
					}
					else if (refExist)
					{
						y2MinInt = y2MaxInt = refInt;
					}
					else
					{
						y2MaxInt = datas[0];
						y2MinInt = datas[0];
					}
					j = 0;
					while (j < data->dataCnt)
					{
						if (datas[j] > y2MaxInt)
							y2MaxInt = datas[j];
						if (datas[j] < y2MinInt)
							y2MinInt = datas[j];
						j++;
					}
					if (y2MaxInt == y2MinInt)
						y2MaxInt = y2MinInt + 1;
				}
				else if (yAxis2Type == Data::LineChart::CT_INTEGER)
				{
					j = 0;
					while (j < data->dataCnt)
					{
						if (datas[j] > y2MaxInt)
							y2MaxInt = datas[j];
						if (datas[j] < y2MinInt)
							y2MinInt = datas[j];
						j++;
					}
					if (y2MaxInt == y2MinInt)
						y2MaxInt = y2MinInt + 1;
				}
				else
				{
					//throw new System::Exception("Unsupported chart");
					return;
				}
			}
			else if (data->dataType == Data::LineChart::CT_DOUBLE)
			{
				Double *datas = (Double*)data->data;
				if (yAxis1Type == Data::LineChart::CT_NO)
				{
					if (this->hasYRangeDbl)
					{
						y1MinDbl = this->yRangeDblMin;
						y1MaxDbl = this->yRangeDblMax;
					}
					else if (refExist)
					{
						y1MinDbl = y1MaxDbl = refDbl;
					}
					else
					{
						y1MaxDbl = datas[0];
						y1MinDbl = datas[0];
					}
					yAxis1Type = Data::LineChart::CT_DOUBLE;
					j = 0;
					while (j < data->dataCnt)
					{
						if (datas[j] > y1MaxDbl)
							y1MaxDbl = datas[j];
						if (datas[j] < y1MinDbl)
							y1MinDbl = datas[j];
						j++;
					}
					if (y1MaxDbl == y1MinDbl)
						y1MaxDbl = y1MinDbl + minDblVal;
				}
				else if (yAxis1Type == Data::LineChart::CT_DOUBLE)
				{
					j = 0;
					while (j < data->dataCnt)
					{
						if (datas[j] > y1MaxDbl)
							y1MaxDbl = datas[j];
						if (datas[j] < y1MinDbl)
							y1MinDbl = datas[j];
						j++;
					}
					if (y1MaxDbl == y1MinDbl)
						y1MaxDbl = y1MinDbl + minDblVal;
				}
				else if (yAxis2Type == Data::LineChart::CT_NO)
				{
					yAxis2Type = Data::LineChart::CT_DOUBLE;
					if (this->hasYRangeDbl)
					{
						y2MinDbl = this->yRangeDblMin;
						y2MaxDbl = this->yRangeDblMax;
					}
					else if (refExist)
					{
						y2MinDbl = y2MaxDbl = refDbl;
					}
					else
					{
						y2MaxDbl = datas[0];
						y2MinDbl = datas[0];
					}
					j = 0;
					while (j < data->dataCnt)
					{
						if (datas[j] > y2MaxDbl)
							y2MaxDbl = datas[j];
						if (datas[j] < y2MinDbl)
							y2MinDbl = datas[j];
						j++;
					}
					if (y2MaxDbl == y2MinDbl)
						y2MaxDbl = y2MinDbl + minDblVal;
				}
				else if (yAxis2Type == Data::LineChart::CT_DOUBLE)
				{
					j = 0;
					while (j < data->dataCnt)
					{
						if (datas[j] > y2MaxDbl)
							y2MaxDbl = datas[j];
						if (datas[j] < y2MinDbl)
							y2MinDbl = datas[j];
						j++;
					}
					if (y2MaxDbl == y2MinDbl)
						y2MaxDbl = y2MinDbl + minDblVal;

				}
				else
				{
				//	throw new System::Exception("Unsupported chart");
					return;
				}
			}
			else if (data->dataType == Data::LineChart::CT_DATETICK)
			{
				Int64 *datas = (Int64*)data->data;
				if (yAxis1Type == Data::LineChart::CT_NO)
				{
					if (refExist)
					{
						y1MinDate = y1MaxDate = this->refTime;
					}
					else
					{
						y1MaxDate = datas[0];
						y1MinDate = datas[0];
					}
					yAxis1Type = Data::LineChart::CT_DATETICK;
					j = 0;
					while (j < data->dataCnt)
					{
						if (datas[j] > y1MaxDate)
							y1MaxDate = datas[j];
						if (datas[j] < y1MinDate)
							y1MinDate = datas[j];
						j++;
					}
					if (y1MaxDate == y1MinDate)
					{
						y1MaxDate = y1MinDate;
						y1MaxDate += 3600000;
					}
				}
				else if (yAxis1Type == Data::LineChart::CT_DATETICK)
				{
					j = 0;
					while (j < data->dataCnt)
					{
						if (datas[j] > y1MaxDate)
							y1MaxDate = datas[j];
						if (datas[j] < y1MinDate)
							y1MinDate = datas[j];
						j++;
					}
					if (y1MaxDate == y1MinDate)
					{
						y1MaxDate = y1MinDate;
						y1MaxDate += 3600000;
					}
				}
				else if (yAxis2Type == Data::LineChart::CT_NO)
				{
					yAxis2Type = Data::LineChart::CT_DATETICK;
					if (refExist)
					{
						y2MinDate = y2MaxDate = refTime;
					}
					else
					{
						y2MaxDate = datas[0];
						y2MinDate = datas[0];
					}
					j = 0;
					while (j < data->dataCnt)
					{
						if (datas[j] > y2MaxDate)
							y2MaxDate = datas[j];
						if (datas[j] < y2MinDate)
							y2MinDate = datas[j];
						j++;
					}
					if (y2MaxDate == y2MinDate)
					{
						y2MaxDate = y2MinDate;
						y2MaxDate += 3600000;
					}
				}
				else if (yAxis2Type == Data::LineChart::CT_DATETICK)
				{
					j = 0;
					while (j < data->dataCnt)
					{
						if (datas[j] > y2MaxDate)
							y2MaxDate = datas[j];
						if (datas[j] < y2MinDate)
							y2MinDate = datas[j];
						j++;
					}
					if (y2MaxDate == y2MinDate)
					{
						y2MaxDate = y2MinDate;
						y2MaxDate += 3600000;
					}
				}
				else
				{
				//	throw new System::Exception("Unsupported chart");
				}
			}
			i++;
		}
	}
	else
	{
		if (this->hasYRangeDbl && this->yRangeDblMax != this->yRangeDblMin)
		{
			yAxis1Type = Data::LineChart::CT_DOUBLE;
			y1MaxDbl = this->yRangeDblMax;
			y1MinDbl = this->yRangeDblMin;
		}
		else if (this->hasYRangeInt && this->yRangeIntMax != this->yRangeIntMin)
		{
			yAxis1Type = Data::LineChart::CT_INTEGER;
			y1MaxInt = this->yRangeIntMax;
			y1MinInt = this->yRangeIntMin;
		}
	}

	xLeng = 0;
	y1Leng = 0;
	y2Leng = 0;
	if (xType == Data::LineChart::CT_INTEGER)
	{
		sptr = Text::StrInt32(sbuff, xMaxInt);
		img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);
		xLeng = (Single)rcSize[0];

		sptr = Text::StrInt32(sbuff, xMinInt);
		img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);
		if (rcSize[0] > xLeng)
			xLeng = (Single)rcSize[0];
	}
	else if (xType == Data::LineChart::CT_DOUBLE)
	{
		sptr = Text::StrDoubleFmt(sbuff, xMaxDbl, dblFormat);
		img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);
		xLeng = (Single)rcSize[0];
		
		sptr = Text::StrDoubleFmt(sbuff, xMinDbl, dblFormat);
		img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);
		if (rcSize[0] > xLeng)
			xLeng = (Single)rcSize[0];
	}
	else if (xType == Data::LineChart::CT_DATETICK)
	{
		dt1.SetTicks(xMaxDate);
		dt1.ConvertTimeZoneQHR(this->timeZoneQHR);
		dt2.SetTicks(xMinDate);
		dt2.ConvertTimeZoneQHR(this->timeZoneQHR);
		if (dt1.IsSameDay(&dt2))
		{
			if (timeFormat)
			{
				sptr = dt1.ToString(sbuff, timeFormat);
			}
			else
			{
				sptr = Text::StrConcat(sbuff, (const UTF8Char*)"");
			}
			img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);
			xLeng = (Single)rcSize[0];
			if (dt2.GetMSPassedDate() == 0)
			{
				sptr = dt2.ToString(sbuff, dateFormat);
			}
			else
			{
				if (timeFormat)
				{
					sptr = dt2.ToString(sbuff, timeFormat);
				}
				else
				{
					sptr = Text::StrConcat(sbuff, (const UTF8Char*)"");
				}
			}
			img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);
			if (rcSize[0] > xLeng)
				xLeng = (Single)rcSize[0];
		}
		else
		{
			sptr = dt1.ToString(sbuff, dateFormat);
			img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);
			xLeng = (Single)rcSize[0];
			if (timeFormat)
			{
				sptr = dt1.ToString(sbuff, timeFormat);
			}
			else
			{
				sptr = Text::StrConcat(sbuff, (const UTF8Char*)"");
			}
			img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);
			if (rcSize[0] > xLeng)
				xLeng = (Single)rcSize[0];
		}
	}
	if (xLabel)
	{
		img->GetTextSize(fnt, xLabel, rcSize);
		xLeng += rcSize[1];
	}
	xLeng += barLeng;


	if (yAxis1Type == Data::LineChart::CT_INTEGER)
	{
		sptr = Text::StrInt32(sbuff, y1MaxInt);
		if (this->yUnit)
			sptr = Text::StrConcat(sptr, this->yUnit);
		img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);
		y1Leng = rcSize[0];

		sptr = Text::StrInt32(sbuff, y1MinInt);
		if (this->yUnit)
			sptr = Text::StrConcat(sptr, this->yUnit);
		img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);
		if (rcSize[0] > y1Leng)
			y1Leng = rcSize[0];
	}
	else if (yAxis1Type == Data::LineChart::CT_DOUBLE)
	{
		sptr = Text::StrDoubleFmt(sbuff, y1MaxDbl, dblFormat);
		if (this->yUnit)
			sptr = Text::StrConcat(sptr, this->yUnit);
		img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);;
		y1Leng = rcSize[0];

		sptr = Text::StrDoubleFmt(sbuff, y1MinDbl, dblFormat);
		if (this->yUnit)
			sptr = Text::StrConcat(sptr, this->yUnit);
		img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);
		if (rcSize[0] > y1Leng)
			y1Leng = rcSize[0];
	}
	else if (yAxis1Type == Data::LineChart::CT_DATETICK)
	{
		dt1.SetTicks(y1MaxDate);
		dt1.ConvertTimeZoneQHR(this->timeZoneQHR);
		sptr = dt1.ToString(sbuff, dateFormat);
		img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);
		y1Leng = rcSize[0];

		dt1.SetTicks(y1MinDate);
		dt1.ConvertTimeZoneQHR(this->timeZoneQHR);
		sptr = dt1.ToString(sbuff, dateFormat);
		img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);;
		if (rcSize[0] > y1Leng)
			y1Leng = rcSize[0];
	}
	if (yLabel)
	{
		img->GetTextSize(fnt, yLabel, rcSize);;
		y1Leng += rcSize[1];
	}
	y1Leng += barLeng;



	if (yAxis2Type == Data::LineChart::CT_INTEGER)
	{
		sptr = Text::StrInt32(sbuff, y2MaxInt);
		if (this->yUnit)
			sptr = Text::StrConcat(sptr, this->yUnit);
		img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);
		y2Leng = rcSize[0];

		sptr = Text::StrInt32(sbuff, y2MinInt);
		if (this->yUnit)
			sptr = Text::StrConcat(sptr, this->yUnit);
		img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);
		if (rcSize[0] > y2Leng)
			y2Leng = rcSize[0];

		y2Leng += barLeng;
		y2show = true;
	}
	else if (yAxis2Type == Data::LineChart::CT_DOUBLE)
	{
		sptr = Text::StrDoubleFmt(sbuff, y2MaxDbl, dblFormat);
		if (this->yUnit)
			sptr = Text::StrConcat(sptr, this->yUnit);
		img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);
		y2Leng = rcSize[0];

		sptr = Text::StrDoubleFmt(sbuff, y2MinDbl, dblFormat);
		if (this->yUnit)
			sptr = Text::StrConcat(sptr, this->yUnit);
		img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);
		if (rcSize[0] > y2Leng)
			y2Leng = rcSize[0];

		y2Leng += barLeng;
		y2show = true;
	}
	else if (yAxis2Type == Data::LineChart::CT_DATETICK)
	{
		dt1.SetTicks(y2MaxDate);
		dt1.ConvertTimeZoneQHR(this->timeZoneQHR);
		sptr = dt1.ToString(sbuff, dateFormat);
		img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);
		y2Leng = rcSize[0];

		dt1.SetTicks(y2MinDate);
		dt1.ConvertTimeZoneQHR(this->timeZoneQHR);
		sptr = dt1.ToString(sbuff, dateFormat);
		img->GetTextSizeC(fnt, sbuff, (UOSInt)(sptr - sbuff), rcSize);
		if (rcSize[0] > y2Leng)
			y2Leng = rcSize[0];

		y2Leng += barLeng;
		y2show = true;
	}
	else if (yAxis2Type == Data::LineChart::CT_NO)
	{
		y2Leng = (rcSize[1] / 2.0);
		y2show = false;
	}
	else
	{
		y2Leng = (rcSize[1] / 2.0);
		y2show = false;
	}

	img->DrawLine((DRAW_UNIT)(x + y1Leng), (DRAW_UNIT)y, (DRAW_UNIT)(x + y1Leng), (DRAW_UNIT)(y + height - xLeng), boundPen);
	img->DrawLine((DRAW_UNIT)(x + y1Leng), (DRAW_UNIT)(y + height - xLeng), (DRAW_UNIT)(x + width - y2Leng), (DRAW_UNIT)(y + height - xLeng), boundPen);
	if (y2show)
	{
		img->DrawLine((DRAW_UNIT)(x + width - y2Leng), (DRAW_UNIT)y, (DRAW_UNIT)(x + width - y2Leng), (DRAW_UNIT)(y + height - xLeng), boundPen);
	}
	
	Data::ArrayListDbl *locations;
	Data::ArrayList<const UTF8Char*> *labels;
	NEW_CLASS(locations, Data::ArrayListDbl());
	NEW_CLASS(labels, Data::ArrayList<const UTF8Char*>());
	if (xType == Data::LineChart::CT_INTEGER)
	{
		Data::IChart::CalScaleMarkInt(locations, labels, xMinInt, xMaxInt, width - y1Leng - y2Leng - this->pointSize * 2, fntH, 0);
	}
	else if (xType == Data::LineChart::CT_DOUBLE)
	{
		Data::IChart::CalScaleMarkDbl(locations, labels, xMinDbl, xMaxDbl, width - y1Leng - y2Leng - this->pointSize * 2, fntH, dblFormat, minDblVal, 0);
	}
	else if (xType == Data::LineChart::CT_DATETICK)
	{
		dt1.SetTicks(xMinDate);
		dt1.ConvertTimeZoneQHR(this->timeZoneQHR);
		dt2.SetTicks(xMaxDate);
		dt2.ConvertTimeZoneQHR(this->timeZoneQHR);
		Data::IChart::CalScaleMarkDate(locations, labels, &dt1, &dt2, width - y1Leng - y2Leng - this->pointSize * 2, fntH, dateFormat, timeFormat);
	}
	else
	{
	}

	i = 0;
	while (i < locations->GetCount())
	{
		img->DrawLine((DRAW_UNIT)(x + y1Leng + this->pointSize + locations->GetItem(i)), (DRAW_UNIT)(y + height - xLeng), (DRAW_UNIT)(x + y1Leng + this->pointSize + locations->GetItem(i)), (DRAW_UNIT)(y + height - xLeng + barLeng), boundPen);
		i++;
	}


	img->SetTextAlign(Media::DrawEngine::DRAW_POS_CENTERRIGHT);
	i = 0;
	while (i < locations->GetCount())
	{
		img->DrawStringRot((DRAW_UNIT)(x + y1Leng + this->pointSize + locations->GetItem(i)), (DRAW_UNIT)(y + height - xLeng + barLeng), labels->GetItem(i), fnt, fontBrush, 90);
		i += 1;
	}
	img->SetTextAlign(Media::DrawEngine::DRAW_POS_TOPLEFT);

	locations->Clear();
	i = labels->GetCount();
	while (i-- > 0)
	{
		Text::StrDelNew(labels->GetItem(i));
	}
	labels->Clear();

	if (yAxis1Type == Data::LineChart::CT_INTEGER)
	{
		Data::IChart::CalScaleMarkInt(locations, labels, y1MinInt, y1MaxInt, height - xLeng - fntH / 2 - this->pointSize * 2, fntH, this->yUnit);
	}
	else if (yAxis1Type == Data::LineChart::CT_DOUBLE)
	{
		Data::IChart::CalScaleMarkDbl(locations, labels, y1MinDbl, y1MaxDbl, height - xLeng - fntH / 2 - this->pointSize * 2, fntH, dblFormat, minDblVal, this->yUnit);
	}
	else if (yAxis1Type == Data::LineChart::CT_DATETICK)
	{
		dt1.SetTicks(y1MinDate);
		dt1.ConvertTimeZoneQHR(this->timeZoneQHR);
		dt2.SetTicks(y2MaxDate);
		dt2.ConvertTimeZoneQHR(this->timeZoneQHR);
		Data::IChart::CalScaleMarkDate(locations, labels, &dt1, &dt2, height - xLeng - fntH / 2 - this->pointSize * 2, fntH, dateFormat, timeFormat);
	}
	else
	{
	}

	i = 0;
	while (i < locations->GetCount())
	{
		if (locations->GetItem(i))
		{
			img->DrawLine((DRAW_UNIT)(x + y1Leng), (DRAW_UNIT)(y + height - this->pointSize - xLeng - locations->GetItem(i)), (DRAW_UNIT)(x + width - y2Leng), (DRAW_UNIT)(y + height - this->pointSize - xLeng - locations->GetItem(i)), gridPen);
		}
		img->DrawLine((DRAW_UNIT)(x + y1Leng), (DRAW_UNIT)(y + height - this->pointSize - xLeng - locations->GetItem(i)), (DRAW_UNIT)(x + y1Leng - barLeng), (DRAW_UNIT)(y + height - this->pointSize - xLeng - locations->GetItem(i)), boundPen);
		csptr = labels->GetItem(i);
		img->GetTextSize(fnt, csptr, rcSize);
		img->DrawString((DRAW_UNIT)(x + y1Leng - barLeng - rcSize[0]), (DRAW_UNIT)(y + height - this->pointSize - xLeng - locations->GetItem(i) - fntH / 2), csptr, fnt, fontBrush);
		i++;
	}

	if (yLabel)
	{
		img->SetTextAlign(Media::DrawEngine::DRAW_POS_CENTER);
		img->DrawStringRot((DRAW_UNIT)(x + fntH / 2), (DRAW_UNIT)(y + (height - xLeng) / 2), yLabel, fnt, fontBrush, 90);
		img->SetTextAlign(Media::DrawEngine::DRAW_POS_TOPLEFT);
	}

	if (xLabel)
	{
		img->GetTextSize(fnt, xLabel, rcSize);
		img->DrawString((DRAW_UNIT)(x + y1Leng + (width - y1Leng - y2Leng) / 2 - rcSize[0] / 2), (DRAW_UNIT)(y + height - rcSize[1]), xLabel, fnt, fontBrush);
	}

	locations->Clear();
	i = labels->GetCount();
	while (i-- > 0)
	{
		Text::StrDelNew(labels->GetItem(i));
	}
	labels->Clear();


//	System::Drawing::PointF currPos[];
	DRAW_UNIT *currPos;
	UOSInt currPosLen;
    
	i = 0;
	while (i < yCharts->GetCount())
	{
		void *xData;
		UOSInt xDataCnt;
		Data::LineChart::ChartData *chart = (Data::LineChart::ChartData*)yCharts->GetItem(i);

		if (xDatas->GetCount() > i)
		{
			xData = xDatas->GetItem(i);
			xDataCnt = this->xDataCnt->GetItem(i);
		}
		else
		{
			xData = xDatas->GetItem(0);
			xDataCnt = this->xDataCnt->GetItem(0);
		}

		if (chart->lineStyle == Data::LineChart::LS_FILL)
		{
			currPosLen = xDataCnt + 2;
			currPos = MemAlloc(DRAW_UNIT, currPosLen << 1);
		}
		else
		{
			currPosLen = xDataCnt;
			currPos = MemAlloc(DRAW_UNIT, currPosLen << 1);
		}

		Double xChartLeng = width - y1Leng - y2Leng - this->pointSize * 2.0;
		if (xType == Data::LineChart::CT_DATETICK)
		{
			Int64 *data = (Int64*)xData;
			j = 0;
			while (j < xDataCnt)
			{
				currPos[j << 1] = (DRAW_UNIT)(x + y1Leng + this->pointSize + Data::DateTime::MS2Minutes(data[j] - xMinDate) / Data::DateTime::MS2Minutes(xMaxDate - xMinDate) * xChartLeng);
				j++;
			}
		}
		else if (xType == Data::LineChart::CT_DOUBLE)
		{
			Double *data = (Double*)xData;
			j = 0;
			while (j < xDataCnt)
			{
				currPos[j << 1] = (DRAW_UNIT)(x + y1Leng + this->pointSize + (data[j] - xMinDbl) / (xMaxDbl - xMinDbl) * xChartLeng);
				j++;
			}
		}
		else if (xType == Data::LineChart::CT_INTEGER)
		{
			Int32 *data = (Int32*)xData;

			j = 0;
			while (j < xDataCnt)
			{
				currPos[j << 1] = (DRAW_UNIT)(x + y1Leng + this->pointSize + (Double)(data[j] - xMinInt) / (Single)(xMaxInt - xMinInt) * xChartLeng);
				j++;
			}
		}

		xChartLeng = height - xLeng - fntH / 2 - this->pointSize * 2;
		if (chart->dataType == Data::LineChart::CT_INTEGER)
		{
			Int32 *data = (Int32*)chart->data;
			Int32 iMax = 0;
			Int32 iMin = 0;
			if (chart->dataType == yAxis1Type)
			{
				iMax = y1MaxInt;
				iMin = y1MinInt;
			}
			else if  (chart->dataType == yAxis2Type)
			{
				iMax = y2MaxInt;
				iMin = y2MinInt;
			}

			j = 0;
			while (j < chart->dataCnt)
			{
				currPos[(j << 1) + 1] = (DRAW_UNIT)(y + height - this->pointSize - xLeng - (Double)(data[j] - iMin) / (Single)(iMax - iMin) * xChartLeng);
				j++;
			}
		}
		else if (chart->dataType == Data::LineChart::CT_DOUBLE)
		{
			Double *data = (Double*)chart->data;
			Double dMax = 0;
			Double dMin = 0;
			if (chart->dataType == yAxis1Type)
			{
				dMax = y1MaxDbl;
				dMin = y1MinDbl;
			}
			else if  (chart->dataType == yAxis2Type)
			{
				dMax = y2MaxDbl;
				dMin = y2MinDbl;
			}


			j = 0;
			while (j < chart->dataCnt)
			{
				currPos[(j << 1) + 1] = (DRAW_UNIT)(y + height - this->pointSize - xLeng - (data[j] - dMin) / (dMax - dMin) * xChartLeng);
				j++;
			}
		}
		else if (chart->dataType == Data::LineChart::CT_DATETICK)
		{
			Int64 *data = (Int64 *)chart->data;
			Int64 dMax = 0;
			Int64 dMin = 0;
			if (chart->dataType == yAxis1Type)
			{
				dMax = y1MaxDate;
				dMin = y1MinDate;
			}
			else if  (chart->dataType == yAxis2Type)
			{
				dMax = y2MaxDate;
				dMin = y2MinDate;
			}

			j = 0;
			while (j < chart->dataCnt)
			{
				currPos[(j << 1) + 1] = (DRAW_UNIT)(y + height - this->pointSize - xLeng - Data::DateTime::MS2Minutes(data[j] - dMin) / Data::DateTime::MS2Minutes(dMax - dMin) * xChartLeng);
				j++;
			}
		}

		if (chart->lineStyle == Data::LineChart::LS_FILL)
		{
			if (currPosLen >= 4)
			{
				j = currPosLen;
				currPos[(j << 1) - 4] = currPos[(j << 1) - 6];
				currPos[(j << 1) - 3] = (DRAW_UNIT)(y + height - xLeng);
				currPos[(j << 1) - 2] = currPos[0];
				currPos[(j << 1) - 1] = (DRAW_UNIT)(y + height - xLeng);
				Media::DrawPen *p = img->NewPenARGB(((Data::LineChart::ChartData*)yCharts->GetItem(i))->lineColor, 1, 0, 0);
				Media::DrawBrush *b = img->NewBrushARGB(((Data::LineChart::ChartData*)yCharts->GetItem(i))->lineColor);
				img->DrawPolygon(currPos, currPosLen, p, b);
				img->DelBrush(b);
				img->DelPen(p);
			}
		}
		else
		{
			if (currPosLen >= 2)
			{
				Media::DrawPen *pen = img->NewPenARGB(((Data::LineChart::ChartData*)yCharts->GetItem(i))->lineColor, this->lineThick, 0, 0);
				img->DrawPolyline(currPos, currPosLen, pen);
				img->DelPen(pen);

				if (this->pointType == PT_CIRCLE && this->pointSize > 0)
				{
					Media::DrawBrush *b = img->NewBrushARGB(((Data::LineChart::ChartData*)yCharts->GetItem(i))->lineColor);
					j = currPosLen;
					while (j-- > 0)
					{
						img->DrawEllipse(currPos[(j << 1)] - this->pointSize, currPos[(j << 1) + 1] - this->pointSize, this->pointSize * 2.0, this->pointSize * 2.0, 0, b);
					}
					img->DelBrush(b);
				}
			}
		}


		MemFree(currPos);
		i += 1;
	}

	if (this->refExist)
	{
		Double xChartLeng = height - xLeng - fntH / 2;
		Int32 iMax = 0;
		Int32 iMin = 0;
		Single yPos;
		Double dMax;
		Double dMin;
		Int64 tMax;
		Int64 tMin;

		if (yAxis1Type == Data::LineChart::CT_INTEGER)
		{
			iMax = y1MaxInt;
			iMin = y1MinInt;
			if (this->refInt >= iMin && this->refInt <= iMax)
			{
				yPos = (Single)(y + height - xLeng - (Double)(this->refInt - iMin) / (Single)(iMax - iMin) * xChartLeng);
				img->DrawLine((DRAW_UNIT)(x + y1Leng), (DRAW_UNIT)yPos, (DRAW_UNIT)(x + width - y2Leng), (DRAW_UNIT)yPos, refLinePen);

				sptr = Text::StrInt32(sbuff, this->refInt);
				if (this->yUnit)
					sptr = Text::StrConcat(sptr, this->yUnit);
				if (this->refType == RT_LEFTALIGN)
				{
					img->GetTextSize(fnt, sbuff, rcSize);
					img->DrawString((DRAW_UNIT)(x + y1Leng), yPos - rcSize[1], sbuff, fnt, fontBrush);
				}
				else if (this->refType == RT_RIGHTALIGN)
				{
					img->GetTextSize(fnt, sbuff, rcSize);
					img->DrawString((DRAW_UNIT)(x + width - y2Leng - rcSize[0]), yPos - rcSize[1], sbuff, fnt, fontBrush);
				}
			}
		}
		else if (yAxis2Type == Data::LineChart::CT_INTEGER)
		{
			iMax = y2MaxInt;
			iMin = y2MinInt;
			if (this->refInt >= iMin && this->refInt <= iMax)
			{
				yPos = (Single)(y + height - xLeng - (Double)(this->refInt - iMin) / (Single)(iMax - iMin) * xChartLeng);
				img->DrawLine((DRAW_UNIT)(x + y1Leng), (DRAW_UNIT)yPos, (DRAW_UNIT)(x + width - y2Leng), (DRAW_UNIT)yPos, refLinePen);

				sptr = Text::StrInt32(sbuff, this->refInt);
				if (this->yUnit)
					sptr = Text::StrConcat(sptr, this->yUnit);
				if (this->refType == RT_LEFTALIGN)
				{
					img->GetTextSize(fnt, sbuff, rcSize);
					img->DrawString((DRAW_UNIT)(x + y1Leng), yPos - rcSize[1], sbuff, fnt, fontBrush);
				}
				else if (this->refType == RT_RIGHTALIGN)
				{
					img->GetTextSize(fnt, sbuff, rcSize);
					img->DrawString((DRAW_UNIT)(x + width - y2Leng - rcSize[0]), yPos - rcSize[1], sbuff, fnt, fontBrush);
				}
			}
		}

		if (Data::LineChart::CT_DOUBLE == yAxis1Type)
		{
			dMax = y1MaxDbl;
			dMin = y1MinDbl;
			if (this->refDbl >= dMin && this->refDbl <= dMax)
			{
				yPos = (Single)(y + height - xLeng - (this->refDbl - dMin) / (dMax - dMin) * xChartLeng);
				img->DrawLine((DRAW_UNIT)(x + y1Leng), (DRAW_UNIT)yPos, (DRAW_UNIT)(x + width - y2Leng), (DRAW_UNIT)yPos, refLinePen);

				sptr = Text::StrDoubleFmt(sbuff, this->refDbl, dblFormat);
				if (this->yUnit)
					sptr = Text::StrConcat(sptr, this->yUnit);
				if (this->refType == RT_LEFTALIGN)
				{
					img->GetTextSize(fnt, sbuff, rcSize);
					img->DrawString((DRAW_UNIT)(x + y1Leng), yPos - rcSize[1], sbuff, fnt, fontBrush);
				}
				else if (this->refType == RT_RIGHTALIGN)
				{
					img->GetTextSize(fnt, sbuff, rcSize);
					img->DrawString((DRAW_UNIT)(x + width - y2Leng - rcSize[0]), yPos - rcSize[1], sbuff, fnt, fontBrush);
				}
			}
		}
		else if  (Data::LineChart::CT_DOUBLE == yAxis2Type)
		{
			dMax = y2MaxDbl;
			dMin = y2MinDbl;
			if (this->refDbl >= dMin && this->refDbl <= dMax)
			{
				yPos = (Single)(y + height - xLeng - (this->refDbl - dMin) / (dMax - dMin) * xChartLeng);
				img->DrawLine((DRAW_UNIT)(x + y1Leng), (DRAW_UNIT)yPos, (DRAW_UNIT)(x + width - y2Leng), (DRAW_UNIT)yPos, refLinePen);

				sptr = Text::StrDoubleFmt(sbuff, this->refDbl, dblFormat);
				if (this->yUnit)
					sptr = Text::StrConcat(sptr, this->yUnit);
				if (this->refType == RT_LEFTALIGN)
				{
					img->GetTextSize(fnt, sbuff, rcSize);
					img->DrawString((DRAW_UNIT)(x + y1Leng), yPos - rcSize[1], sbuff, fnt, fontBrush);
				}
				else if (this->refType == RT_RIGHTALIGN)
				{
					img->GetTextSize(fnt, sbuff, rcSize);
					img->DrawString((DRAW_UNIT)(x + width - y2Leng - rcSize[0]), yPos - rcSize[1], sbuff, fnt, fontBrush);
				}
			}
		}

		if (this->refTime != 0)
		{
			if (Data::LineChart::CT_DATETICK == yAxis1Type)
			{
				tMax = y1MaxDate;
				tMin = y1MinDate;
				if (this->refTime >= tMin && this->refTime <= tMax)
				{
					yPos = (Single)(y + height - xLeng - Data::DateTime::MS2Minutes(this->refTime - tMin) / Data::DateTime::MS2Minutes(tMax - tMin) * xChartLeng);
					img->DrawLine((DRAW_UNIT)(x + y1Leng), (DRAW_UNIT)yPos, (DRAW_UNIT)(x + width - y2Leng), (DRAW_UNIT)yPos, refLinePen);
				}
			}
			else if (Data::LineChart::CT_DATETICK == yAxis2Type)
			{
				tMax = y2MaxDate;
				tMin = y2MinDate;
				if (this->refTime >= tMin && this->refTime <= tMax)
				{
					yPos = (Single)(y + height - xLeng - Data::DateTime::MS2Minutes(this->refTime - tMin) / Data::DateTime::MS2Minutes(tMax - tMin) * xChartLeng);
					img->DrawLine((DRAW_UNIT)(x + y1Leng), (DRAW_UNIT)yPos, (DRAW_UNIT)(x + width - y2Leng), (DRAW_UNIT)yPos, refLinePen);
				}
			}
		}
	}

	DEL_CLASS(locations);
	DEL_CLASS(labels);
	img->DelFont(fnt);
	img->DelBrush(bgBrush);
	img->DelPen(boundPen);
	img->DelBrush(fontBrush);
	img->DelPen(gridPen);
	img->DelPen(refLinePen);

	if (customX)
	{
		i = this->xDatas->GetCount();
		while (i-- > 0)
		{
			MemFree(this->xDatas->GetItem(i));
		}
		this->xDatas->Clear();
		this->xDataCnt->Clear();
	}
}

UOSInt Data::LineChart::GetLegendCount()
{
	return this->yCharts->GetCount();
}

UTF8Char *Data::LineChart::GetLegend(UTF8Char *sbuff, UInt32 *color, UOSInt index)
{
	if (index >= this->yCharts->GetCount())
		return 0;
	Data::LineChart::ChartData *cdata = this->yCharts->GetItem(index);
	*color = cdata->lineColor;
	return Text::StrConcat(sbuff, dataNames->GetItem(index));
}

Data::LineChart::ChartData::ChartData(void *data, UOSInt dataCnt, Data::LineChart::ChartType dataType, UInt32 lineColor, Data::LineChart::LineStyle lineStyle)
{
	this->data = data;
	this->dataCnt = dataCnt;
	this->dataType = dataType;
	this->lineColor = lineColor;
	this->lineStyle = lineStyle;
}

Data::LineChart::ChartData::~ChartData()
{
	MemFree(data);
}
