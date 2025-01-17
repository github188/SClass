#ifndef _SM_MAP_GPSTRACK
#define _SM_MAP_GPSTRACK
#include "Data/ArrayListInt32.h"
#include "Data/ArrayListStrUTF8.h"
#include "IO/ParsedObject.h"
#include "Map/IMapDrawLayer.h"
#include "Sync/Mutex.h"

namespace Map
{
	class GPSTrack : public IMapDrawLayer
	{
	public:
		typedef struct
		{
			Int64 utcTimeTicks;
			Double lat;
			Double lon;
			Double altitude;
			Double speed;
			Double heading;
			Int32 valid;
			Int32 nSateUsed;
			Int32 nSateView;
		} GPSRecord;

		typedef struct
		{
			Double maxLat;
			Double maxLon;
			Double minLat;
			Double minLon;
			const UTF8Char *name;
			UOSInt nRecords;
			GPSRecord *records;
			const UInt8 **extraData;
			UOSInt *extraDataSize;
		} TrackRecord;

		class GPSExtraParser
		{
		public:
			virtual ~GPSExtraParser() {};
			
			virtual UOSInt GetExtraCount(const UInt8 *buff, UOSInt buffSize) = 0;
			virtual Bool GetExtraName(const UInt8 *buff, UOSInt buffSize, UOSInt extIndex, Text::StringBuilderUTF *sb) = 0;
			virtual Bool GetExtraValueStr(const UInt8 *buff, UOSInt buffSize, UOSInt extIndex, Text::StringBuilderUTF *sb) = 0;
		};
	private:
		UInt32 codePage;
		Bool hasAltitude;
		Double currMaxLat;
		Double currMaxLon;
		Double currMinLat;
		Double currMinLon;
		Double maxLat;
		Double maxLon;
		Double minLat;
		Double minLon;
		const UTF8Char *currTrackName;
		Sync::Mutex *recMut;
		Data::ArrayListInt64 *currTimes;
		Data::ArrayList<GPSRecord*> *currRecs;
		Data::ArrayList<const UInt8 *> *currExtraData;
		Data::ArrayList<UOSInt> *currExtraSize;
		Data::ArrayList<TrackRecord*> *currTracks;
		Map::GPSTrack::GPSRecord *tmpRecord;
		GPSExtraParser *extraParser;

		Sync::Mutex *updMut;
		Data::ArrayList<Map::IMapDrawLayer::UpdatedHandler> *updHdlrs;
		Data::ArrayList<void *> *updObjs;

	public:
		GPSTrack(const UTF8Char *sourceName, Bool hasAltitude, UInt32 codePage, const UTF8Char *layerName);
		virtual ~GPSTrack();

		virtual DrawLayerType GetLayerType();
		virtual UOSInt GetAllObjectIds(Data::ArrayListInt64 *outArr, void **nameArr);
		virtual UOSInt GetObjectIds(Data::ArrayListInt64 *outArr, void **nameArr, Double mapRate, Int32 x1, Int32 y1, Int32 x2, Int32 y2, Bool keepEmpty);
		virtual UOSInt GetObjectIdsMapXY(Data::ArrayListInt64 *outArr, void **nameArr, Double x1, Double y1, Double x2, Double y2, Bool keepEmpty);
		virtual Int64 GetObjectIdMax();
		virtual void ReleaseNameArr(void *nameArr);
		virtual UTF8Char *GetString(UTF8Char *buff, UOSInt buffSize, void *nameArr, Int64 id, UOSInt strIndex);
		virtual UOSInt GetColumnCnt();
		virtual UTF8Char *GetColumnName(UTF8Char *buff, UOSInt colIndex);
		virtual DB::DBUtil::ColType GetColumnType(UOSInt colIndex, UOSInt *colSize);
		virtual Bool GetColumnDef(UOSInt colIndex, DB::ColDef *colDef);
		virtual UInt32 GetCodePage();
		virtual Bool GetBoundsDbl(Double *minX, Double *minY, Double *maxX, Double *maxY);

		virtual void *BeginGetObject();
		virtual void EndGetObject(void *session);
		virtual DrawObjectL *GetObjectByIdD(void *session, Int64 id);
		virtual Math::Vector2D *GetVectorById(void *session, Int64 id);
		virtual void ReleaseObject(void *session, DrawObjectL *obj);
		virtual void AddUpdatedHandler(UpdatedHandler hdlr, void *obj);
		virtual void RemoveUpdatedHandler(UpdatedHandler hdlr, void *obj);

