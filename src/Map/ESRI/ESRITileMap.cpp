#include "Stdafx.h"
#include "Text/MyString.h"
#include "Text/Encoding.h"
#include "Text/JSON.h"
#include "Math/Math.h"
#include "IO/FileStream.h"
#include "IO/MemoryStream.h"
#include "IO/Path.h"
#include "IO/StmData/FileData.h"
#include "Net/HTTPClient.h"
#include "Map/ESRI/ESRITileMap.h"

Map::ESRI::ESRITileMap::ESRITileMap(const UTF8Char *url, const UTF8Char *cacheDir, Net::SocketFactory *sockf, Net::SSLEngine *ssl)
{
	UTF8Char sbuff[512];
	UInt8 buff[2048];
	UOSInt readSize;
	UInt32 codePage;
	this->url = Text::StrCopyNew(url);
	this->cacheDir = Text::StrCopyNew(cacheDir);
	this->sockf = sockf;
	this->ssl = ssl;
	this->minX = 0;
	this->minY = 0;
	this->maxX = 0;
	this->maxY = 0;
	this->oriX = 0;
	this->oriY = 0;
	this->tileWidth = 0;
	this->tileHeight = 0;
	this->isMercatorProj = false;
	NEW_CLASS(this->levels, Data::ArrayListDbl());

	IO::MemoryStream *mstm;
	Text::StrConcat(Text::StrConcat(sbuff, url), (const UTF8Char*)"?f=json");
	Net::HTTPClient *cli = Net::HTTPClient::CreateConnect(sockf, ssl, sbuff, "GET", true);
	NEW_CLASS(mstm, IO::MemoryStream((const UTF8Char*)"Map.ESRI.ESRITileMap.ESRITileMap"));
	while ((readSize = cli->Read(buff, 2048)) > 0)
	{
		mstm->Write(buff, readSize);
	}
	codePage = cli->GetContentCodePage();
	DEL_CLASS(cli);


	UInt8 *jsonBuff = mstm->GetBuff(&readSize);
	if (jsonBuff && readSize > 0)
	{
		Text::Encoding enc(codePage);
		UOSInt charsCnt;
		UTF8Char *jsonStr;
		charsCnt = enc.CountUTF8Chars(jsonBuff, readSize);
		jsonStr = MemAlloc(UTF8Char, charsCnt + 1);
		enc.UTF8FromBytes(jsonStr, jsonBuff, readSize, 0);
		
		Text::JSONBase *json = Text::JSONBase::ParseJSONStr(jsonStr);
		if (json)
		{
			if (json->GetJSType() == Text::JSONBase::JST_OBJECT)
			{
				Text::JSONObject *jobj = (Text::JSONObject*)json;
				Text::JSONBase *o = jobj->GetObjectValue((const UTF8Char*)"initialExtent");
				Text::JSONBase *v;
				Text::JSONObject *vobj;
				Text::JSONBase *v2;
				if (o != 0 && o->GetJSType() == Text::JSONBase::JST_OBJECT)
				{
					Text::JSONObject *ext = (Text::JSONObject*)o;
					v = ext->GetObjectValue((const UTF8Char*)"xmin");
					if (v != 0 && v->GetJSType() == Text::JSONBase::JST_NUMBER)
						this->minX = ((Text::JSONNumber*)v)->GetValue();
					v = ext->GetObjectValue((const UTF8Char*)"ymin");
					if (v != 0 && v->GetJSType() == Text::JSONBase::JST_NUMBER)
						this->minY = ((Text::JSONNumber*)v)->GetValue();
					v = ext->GetObjectValue((const UTF8Char*)"xmax");
					if (v != 0 && v->GetJSType() == Text::JSONBase::JST_NUMBER)
						this->maxX = ((Text::JSONNumber*)v)->GetValue();
					v = ext->GetObjectValue((const UTF8Char*)"ymax");
					if (v != 0 && v->GetJSType() == Text::JSONBase::JST_NUMBER)
						this->maxY = ((Text::JSONNumber*)v)->GetValue();
				}
				else
				{
					o = jobj->GetObjectValue((const UTF8Char*)"fullExtent");
					if (o != 0 && o->GetJSType() == Text::JSONBase::JST_OBJECT)
					{
						Text::JSONObject *ext = (Text::JSONObject*)o;
						v = ext->GetObjectValue((const UTF8Char*)"xmin");
						if (v != 0 && v->GetJSType() == Text::JSONBase::JST_NUMBER)
							this->minX = ((Text::JSONNumber*)v)->GetValue();
						v = ext->GetObjectValue((const UTF8Char*)"ymin");
						if (v != 0 && v->GetJSType() == Text::JSONBase::JST_NUMBER)
							this->minY = ((Text::JSONNumber*)v)->GetValue();
						v = ext->GetObjectValue((const UTF8Char*)"xmax");
						if (v != 0 && v->GetJSType() == Text::JSONBase::JST_NUMBER)
							this->maxX = ((Text::JSONNumber*)v)->GetValue();
						v = ext->GetObjectValue((const UTF8Char*)"ymax");
						if (v != 0 && v->GetJSType() == Text::JSONBase::JST_NUMBER)
							this->maxY = ((Text::JSONNumber*)v)->GetValue();
					}
				}

				o = jobj->GetObjectValue((const UTF8Char*)"spatialReference");
				if (o != 0 && o->GetJSType() == Text::JSONBase::JST_OBJECT)
				{
					Text::JSONObject *spRef = (Text::JSONObject*)o;
					v = spRef->GetObjectValue((const UTF8Char*)"wkid");
					if (v != 0 && v->GetJSType() == Text::JSONBase::JST_NUMBER)
						this->isMercatorProj = ((Text::JSONNumber*)v)->GetValue() == 102100;
				}

				o = jobj->GetObjectValue((const UTF8Char*)"tileInfo");
				if (o != 0 && o->GetJSType() == Text::JSONBase::JST_OBJECT)
				{
					Text::JSONObject *tinfo = (Text::JSONObject*)o;
					v = tinfo->GetObjectValue((const UTF8Char*)"rows");
					if (v != 0 && v->GetJSType() == Text::JSONBase::JST_NUMBER)
						this->tileHeight = (UOSInt)Math::Double2Int32(((Text::JSONNumber*)v)->GetValue());
					v = tinfo->GetObjectValue((const UTF8Char*)"cols");
					if (v != 0 && v->GetJSType() == Text::JSONBase::JST_NUMBER)
						this->tileWidth = (UOSInt)Math::Double2Int32(((Text::JSONNumber*)v)->GetValue());
					v = tinfo->GetObjectValue((const UTF8Char*)"origin");
					if (v != 0 && v->GetJSType() == Text::JSONBase::JST_OBJECT)
					{
						Text::JSONObject *origin = (Text::JSONObject*)v;
						v = origin->GetObjectValue((const UTF8Char*)"x");
						if (v != 0 && v->GetJSType() == Text::JSONBase::JST_NUMBER)
							this->oriX = ((Text::JSONNumber*)v)->GetValue();
						v = origin->GetObjectValue((const UTF8Char*)"y");
						if (v != 0 && v->GetJSType() == Text::JSONBase::JST_NUMBER)
							this->oriY = ((Text::JSONNumber*)v)->GetValue();
					}
					v = tinfo->GetObjectValue((const UTF8Char*)"lods");
					if (v != 0 && v->GetJSType() == Text::JSONBase::JST_ARRAY)
					{
						Text::JSONArray *levs = (Text::JSONArray*)v;
						UOSInt i;
						UOSInt j;
						i = 0;
						j = levs->GetArrayLength();
						while (i < j)
						{
							v = levs->GetArrayValue(i);
							if (v != 0 && v->GetJSType() == Text::JSONBase::JST_OBJECT)
							{
								vobj = (Text::JSONObject*)v;
								v2 = vobj->GetObjectValue((const UTF8Char*)"resolution");
								if (v2 != 0 && v2->GetJSType() == Text::JSONBase::JST_NUMBER)
									this->levels->Add(((Text::JSONNumber*)v2)->GetValue());
							}
							i++;
						}
					}
				}
			}
		
			json->EndUse();
		}
		
		MemFree(jsonStr);
	}
	DEL_CLASS(mstm);
}

