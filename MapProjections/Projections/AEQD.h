#ifndef AEQD_H
#define AEQD_H

#include <cmath>

#include "../GeoCoordinate.h"
#include "../ProjectionInfo.h"
#include "../MapProjectionStructures.h"


namespace Projections
{

	/// <summary>
	/// Based on:
	/// https://en.wikipedia.org/wiki/Azimuthal_equidistant_projection
	/// </summary>
	class AEQD : public ProjectionInfo<AEQD>
	{
	public:
		static const bool INDEPENDENT_LAT_LON = false; //can Lat / Lon be computed separatly. To compute one, we dont need the other
		static const bool ORTHOGONAL_LAT_LON = false; //is lat / lon is orthogonal to each other
		
		AEQD(const Longitude& centerLon, const Latitude& centerLat, MyRealType radius) :
			ProjectionInfo(PROJECTION::AEQD),
			centerLon(centerLon),
			centerLat(centerLat),
			radius(radius),
			sinCenterLat(std::sin(centerLat.rad())),
			cosCenterLat(std::cos(centerLat.rad()))
		{}

		AEQD(const AEQD& mi) : 
			AEQD(mi.centerLon, mi.centerLat, mi.radius)
		{
			this->frame = mi.frame;
		}


		void CalcBounds(Coordinate& min, Coordinate& max)
		{
			const MyRealType rad = radius / ProjectionConstants::EARTH_RADIUS;

			// latitude bounds
			MyRealType latMin = centerLat.rad() - rad;
			MyRealType latMax = centerLat.rad() + rad;
			if (latMin < -ProjectionConstants::PI_2)
			{
				latMin = -ProjectionConstants::PI_2;
			}
			if (latMax > ProjectionConstants::PI_2)
			{
				latMax = ProjectionConstants::PI_2;
			}

			// longitude bounds
			MyRealType lonMin, lonMax;
			if (latMin <= -ProjectionConstants::PI_2 || latMax >= ProjectionConstants::PI_2)
			{
				// circle includes a pole -> all longitudes
				lonMin = -ProjectionConstants::PI;
				lonMax = ProjectionConstants::PI;
			}
			else 
			{
				MyRealType dlon = std::asin(std::sin(rad) / cosCenterLat);
				lonMin = centerLon.rad() - dlon;
				lonMax = centerLon.rad() + dlon;
			}

			min.lat = Latitude::rad(latMin);
			min.lon = Longitude::rad(lonMin);

			max.lat = Latitude::rad(latMax);
			max.lon = Longitude::rad(lonMax);
		}


		friend class ProjectionInfo<AEQD>;

	protected:

		const Longitude centerLon;
		const Latitude centerLat;
		const MyRealType radius;

		const double sinCenterLat;
		const double cosCenterLat;
		
		const char* GetNameInternal() const
		{
			return "AEQD";
		}

		InternalBoundingBox GetInternalBoundingBox(const Coordinate& botLeft, const Coordinate& topRight) override
		{
			InternalBoundingBox bb;
			bb.min.x = -radius;
			bb.min.y = -radius;

			bb.max.x = radius;
			bb.max.y = radius;

			return bb;
		}

		ProjectedValue ProjectInternal(const Coordinate& c) const
		{
			auto cosLat = std::cos(c.lat.rad());
			auto sinLat = std::sin(c.lat.rad());
			auto cosDifLon = std::cos(c.lon.rad() - centerLon.rad());
			auto sinDifLon = std::sin(c.lon.rad() - centerLon.rad());

			
			auto cosPhiR = sinCenterLat * sinLat + cosCenterLat * cosLat * cosDifLon;

			auto phiR = std::acos(cosPhiR);
			auto phi = phiR * ProjectionConstants::EARTH_RADIUS;

			auto tmp0 = (cosLat * sinDifLon);
			auto tmp1 = (cosCenterLat * sinLat - sinCenterLat * cosLat * cosDifLon);
			auto tanDelta = tmp0 / tmp1;
			//auto delta = std::atan(tanDelta);
			auto delta = std::atan2(tmp0, tmp1);

			return {
				phi * std::sin(delta),
				phi * std::cos(delta)
			};
		};

		ProjectedValueInverse ProjectInverseInternal(MyRealType x, MyRealType y) const
		{
			
			//ChatGpt
			
			const double p = std::hypot(x, y);
			if (p < 1e-12) 
			{
				return { centerLat, centerLon};
			}

			const double c = p / ProjectionConstants::EARTH_RADIUS;
			const double sinc = std::sin(c);
			const double cosc = std::cos(c);
						
			double sinPhi = cosc * sinCenterLat + (y * sinc * cosCenterLat) / p;

			// Clamp for numeric safety
			if (sinPhi > 1.0) sinPhi = 1.0;
			if (sinPhi < -1.0) sinPhi = -1.0;

			const double lat = std::asin(sinPhi);

			const double numerator = x * sinc;
			const double denominator = p * cosCenterLat * cosc - y * sinCenterLat * sinc;
			double lon = centerLon.rad() + std::atan2(numerator, denominator);

			/*
			// Normalize longitude to [-pi, pi) if you want
			auto wrapPi = [](double a) {
				const double TWO_PI = 2.0 * ProjectionConstants::PI;
				a = std::fmod(a + ProjectionConstants::PI, 2 * ProjectionConstants::PI);
				if (a < 0) a += TWO_PI;
				return a - ProjectionConstants::PI;
				};
			lon = wrapPi(lon);
			*/

			return {
				Latitude::rad(lat),
				Longitude::rad(lon)
			};
		};

	};
}

#endif
