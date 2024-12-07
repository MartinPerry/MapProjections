#ifndef GEOS_PROJECTION_H
#define GEOS_PROJECTION_H

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
	class GEOS : public ProjectionInfo<GEOS>
	{
	public:

		struct SatelliteSettings
		{
			//satellite longitude position 
			Longitude lon;

			//column / row offsets
			//(basically half of satellite image size)
			MyRealType coff;
			MyRealType loff;

			// Intermediate coords deltas
			MyRealType cfac;
			MyRealType lfac;

			bool sweepY = true;

			static SatelliteSettings Himawari8()
			{
				return {
					140.7_deg,
					MyRealType(5500.5),
					MyRealType(5500.5),
					MyRealType(40'932'513.0),
					MyRealType(-40'932'513.0)
				};
			};

			static SatelliteSettings Meteosat11()
			{
				return {
					0.0_deg,
					MyRealType(5566.0),
					MyRealType(5566.0),
					MyRealType(40'927'010.0),
					MyRealType(-40'927'010.0)
				};
			};

			static SatelliteSettings Meteosat8()
			{
				return {
					41.5_deg,
					MyRealType(5566.0),
					MyRealType(5566.0),
					MyRealType(40'927'010.0),
					MyRealType(-40'927'010.0)
				};
			};
			
			static SatelliteSettings Goes16()
			{
				//cfac calculated as:
				//(2.0 / AngleUtils::radToDeg(0.000056))) = (2^-16) * cfac,

				// !!!!!!
				// GOES has sweep axis x
				// !!!!!!

				//Visible area:
				//bbMin.lat = -81.3282_deg; bbMin.lon = -156.2995_deg;
				//bbMax.lat = 81.3282_deg; bbMax.lon = 6.2995_deg;

				return
				{
					-75.0_deg,
					MyRealType(5423.5),
					MyRealType(5423.5),
					MyRealType(40'850'678.0),
					MyRealType (-40'850'678.0),
					false
				};
			};

			static SatelliteSettings Goes17()
			{
				SatelliteSettings goes = SatelliteSettings::Goes16();
				goes.lon = -137.2_deg;
				return goes;
			};
		};


		static const bool INDEPENDENT_LAT_LON = false; //can Lat / Lon be computed separatly. To compute one, we dont need the other
		static const bool ORTHOGONAL_LAT_LON = false; //is lat / lon is orthogonal to each other


		GEOS(const SatelliteSettings & sets) :
			ProjectionInfo(PROJECTION::GEOS),
			sat(sets)
		{}

		GEOS(const GEOS& ge) :
			GEOS(ge.sat)
		{
			this->frame = ge.frame;
		}

		friend class ProjectionInfo<GEOS>;

	protected:
		const SatelliteSettings sat;
		
		const MyRealType SAT_DIST = MyRealType(42164.160); //Semi-major axis, in km
		const MyRealType RADIUS_EQUATOR = MyRealType(6378.1370); //in km
		const MyRealType RADIUS_POLAR = MyRealType(6356.7523); //in km
		const MyRealType TWO_POW_MINUS_16 = std::pow(2, -16);

		const char* GetNameInternal() const
		{
			return "GEOS";
		}

		std::tuple<double, double, double, double>
			GetFrameBotLeftTopRight(const Coordinate & botLeft, const Coordinate & topRight)
		{
			return std::make_tuple(0, 0, 2 * sat.coff, 2 * sat.loff);
		}

		ProjectedValue ProjectInternal(const Coordinate & c) const
		{
			//0.993305616 = RADIUS_POLAR^2 / RADIUS_EQUATOR^2
			//0.00669438444 = (RADIUS_EQUATOR^2 - RADIUS_POLAR^2) / RADIUS_EQUATOR^2 

			MyRealType lonDif = c.lon.rad() - sat.lon.rad();

			MyRealType cLat = std::atan(0.993305616 * std::tan(c.lat.rad()));
			MyRealType cosCLat = std::cos(cLat);

			MyRealType r = RADIUS_POLAR / std::sqrt(1 - 0.00669438444 * cosCLat * cosCLat);

			MyRealType r1 = SAT_DIST - r * cosCLat * std::cos(lonDif);
			MyRealType r2 = r * cosCLat * std::sin(lonDif);
			MyRealType r3 = r * std::sin(cLat);
			
			ProjectedValue p;
						
			if (sat.sweepY)
			{
				MyRealType rn = std::sqrt(r1 * r1 + r2 * r2 + r3 * r3);
				p.x = std::atan(r2 / r1);
				p.y = std::asin(-r3 / rn);
			}
			else
			{
				MyRealType rn = std::sqrt(r1 * r1 + r3 * r3);
				p.x = std::atan(r2 / rn);
				p.y = std::atan(-r3 / r1);
			}
			
			p.x = sat.coff + (AngleUtils::radToDeg(p.x) * TWO_POW_MINUS_16 * sat.cfac);
			p.y = sat.loff + (AngleUtils::radToDeg(p.y) * TWO_POW_MINUS_16 * sat.lfac);

			return p;
		};

		ProjectedValueInverse ProjectInverseInternal(MyRealType x, MyRealType y) const
		{			
			//1.006739501 = RADIUS_EQUATOR^2 / RADIUS_POLAR^2
			//1737122264 = (SAT_DIST^2 - RADIUS_EQUATOR^2)

			x = AngleUtils::degToRad((x - sat.coff) / (TWO_POW_MINUS_16 * sat.cfac));
			y = AngleUtils::degToRad((y - sat.loff) / (TWO_POW_MINUS_16 * sat.lfac));

			MyRealType cosX = std::cos(x);
			MyRealType cosY = std::cos(y);
			MyRealType cos2Y = cosY * cosY;

			MyRealType sinX = std::sin(x);
			MyRealType sinY = std::sin(y);
			MyRealType sin2Y = sinY * sinY;

			MyRealType tmp = SAT_DIST * cosX * cosY;
			MyRealType tmp2 = (cos2Y + 1.006739501 * sin2Y);

			MyRealType sd = std::sqrt(tmp * tmp - tmp2 * 1'737'122'264);
			MyRealType sn = (tmp - sd) / tmp2;

			MyRealType s1 = SAT_DIST - sn * cosX * cosY;
			MyRealType s2, s3;
			if (sat.sweepY)
			{
				s2 = sn * sinX * cosY;
				s3 = -sn * sinY;
			}
			else 
			{
				s2 = sn * sinX;
				s3 = -sn * sinY * cosX;
			}
			MyRealType sxy = std::sqrt(s1 * s1 + s2 * s2);

			return {
				Latitude::rad(std::atan(1.006739501 * s3 / sxy)),
				Longitude::rad(std::atan(s2 / s1) + sat.lon.rad())
			};			
		};
	};
}

#endif