Map::ESRI::ESRITileMap::~ESRITileMap()
{
	if (this->url)
	{
		Text::StrDelNew(this->url);
	}
	if (this->cacheDir)
	{
		Text::StrDelNew(this->cacheDir);
	}
	DEL_CLASS(this->levels);
}

const UTF8Char *Map::ESRI::ESRITileMap::GetName()
{
	return (const UTF8Char*)"ESRITileMap";
}

Bool Map::ESRI::ESRITileMap::IsError()
{
	if (this->minX == this->maxX || this->minY == this->maxY)
		return true;
	if (this->tileWidth == 0 || this->tileHeight == 0)
		return true;
	return false;
}

Map::TileMap::TileType Map::ESRI::ESRITileMap::GetTileType()
{
	return Map::TileMap::TT_ESRI;
}

UOSInt Map::ESRI::ESRITileMap::GetLevelCount()
{
	return this->levels->GetCount();
}


Double Map::ESRI::ESRITileMap::GetLevelScale(UOSInt index)
{
	if (this->isMercatorProj)
	{
		return 204094080000.0 / Math::UOSInt2Double(this->tileWidth) / (1 << index);
	}
	else
	{
		Double level = this->levels->GetItem(index);
		if (level == 0)
			return 0;

		return level * 566928000.0;
	}
}

