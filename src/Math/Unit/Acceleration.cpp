#include "Stdafx.h"
#include "MyMemory.h"
#include "Math/Math.h"
#include "Math/Unit/Acceleration.h"

Double Math::Unit::Acceleration::GetUnitRatio(AccelerationUnit unit)
{
	switch (unit)
	{
	case AU_METER_PER_SECOND_SQUARED:
		return 1;
	case AU_STANDARD_GRAVITY:
		return 9.80665;
	}
	return 1;
}

const UTF8Char *Math::Unit::Acceleration::GetUnitShortName(AccelerationUnit unit)
{
	switch (unit)
	{
	case AU_METER_PER_SECOND_SQUARED:
		return (const UTF8Char*)"m/s^2";
	case AU_STANDARD_GRAVITY:
		return (const UTF8Char*)"g";
	}
	return (const UTF8Char*)"";
}

const UTF8Char *Math::Unit::Acceleration::GetUnitName(AccelerationUnit unit)
{
	switch (unit)
	{
	case AU_METER_PER_SECOND_SQUARED:
		return (const UTF8Char*)"Meter Per Second Squared";
	case AU_STANDARD_GRAVITY:
		return (const UTF8Char*)"Gravity";
	}
	return (const UTF8Char*)"";
}

Double Math::Unit::Acceleration::Convert(AccelerationUnit fromUnit, AccelerationUnit toUnit, Double fromValue)
{
	return fromValue * GetUnitRatio(fromUnit) / GetUnitRatio(toUnit);
}
