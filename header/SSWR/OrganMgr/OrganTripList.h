#ifndef _SM_SSWR_ORGANMGR_ORGANTRIPLIST
#define _SM_SSWR_ORGANMGR_ORGANTRIPLIST

#include "Data/DateTime.h"
#include "Data/ArrayList.h"
#include "DB/DBTool.h"

namespace SSWR
{
	namespace OrganMgr
	{
		class Trip
		{
		public:
			Int64 fromDate;
			Int64 toDate;
			Int32 locId;

		public:
			Trip(Data::DateTime *fromDate, Data::DateTime *toDate, Int32 locId);
			~Trip();
			UTF8Char *ToString(UTF8Char *sbuff);
		};

		class Location
		{
		public:
			Int32 id;
			Int32 parId;
			const UTF8Char *ename;
			const UTF8Char *cname;
			Double lat;
			Double lon;
			Int32 locType;

			Location(Int32 id, Int32 parId, const UTF8Char *ename, const UTF8Char *cname, Int32 locType);
			~Location();
			UTF8Char *ToString(UTF8Char *sbuff);
		};

		class LocationType
		{
		public:
			Int32 id;
			const UTF8Char *engName;
			const UTF8Char *chiName;

			LocationType(Int32 id, const UTF8Char *engName, const UTF8Char *chiName);
			~LocationType();
		};

/*		class OrganTripList
		{
		private:
			DB::DBTool *db;
			Int32 cateId;
			Data::ArrayList<Trip*> *trips;
			Data::ArrayList<Location*> *locs;
			Data::ArrayList<LocationType*> *locType;

			void FreeTrips();
		public:
			OrganTripList(DB::DBTool *db, Int32 cateId);
			~OrganTripList();
			void ReloadTrips(Int32 cateId);
			OSInt GetTripIndex(Data::DateTime *d);
			Trip *GetTrip(Int32 userId, Data::DateTime *d);
			Data::ArrayList<Trip*> *GetTripList();
			Bool AddTrip(Data::DateTime *fromDate, Data::DateTime *toDate, Int32 locId);
			OSInt GetLocIndex(Int32 locId);
			Location *GetLocation(Int32 locId);
			Data::ArrayList<Location*> *GetLocSub(Int32 locId);
			Bool UpdateLoc(Int32 locId, const UTF8Char *engName, const UTF8Char *chiName);
			Bool AddLoc(Int32 locId, const UTF8Char *engName, const UTF8Char *chiName);
			OSInt GetLocTypeIndex(Int32 lType);
		};*/
	}
}
#endif