UOSInt Map::ESRI::ESRITileMap::GetNearestLevel(Double scale)
{
	if (this->isMercatorProj)
	{
		Int32 level = Math::Double2Int32(Math::Log10(204094080000.0 / scale / Math::UOSInt2Double(this->tileWidth)) / Math::Log10(2));
		if (level < 0)
			level = 0;
		else if (level >= (Int32)GetLevelCount())
			level = (Int32)GetLevelCount() - 1;
		return (UOSInt)level;
	}
	else
	{
		Double ldiff;
		Double minDiff;
		UOSInt minInd;
		UOSInt i;
		Double logResol = Math::Log10(scale / 566928000.0);
		minInd = 0;
		minDiff = 100000.0;
		i = this->levels->GetCount();
		while (i-- > 0)
		{
			ldiff = Math::Log10(this->levels->GetItem(i)) - logResol;
			if (ldiff < 0)
				ldiff = -ldiff;
			if (ldiff < minDiff)
			{
				minDiff = ldiff;
				minInd = i;
			}
		}
		return minInd;
	}
}

UOSInt Map::ESRI::ESRITileMap::GetConcurrentCount()
{
	return 2;
}

Bool Map::ESRI::ESRITileMap::GetBounds(Double *minX, Double *minY, Double *maxX, Double *maxY)
{
	if (this->isMercatorProj)
	{
		*minX = WebMercatorX2Lon(this->minX);
		*minY = WebMercatorY2Lat(this->minY);
		*maxX = WebMercatorX2Lon(this->maxX);
		*maxY = WebMercatorY2Lat(this->maxY);
	}
	else
	{
		*minX = this->minX;
		*minY = this->minY;
		*maxX = this->maxX;
		*maxY = this->maxY;
	}
	return this->minX != 0 || this->minY != 0 || this->maxX != 0 || this->maxY != 0;
}

Map::TileMap::ProjectionType Map::ESRI::ESRITileMap::GetProjectionType()
{
	if (this->isMercatorProj)
	{
		return Map::TileMap::PT_MERCATOR;
	}
	else
	{
		return Map::TileMap::PT_WGS84;
	}
}

UOSInt Map::ESRI::ESRITileMap::GetTileSize()
{
	return this->tileWidth;
}

