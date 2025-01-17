#ifndef _SM_MATH_VECTOR2D
#define _SM_MATH_VECTOR2D

namespace Math
{
	class CoordinateSystem;
	class Vector2D
	{
	public:
		typedef enum
		{
			VT_UNKNOWN,
			VT_POINT,
			VT_MULTIPOINT,
			VT_POLYLINE,
			VT_POLYGON,
			VT_IMAGE,
			VT_STRING,
			VT_ELLIPSE,
			VT_PIEAREA
		} VectorType;

	protected:
		UInt32 srid;
	public:
		Vector2D(UInt32 srid);
		virtual ~Vector2D();

		virtual VectorType GetVectorType() = 0;
		virtual void GetCenter(Double *x, Double *y) = 0;
		virtual Math::Vector2D *Clone() = 0;
		virtual void GetBounds(Double *minX, Double *minY, Double *maxX, Double *maxY) = 0;
		virtual Double CalSqrDistance(Double x, Double y, Double *nearPtX, Double *nearPtY) = 0;
		virtual Bool JoinVector(Math::Vector2D *vec) = 0;
		virtual Bool Support3D() { return false; };
		virtual void ConvCSys(Math::CoordinateSystem *srcCSys, Math::CoordinateSystem *destCSys) = 0;

		UInt32 GetSRID();
		void SetSRID(UInt32 srid);
	};
}
#endif
