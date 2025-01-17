#include "Stdafx.h"
#include "MyMemory.h"
#include "Math/Unit/ApparentPower.h"

Double Math::Unit::ApparentPower::GetUnitRatio(ApparentPowerUnit unit)
{
	switch (unit)
	{
	case APU_VOLTAMPERE:
		return 1.0;
	}
	return 1;
}

const UTF8Char *Math::Unit::ApparentPower::GetUnitShortName(ApparentPowerUnit unit)
{
	switch (unit)
	{
	case APU_VOLTAMPERE:
		return (const UTF8Char*)"VA";
	}
	return (const UTF8Char*)"";
}

const UTF8Char *Math::Unit::ApparentPower::GetUnitName(ApparentPowerUnit unit)
{
	switch (unit)
	{
	case APU_VOLTAMPERE:
		return (const UTF8Char*)"Volt-Ampere";
	}
	return (const UTF8Char*)"";
}

Double Math::Unit::ApparentPower::Convert(ApparentPowerUnit fromUnit, ApparentPowerUnit toUnit, Double fromValue)
{
	return fromValue * GetUnitRatio(fromUnit) / GetUnitRatio(toUnit);
}