UOSInt Map::ESRI::ESRITileMap::GetImageIDs(UOSInt level, Double x1, Double y1, Double x2, Double y2, Data::ArrayList<Int64> *ids)
{
	if (this->isMercatorProj)
	{
		Int32 i;
		Int32 j;
		Double max = 85.051128779806592377796715521925;
		if (y1 < -max)
			y1 = -max;
		else if (y1 > max)
			y1 = max;
		if (y2 < -max)
			y2 = -max;
		else if (y2 > max)
			y2 = max;
		
		if (x1 == x2)
			return 0;
		if (y1 == y2)
			return 0;
		Int32 pixX1 = Lon2TileX(x1, level);
		Int32 pixX2 = Lon2TileX(x2, level);
		Int32 pixY1 = Lat2TileY(y1, level);
		Int32 pixY2 = Lat2TileY(y2, level);
		if (pixX1 > pixX2)
		{
			i = pixX1;
			pixX1 = pixX2;
			pixX2 = i;
		}
		if (pixY1 > pixY2)
		{
			i = pixY1;
			pixY1 = pixY2;
			pixY2 = i;
		}
		if (pixX1 < 0)
			pixX1 = 0;
		if (pixY1 < 0)
			pixY1 = 0;
		if (pixX2 < 0)
			pixX2 = 0;
		if (pixY2 < 0)
			pixY2 = 0;
		if (pixX2 >= (1 << level))
		{
			pixX2 = (1 << level) - 1;
		}
		i = pixY1;
		while (i <= pixY2)
		{
			j = pixX1;
			while (j <= pixX2)
			{
				ids->Add((((Int64)(UInt32)j) << 32) | (UInt32)i);
				j++;
			}
			i++;
		}
		return (UOSInt)((pixX2 - pixX1 + 1) * (pixY2 - pixY1 + 1));
	}
	else
	{
		Double resol = this->levels->GetItem(level);
		Int32 i;
		Int32 j;
		if (resol == 0)
			return 0;
		if (x1 > this->maxX)
			x1 = this->maxX;
		if (x1 < this->minX)
			x1 = this->minX;
		if (x2 > this->maxX)
			x2 = this->maxX;
		if (x2 < this->minX)
			x2 = this->minX;
		if (y1 > this->maxY)
			y1 = this->maxY;
		if (y1 < this->minY)
			y1 = this->minY;
		if (y2 > this->maxY)
			y2 = this->maxY;
		if (y2 < this->minY)
			y2 = this->minY;

		if (x1 == x2)
			return 0;
		if (y1 == y2)
			return 0;
		Int32 pixX1 = (Int32)((x1 - this->oriX) / resol / Math::UOSInt2Double(this->tileWidth));
		Int32 pixX2 = (Int32)((x2 - this->oriX) / resol / Math::UOSInt2Double(this->tileWidth));
		Int32 pixY1 = (Int32)((this->oriY - y1) / resol / Math::UOSInt2Double(this->tileHeight));
		Int32 pixY2 = (Int32)((this->oriY - y2) / resol / Math::UOSInt2Double(this->tileHeight));
		if (pixX1 > pixX2)
		{
			i = pixX1;
			pixX1 = pixX2;
			pixX2 = i;
		}
		if (pixY1 > pixY2)
		{
			i = pixY1;
			pixY1 = pixY2;
			pixY2 = i;
		}
		i = pixY1;
		while (i <= pixY2)
		{
			j = pixX1;
			while (j <= pixX2)
			{
				ids->Add((((Int64)(UInt32)j) << 32) | (UInt32)i);
				j++;
			}
			i++;
		}
		return (UOSInt)((pixX2 - pixX1 + 1) * (pixY2 - pixY1 + 1));
	}
}

