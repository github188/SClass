#ifndef _SM_MATH_POLYLINE
#define _SM_MATH_POLYLINE
#include "Math/Polygon.h"


namespace Math
{
	class Polyline : public Math::PointCollection
	{
	protected:
		Double *pointArr;
		UOSInt nPoint;
		UInt32 *ptOfstArr;
		UOSInt nPtOfst;
		Int32 flags;
		UInt32 color;
	public:
		Polyline(UInt32 srid, Double *pointArr, UOSInt nPoint);
		Polyline(UInt32 srid, UOSInt nPtOfst, UOSInt nPoint);
		virtual ~Polyline();

		virtual VectorType GetVectorType();
		virtual UInt32 *GetPtOfstList(UOSInt *nPtOfst);
		virtual Double *GetPointList(UOSInt *nPoint);
		virtual void GetCenter(Double *x, Double *y);
		virtual Math::Vector2D *Clone();
		virtual void GetBounds(Double *minX, Double *minY, Double *maxX, Double *maxY);
		virtual Double CalSqrDistance(Double x, Double y, Double *nearPtX, Double *nearPtY);
		virtual Bool JoinVector(Math::Vector2D *vec);
		virtual void ConvCSys(Math::CoordinateSystem *srcCSys, Math::CoordinateSystem *destCSys);

		virtual Math::Polyline *SplitByPoint(Double x, Double y);
		virtual void OptimizePolyline();
		OSInt GetPointNo(Double x, Double y, Bool *isPoint, Double *calPtX, Double *calPtY);

		Math::Polygon *CreatePolygonByDist(Double dist);
		Bool HasColor();
		UInt32 GetColor();
		void SetColor(UInt32 color);
	};
}
#endif
