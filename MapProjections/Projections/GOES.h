#ifndef GOES_PROJECTION_H
#define GOES_PROJECTION_H

#include <cmath>
#include <tuple>

#include "../GeoCoordinate.h"
#include "../ProjectionInfo.h"
#include "../MapProjectionStructures.h"


namespace Projections
{
	/// <summary>
	/// Based on section 4.4:
	/// https://www.cgms-info.org/documents/cgms-lrit-hrit-global-specification-(v2-8-of-30-oct-2013).pdf
	/// 
	/// https://github.com/yaswant/ypylib/blob/master/geo.py
	/// </summary>
	class GOES : public ProjectionInfo<GOES>
	{
	public:
		static const bool INDEPENDENT_LAT_LON = false; //can Lat / Lon be computed separatly. To compute one, we dont need the other
		static const bool ORTHOGONAL_LAT_LON = false; //is lat / lon is orthogonal to each other

		GOES(const Longitude & sateliteLon) : ProjectionInfo(PROJECTION::GOES),
			satLon(sateliteLon)
		{}
	
		
		friend class ProjectionInfo<GOES>;

	protected:		
		const Longitude satLon;
		const MyRealType COFF_COEF = MyRealType(5500.5);
		const MyRealType LOFF_COEF = MyRealType(5500.5);
		const MyRealType LFAC_COEF = std::pow(2, -16) * 40'932'513.0;
		const MyRealType CFAC_COEF = -std::pow(2, -16) * 40'932'513.0;

		const MyRealType SAT_DIST = MyRealType(42164.0); //in km
		const MyRealType RADIUS_EQUATOR = MyRealType(6378.1370); //in km
		const MyRealType RADIUS_POLAR = MyRealType(6356.7523); //in km

		std::tuple<double, double, double, double>
			GetFrameBotLeftTopRight(const Coordinate & botLeft, const Coordinate & topRight)
		{
			return std::make_tuple(0, 0, 2 * COFF_COEF, 2 * LOFF_COEF);
		}

		ProjectedValue ProjectInternal(const Coordinate & c) const
		{
			//0.993305616 = RADIUS_POLAR^2 / RADIUS_EQUATOR^2
			//0.00669438444 = (RADIUS_EQUATOR^2 - RADIUS_POLAR^2) / RADIUS_EQUATOR^2 

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
									
			p.x = COFF_COEF + AngleUtils::radToDeg(p.x * LFAC_COEF);
			p.y = LOFF_COEF + AngleUtils::radToDeg(p.y * CFAC_COEF);

			return p;
		};

		ProjectedValueInverse ProjectInverseInternal(MyRealType x, MyRealType y) const
		{		
			//1.006739501 = RADIUS_EQUATOR^2 / RADIUS_POLAR^2
			//1737122264 = (SAT_DIST^2 - RADIUS_EQUATOR^2)

			x = AngleUtils::degToRad((x - COFF_COEF) / LFAC_COEF);
			y = AngleUtils::degToRad((y - LOFF_COEF) / CFAC_COEF);

			MyRealType cosX = std::cos(x);
			MyRealType cosY = std::cos(y);			
			MyRealType cos2Y = cosY * cosY;

			MyRealType sinX = std::sin(x);
			MyRealType sinY = std::sin(y);			
			MyRealType sin2Y = sinY * sinY;

			MyRealType tmp = SAT_DIST * cosX * cosY;
			MyRealType tmp2 = (cos2Y + 1.006739501 * sin2Y);

			MyRealType sd = std::sqrt(tmp * tmp - tmp2 * 1737122264);
			MyRealType sn = (tmp - sd) / tmp2;

			MyRealType s1 = SAT_DIST - sn * cosX * cosY;
			MyRealType s2 = sn * sinX * cosY;
			MyRealType s3 = -sn * sinY;
			MyRealType sxy = std::sqrt(s1 * s1 + s2 * s2);

			ProjectedValueInverse c;
			c.lon = Longitude::rad(std::atan(s2 / s1) + satLon.rad());
			c.lat = Latitude::rad(std::atan(1.006739501 * s3 / sxy));

			return c;
		};
	};
}

#endif