Media::ImageList *Map::ESRI::ESRITileMap::LoadTileImage(UOSInt level, Int64 imgId, Parser::ParserList *parsers, Double *boundsXY, Bool localOnly)
{
	UInt8 dataBuff[2048];
	UOSInt readSize;
	UTF8Char filePath[512];
	UTF8Char url[512];
	UTF8Char *sptr;
	Net::HTTPClient *cli;
	IO::FileStream *fs;
	IO::StmData::FileData *fd;
	IO::ParsedObject *pobj;
	Int32 imgX = (Int32)(imgId >> 32);
	Int32 imgY = (Int32)(imgId & 0xffffffffLL);
	if (this->isMercatorProj)
	{
		if (level < 0 || level >= this->levels->GetCount())
			return 0;
		Double x1 = TileX2Lon(imgX, level);
		Double y1 = TileY2Lat(imgY, level);
		Double x2 = TileX2Lon(imgX + 1, level);
		Double y2 = TileY2Lat(imgY + 1, level);

		boundsXY[0] = x1;
		boundsXY[1] = y1;
		boundsXY[2] = x2;
		boundsXY[3] = y2;
		if (x1 > 180 || y1 < -90)
			return 0;
	}
	else
	{
		Double resol = this->levels->GetItem(level);
		if (resol == 0)
			return 0;
		Double x1 = imgX * Math::UOSInt2Double(this->tileWidth) * resol + this->oriX;
		Double y1 = this->oriY - imgY * Math::UOSInt2Double(this->tileHeight) * resol;
		Double x2 = x1 + Math::UOSInt2Double(this->tileWidth) * resol;
		Double y2 = y1 - Math::UOSInt2Double(this->tileHeight) * resol;

		if (x1 > this->maxX || x2 < this->minX || y1 < minY || y2 > maxY)
			return 0;

		boundsXY[0] = x1;
		boundsXY[1] = y1;
		boundsXY[2] = x2;
		boundsXY[3] = y2;
	}

	sptr = Text::StrConcat(filePath, this->cacheDir);
	if (sptr[-1] != IO::Path::PATH_SEPERATOR)
		*sptr++ = IO::Path::PATH_SEPERATOR;
	sptr = Text::StrInt32(sptr, (Int32)level);
	sptr = Text::StrConcat(sptr, (const UTF8Char*)"\\");
	sptr = Text::StrInt32(sptr, imgY);
	IO::Path::CreateDirectory(filePath);
	sptr = Text::StrConcat(sptr, (const UTF8Char*)"\\");
	sptr = Text::StrInt32(sptr, imgX);
	sptr = Text::StrConcat(sptr, (const UTF8Char*)".dat");
	NEW_CLASS(fd, IO::StmData::FileData(filePath, false));
	if (fd->GetDataSize() > 0)
	{
		IO::ParsedObject::ParserType pt;
		pobj = parsers->ParseFile(fd, &pt);
		if (pobj)
		{
			if (pt == IO::ParsedObject::PT_IMAGE_LIST_PARSER)
			{
				DEL_CLASS(fd);
				return (Media::ImageList*)pobj;
			}
			DEL_CLASS(pobj);
		}
	}
	DEL_CLASS(fd);

	if (localOnly)
		return 0;

	sptr = Text::StrConcat(url, this->url);
	sptr = Text::StrConcat(sptr, (const UTF8Char*)"/tile/");
	sptr = Text::StrInt32(sptr, (Int32)level);
	sptr = Text::StrConcat(sptr, (const UTF8Char*)"/");
	sptr = Text::StrInt32(sptr, imgY);
	sptr = Text::StrConcat(sptr, (const UTF8Char*)"/");
	sptr = Text::StrInt32(sptr, imgX);

	cli = Net::HTTPClient::CreateConnect(this->sockf, this->ssl, url, "GET", true);
	NEW_CLASS(fs, IO::FileStream(filePath, IO::FileStream::FILE_MODE_CREATE, IO::FileStream::FILE_SHARE_DENY_NONE, IO::FileStream::BT_NORMAL));
	while ((readSize = cli->Read(dataBuff, 2048)) > 0)
	{
		fs->Write(dataBuff, readSize);
	}
	DEL_CLASS(cli);
	DEL_CLASS(fs);

	NEW_CLASS(fd, IO::StmData::FileData(filePath, false));
	if (fd->GetDataSize() > 0)
	{
		IO::ParsedObject::ParserType pt;
		pobj = parsers->ParseFile(fd, &pt);
		if (pobj)
		{
			if (pt == IO::ParsedObject::PT_IMAGE_LIST_PARSER)
			{
				DEL_CLASS(fd);
				return (Media::ImageList*)pobj;
			}
			DEL_CLASS(pobj);
		}
	}
	DEL_CLASS(fd);
	return 0;
}

