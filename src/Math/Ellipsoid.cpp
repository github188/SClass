#include "Stdafx.h"
#include "Math/Math.h"
#include "Math/Ellipsoid.h"

Double Math::Ellipsoid::CalcPolarAxis(Double equator, Double flattening)
{
	return equator * (1 - 1 / flattening);
}

Math::Ellipsoid::Ellipsoid(Double radiusX, Double radiusY, Double radiusZ)
{
	this->radiusX = radiusX;
	this->radiusY = radiusY;
	this->radiusZ = radiusZ;
}

Math::Ellipsoid::Ellipsoid(Double equatorRadius, Double flattening)
{
	this->radiusX = equatorRadius;
	this->radiusY = equatorRadius;
	this->radiusZ = CalcPolarAxis(equatorRadius, flattening);
}

Math::Ellipsoid::~Ellipsoid()
{
}

Bool Math::Ellipsoid::GetX(Double y, Double z, Double *x1, Double *x2)
{
	Double tmp = 1 - (y * y) / (radiusY * radiusY) - (z * z) / (radiusZ * radiusZ);
	if (tmp < 0)
		return false;
	tmp = Math::Sqrt(tmp * radiusX * radiusX);
	*x1 = tmp;
	*x2 = -tmp;
	return true;
}

Bool Math::Ellipsoid::GetY(Double x, Double z, Double *y1, Double *y2)
{
	Double tmp = 1 - (x * x) / (radiusX * radiusX) - (z * z) / (radiusZ * radiusZ);
	if (tmp < 0)
		return false;
	tmp = Math::Sqrt(tmp * radiusY * radiusY);
	*y1 = tmp;
	*y2 = -tmp;
	return true;
}

Bool Math::Ellipsoid::GetZ(Double x, Double y, Double *z1, Double *z2)
{
	Double tmp = 1 - (x * x) / (radiusX * radiusX) - (y * y) / (radiusY * radiusY);
	if (tmp < 0)
		return false;
	tmp = Math::Sqrt(tmp * radiusZ * radiusZ);
	*z1 = tmp;
	*z2 = -tmp;
	return true;
}

Double Math::Ellipsoid::GetVolume()
{
	return 4 * Math::PI / 3 * radiusX * radiusY * radiusZ;
}

Double Math::Ellipsoid::GetSurfaceArea()
{
	Double modularAngle;
	if (radiusX > radiusZ)
	{
		modularAngle = Math::ArcCos(radiusZ / radiusX);
	}
	else
	{
		modularAngle = Math::ArcCos(radiusX / radiusZ);
	}

	if (radiusX == radiusY)
	{
		if (radiusX > radiusZ)
		{
			return 2 * Math::PI * (radiusX * radiusX + Math::ArcTanh(Math::Sin(modularAngle)) * radiusZ * radiusZ / Math::Sin(modularAngle));
		}
		else
		{
			return 2 * Math::PI * (radiusX * radiusX + modularAngle * radiusZ * radiusZ / Math::Tan(modularAngle));
		}
	}
	else
	{
		//approximate formula
		Double p = 1.6075;
		return Math::Pow((Math::Pow(radiusX * radiusY, p) + Math::Pow(radiusX * radiusZ, p) + Math::Pow(radiusY * radiusZ, p)) / 3, 1 / p) * 4 * Math::PI;
	}
}
