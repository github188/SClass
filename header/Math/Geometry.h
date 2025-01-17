#ifndef _SM_MATH_GEOMETRY
#define _SM_MATH_GEOMETRY
namespace Math
{
	class Geometry
	{
	public:
		static const Int32 RADIUS_METER_EARTH_WGS1984 = 6378137;
	public:
		static void RotateACW(Int32 *ptOut, Int32 *ptIn, UOSInt nPoint, Double centX, Double centY, Double angleRad);
		static void RotateACW(Double *ptOut, Double *ptIn, UOSInt nPoint, Double centX, Double centY, Double angleRad);
		static UOSInt BoundPolygonY(Int32 *points, UOSInt nPoints, Int32 *pointOut, OSInt minY, OSInt maxY, OSInt ofstX, OSInt ofstY);
		static UOSInt BoundPolygonX(Int32 *points, UOSInt nPoints, Int32 *pointOut, OSInt minX, OSInt maxX, OSInt ofstX, OSInt ofstY);
		static UOSInt BoundPolygonY(Double *points, UOSInt nPoints, Double *pointOut, Double minY, Double maxY, Double ofstX, Double ofstY);
		static UOSInt BoundPolygonX(Double *points, UOSInt nPoints, Double *pointOut, Double minX, Double maxX, Double ofstX, Double ofstY);
		static Bool InPolygon(Int32 *points, UOSInt nPoints, Int32 ptX, Int32 ptY); /////////////////////////
		static void PtNearPline(Int32 *points, UOSInt nPoints, OSInt ptX, OSInt ptY, Int32 *nearPtX, Int32 *nearPtY);
		static Double SphereDistDeg(Double lat1, Double lon1, Double lat2, Double lon2, Double radius);
		static void GetPolygonCenter(UOSInt nParts, UOSInt nPoints, UInt32 *parts, Int32 *points, Int32 *outPtX, Int32 *outPtY);
		static void GetPolygonCenter(UOSInt nParts, UOSInt nPoints, UInt32 *parts, Double *points, Double *outPtX, Double *outPtY);
	};
}
#endif