UTF8Char *Map::ESRI::ESRITileMap::GetImageURL(UTF8Char *sbuff, UOSInt level, Int64 imgId)
{
	UTF8Char *sptr;
	Int32 imgX = (Int32)(imgId >> 32);
	Int32 imgY = (Int32)(imgId & 0xffffffffLL);
	sptr = Text::StrConcat(sbuff, this->url);
	sptr = Text::StrConcat(sptr, (const UTF8Char*)"/tile/");
	sptr = Text::StrUOSInt(sptr, level);
	sptr = Text::StrConcat(sptr, (const UTF8Char*)"/");
	sptr = Text::StrInt32(sptr, imgY);
	sptr = Text::StrConcat(sptr, (const UTF8Char*)"/");
	sptr = Text::StrInt32(sptr, imgX);
	return sptr;
}

IO::IStreamData *Map::ESRI::ESRITileMap::LoadTileImageData(UOSInt level, Int64 imgId, Double *boundsXY, Bool localOnly, Int32 *blockX, Int32 *blockY, ImageType *it)
{
	UInt8 dataBuff[2048];
	UOSInt readSize;
	UTF8Char filePath[512];
	UTF8Char url[512];
	UTF8Char *u8ptr;
	Net::HTTPClient *cli;
	IO::FileStream *fs;
	IO::StmData::FileData *fd;
	Int32 imgX = (Int32)(imgId >> 32);
	Int32 imgY = (Int32)(imgId & 0xffffffffLL);
	if (this->isMercatorProj)
	{
		if (level < 0 || level >= this->levels->GetCount())
			return 0;
		Double x1 = TileX2Lon(imgX, level);
		Double y1 = TileY2Lat(imgY, level);
		Double x2 = TileX2Lon(imgX + 1, level);
		Double y2 = TileY2Lat(imgY + 1, level);

		boundsXY[0] = x1;
		boundsXY[1] = y1;
		boundsXY[2] = x2;
		boundsXY[3] = y2;
		if (x1 > 180 || y1 < -90)
			return 0;
	}
	else
	{
		Double resol = this->levels->GetItem(level);
		if (resol == 0)
			return 0;
		Double x1 = imgX * Math::UOSInt2Double(this->tileWidth) * resol + this->oriX;
		Double y1 = this->oriY - imgY * Math::UOSInt2Double(this->tileHeight) * resol;
		Double x2 = x1 + Math::UOSInt2Double(this->tileWidth) * resol;
		Double y2 = y1 - Math::UOSInt2Double(this->tileHeight) * resol;

		if (x1 > this->maxX || x2 < this->minX || y1 < minY || y2 > maxY)
			return 0;

		boundsXY[0] = x1;
		boundsXY[1] = y1;
		boundsXY[2] = x2;
		boundsXY[3] = y2;
	}

	u8ptr = Text::StrConcat(filePath, this->cacheDir);
	if (u8ptr[-1] != IO::Path::PATH_SEPERATOR)
		*u8ptr++ = IO::Path::PATH_SEPERATOR;
	u8ptr = Text::StrUOSInt(u8ptr, level);
	*u8ptr++ = IO::Path::PATH_SEPERATOR;
	u8ptr = Text::StrInt32(u8ptr, imgY);
	IO::Path::CreateDirectory(filePath);
	*u8ptr++ = IO::Path::PATH_SEPERATOR;
	u8ptr = Text::StrInt32(u8ptr, imgX);
	u8ptr = Text::StrConcat(u8ptr, (const UTF8Char*)".dat");
	NEW_CLASS(fd, IO::StmData::FileData(filePath, false));
	if (fd->GetDataSize() > 0)
	{
		if (blockX)
			*blockX = imgX;
		if (blockY)
			*blockY = imgY;
		if (it)
			*it = IT_PNG;
		return fd;
	}
	DEL_CLASS(fd);

	if (localOnly)
		return 0;

	u8ptr = Text::StrConcat(url, this->url);
	u8ptr = Text::StrConcat(u8ptr, (const UTF8Char*)"/tile/");
	u8ptr = Text::StrInt32(u8ptr, (Int32)level);
	u8ptr = Text::StrConcat(u8ptr, (const UTF8Char*)"/");
	u8ptr = Text::StrInt32(u8ptr, imgY);
	u8ptr = Text::StrConcat(u8ptr, (const UTF8Char*)"/");
	u8ptr = Text::StrInt32(u8ptr, imgX);

	cli = Net::HTTPClient::CreateConnect(this->sockf, this->ssl, url, "GET", true);
	NEW_CLASS(fs, IO::FileStream(filePath, IO::FileStream::FILE_MODE_CREATE, IO::FileStream::FILE_SHARE_DENY_NONE, IO::FileStream::BT_NORMAL));
	while ((readSize = cli->Read(dataBuff, 2048)) > 0)
	{
		fs->Write(dataBuff, readSize);
	}
	DEL_CLASS(cli);
	DEL_CLASS(fs);

	NEW_CLASS(fd, IO::StmData::FileData(filePath, false));
	if (fd->GetDataSize() > 0)
	{
		if (blockX)
			*blockX = imgX;
		if (blockY)
			*blockY = imgY;
		if (it)
			*it = IT_PNG;
		return fd;
	}
	DEL_CLASS(fd);
	return 0;
}

