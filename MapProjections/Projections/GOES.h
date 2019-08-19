#ifndef GOES_PROJECTION_H
#define GOES_PROJECTION_H

#include <cmath>

#include "../GeoCoordinate.h"
#include "../ProjectionInfo.h"
#include "../MapProjectionStructures.h"


namespace Projections
{
	/// <summary>
	/// Based on section 4.4:
	/// https://www.cgms-info.org/documents/cgms-lrit-hrit-global-specification-(v2-8-of-30-oct-2013).pdf
	/// </summary>
	class GOES : public ProjectionInfo<GOES>
	{
	public:
		static const bool INDEPENDENT_LAT_LON = false; //can Lat / Lon be computed separatly. To compute one, we dont need the other

		GOES(const Longitude & sateliteLon) : ProjectionInfo(PROJECTION::GOES_PROJ),
			satLon(sateliteLon)
		{}


		friend class ProjectionInfo<GOES>;

	protected:

		const Longitude satLon;
		const MyRealType SAT_DIST = MyRealType(42164.0);
		const MyRealType RADIUS_EQUATOR = MyRealType(6378.1370);
		const MyRealType RADIUS_POLAR = MyRealType(6356.7523);

		ProjectedValue ProjectInternal(const Coordinate & c) const
		{
			MyRealType cLat = std::atan(0.993305616 * std::tan(c.lat.rad()));
			MyRealType cosCLat = std::cos(cLat);

			MyRealType r = RADIUS_POLAR / std::sqrt(1 - 0.00669438444 * cosCLat * cosCLat);

			MyRealType r1 = SAT_DIST - r * cosCLat * std::cos(c.lon.rad() - satLon.rad());
			MyRealType r2 = -r * cosCLat * std::sin(c.lon.rad() - satLon.rad());
			MyRealType r3 = r * std::sin(cLat);
			MyRealType rn = std::sqrt(r1 * r1 + r2 * r2 + r3 * r3);

			ProjectedValue p;
			p.x = std::atan(-r2 / r1);
			p.y = std::asin(-r3 / rn);

			//?
			//p.x = std::atan(Angle::deg(-r2 / r1).rad()); //?
			//p.y = std::asin(Angle::deg(-r3 / rn).rad()); //?

			return p;
		};

		ProjectedValueInverse ProjectInverseInternal(MyRealType x, MyRealType y) const
		{		
			MyRealType cosX = std::cos(x);
			MyRealType cosY = std::cos(y);
			MyRealType cos2X = cosX * cosX;
			MyRealType cos2Y = cosY * cosY;

			MyRealType sinY = std::sin(y);
			MyRealType sinX = std::sin(x);
			MyRealType sin2Y = sinY * sinY;

			MyRealType tmp = SAT_DIST * cosX * cosY;
			MyRealType tmp2 = (cos2Y + 1.006739501 * sin2Y);

			MyRealType sd = std::sqrt(tmp * tmp - tmp2 * 1737122264);
			MyRealType sn = (SAT_DIST * cosX * cosY - sd) / tmp2;

			MyRealType s1 = SAT_DIST - sn * cosX * cosY;
			MyRealType s2 = sn * sinY * cosY;
			MyRealType s3 = -sn * sinY;
			MyRealType sxy = std::sqrt(s1 * s1 + s2 * s2);

			ProjectedValueInverse c;
			c.lon = Longitude::rad(std::atan(s2 / s1) + satLon.rad());
			c.lat = Latitude::rad(std::atan(1.006739501 * s3 / sxy));

			//?
			//c.lon = Longitude::rad(std::atan(Angle::deg(s2 / s1).rad()) + satLon.rad()); //?
			//c.lat = Latitude::rad(std::atan(Angle::deg(1.006739501 * s3 / sxy).rad())); //?

			return c;
		};
	};
}

#endif
