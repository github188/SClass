#ifndef _SM_MATH_UNIT_SPEED
#define _SM_MATH_UNIT_SPEED
#include "Math/Unit/Temperature.h"

namespace Math
{
	namespace Unit
	{
		class Speed
		{
		public:
			typedef enum
			{
				SU_METER_PER_SECOND,
				SU_KM_PER_HOUR,
				SU_MILE_PER_HOUR,
				SU_KNOT
			} SpeedUnit;
		public:
			static Double GetUnitRatio(SpeedUnit unit);
			static const UTF8Char *GetUnitShortName(SpeedUnit unit);
			static const UTF8Char *GetUnitName(SpeedUnit unit);
			static Double Convert(SpeedUnit fromUnit, SpeedUnit toUnit, Double fromValue);
			static Double CalcDryAirSpeed(Double temperatureK, SpeedUnit unit);
		};
	}
}
#endif