		virtual UOSInt GetTableNames(Data::ArrayList<const UTF8Char*> *names); // no need to release
		virtual DB::DBReader *GetTableData(const UTF8Char *name, UOSInt maxCnt, void *ordering, void *condition);
		virtual ObjectClass GetObjectClass();

		void NewTrack();
		UOSInt AddRecord(GPSRecord *rec);
		Bool RemoveRecordRange(UOSInt index, UOSInt recStart, UOSInt recEnd);
		Bool GetHasAltitude();
		void SetTrackName(const UTF8Char *name);
		void GetTrackNames(Data::ArrayListStrUTF8 *nameArr);
		const UTF8Char *GetTrackName(UOSInt index);
		Bool GetTrackStartTime(UOSInt index, Data::DateTime *dt);
		Bool GetTrackEndTime(UOSInt index, Data::DateTime *dt);

		UOSInt GetTrackCnt();
		GPSRecord *GetTrack(UOSInt index, UOSInt *recordCnt);
		void GetLatLonByTime(Data::DateTime *dt, Double *lat, Double *lon);
		void GetLatLonByTicks(Int64 tiemTicks, Double *lat, Double *lon);

		void SetExtraParser(GPSExtraParser *parser);
		void SetExtraDataIndex(UOSInt recIndex, const UInt8 *data, UOSInt dataSize);
		const UInt8 *GetExtraData(UOSInt trackIndex, UOSInt recIndex, UOSInt *dataSize);
		UOSInt GetExtraCount(UOSInt trackIndex, UOSInt recIndex);
		Bool GetExtraName(UOSInt trackIndex, UOSInt recIndex, UOSInt extIndex, Text::StringBuilderUTF *sb);
		Bool GetExtraValueStr(UOSInt trackIndex, UOSInt recIndex, UOSInt extIndex, Text::StringBuilderUTF *sb);
	};

	class GPSTrackReader : public Map::MapLayerReader
	{
	private:
		Map::GPSTrack *gps;
	public:
		GPSTrackReader(Map::GPSTrack *gps);
		virtual ~GPSTrackReader();

		virtual DateErrType GetDate(UOSInt colIndex, Data::DateTime *outVal); //return 0 = ok, 2 = Error
	};

	class GPSDataReader : public DB::DBReader
	{
	private:
		Map::GPSTrack *gps;
		OSInt currRow;
		GPSTrack::GPSRecord *currRec;
	public:
		GPSDataReader(Map::GPSTrack *gps);
		virtual ~GPSDataReader();

		virtual Bool ReadNext();
		virtual UOSInt ColCount();
		virtual OSInt GetRowChanged();

		virtual Int32 GetInt32(UOSInt colIndex);
		virtual Int64 GetInt64(UOSInt colIndex);
		virtual WChar *GetStr(UOSInt colIndex, WChar *buff);
		virtual Bool GetStr(UOSInt colIndex, Text::StringBuilderUTF *sb);
		virtual const UTF8Char *GetNewStr(UOSInt colIndex);
		virtual UTF8Char *GetStr(UOSInt colIndex, UTF8Char *buff, UOSInt buffSize);
		virtual DateErrType GetDate(UOSInt colIndex, Data::DateTime *outVal);
		virtual Double GetDbl(UOSInt colIndex);
		virtual Bool GetBool(UOSInt colIndex);
		virtual UOSInt GetBinarySize(UOSInt colIndex);
		virtual UOSInt GetBinary(UOSInt colIndex, UInt8 *buff);
		virtual Math::Vector2D *GetVector(UOSInt colIndex);

		virtual UTF8Char *GetName(UOSInt colIndex, UTF8Char *buff);
		virtual Bool IsNull(UOSInt colIndex);
		virtual DB::DBUtil::ColType GetColType(UOSInt colIndex, UOSInt *colSize);
		virtual Bool GetColDef(UOSInt colIndex, DB::ColDef *colDef);
		virtual void DelNewStr(const UTF8Char *s);

		const UTF8Char *GetName(UOSInt colIndex);
	};
}
#endif