Int32 Map::ESRI::ESRITileMap::Lon2TileX(Double lon, UOSInt level)
{
	return (Int32)((lon + 180.0) / 360.0 * (1 << level)); 
}

Int32 Map::ESRI::ESRITileMap::Lat2TileY(Double lat, UOSInt level)
{
	return (Int32)((1.0 - Math::Ln( Math::Tan(lat * Math::PI / 180.0) + 1.0 / Math::Cos(lat * Math::PI / 180.0)) / Math::PI) / 2.0 * (1 << level));
}

Double Map::ESRI::ESRITileMap::TileX2Lon(Int32 x, UOSInt level)
{
	return x * 360.0 / (1 << level) - 180;
}

Double Map::ESRI::ESRITileMap::TileY2Lat(Int32 y, UOSInt level)
{
	Double n = Math::PI - 2.0 * Math::PI * y / (1 << level);
	return 180.0 / Math::PI * Math::ArcTan(0.5 * (Math::Exp(n) - Math::Exp(-n)));
}

Double Map::ESRI::ESRITileMap::WebMercatorX2Lon(Double x)
{
	x = x / 6378137.0;
	Double w2 = x * 57.295779513082323;\
	Double w3 = Math::Fix((x + 180.0) / 360.0);
	return w2 - (w3 * 360.0);
}

Double Map::ESRI::ESRITileMap::WebMercatorY2Lat(Double y)
{
	return (1.5707963267948966 - (2.0 * Math::ArcTan(Math::Exp((-1.0 * y) / 6378137.0)))) * 57.295779513082323;
}

Double Map::ESRI::ESRITileMap::Lon2WebMercatorX(Double lon)
{
	return 6378137.0 * lon * 0.017453292519943295;
}

Double Map::ESRI::ESRITileMap::Lat2WebMercatorY(Double lat)
{
	Double a = lat * 0.017453292519943295;
	return 3189068.5 * Math::Log10((1.0 + Math::Sin(a)) / (1.0 - Math::Sin(a)));
}
