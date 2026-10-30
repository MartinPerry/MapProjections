#ifndef AZIMUTHAL_EQUIDISTANT_H
#define AZIMUTHAL_EQUIDISTANT_H

#include <cmath>

#include "../GeoCoordinate.h"
#include "../ProjectionInfo.h"
#include "../MapProjectionStructures.h"


namespace Projections
{

	/// <summary>
	/// Based on:
	/// https://mathworld.wolfram.com/AzimuthalEquidistantProjection.html
	/// 
	/// </summary>
	class AzimuthalEquidistant : public ProjectionInfo<AzimuthalEquidistant>
	{
	public:
		static const bool INDEPENDENT_LAT_LON = false; //can Lat / Lon be computed separatly. To compute one, we dont need the other
		static const bool ORTHOGONAL_LAT_LON = false; //is lat / lon is orthogonal to each other

		AzimuthalEquidistant(const Longitude& centralLon, const Latitude& stanParallel) :
			ProjectionInfo(PROJECTION::AZIMUTHAL_EQUIDISTANT),
			centralLon(centralLon),
			stanParallel(stanParallel),
			sinStanParallel(std::sin(stanParallel.rad())),
			cosStanParallel(std::cos(stanParallel.rad()))
		{
		}

		AzimuthalEquidistant(const AzimuthalEquidistant& lc) :
			AzimuthalEquidistant(lc.centralLon, lc.stanParallel)
		{
			this->frame = lc.frame;
		}


		friend class ProjectionInfo<AzimuthalEquidistant>;

	protected:
		const Longitude centralLon;
		const Latitude stanParallel;

		const double sinStanParallel;
		const double cosStanParallel;


		const char* GetNameInternal() const
		{
			return "AzimuthalEquidistant";
		}

		ProjectedValue ProjectInternal(const Coordinate& c) const
		{
			double sinLat = std::sin(c.lat.rad());
			double cosLat = std::cos(c.lat.rad());
			double cosLonDif = std::cos(c.lon.rad() - centralLon.rad());

			double tmp0 = sinStanParallel * sinLat;
			double tmp1 = cosStanParallel * cosLat * cosLonDif;
			
			double cc = std::acos(sinStanParallel* sinLat + cosStanParallel * cosLat * cosLonDif);

			double k = cc / std::sin(cc);

			MyRealType x = k * cosLat * std::sin(c.lon.rad() - centralLon.rad());
			MyRealType y = k * (cosStanParallel * sinLat - sinStanParallel * cosLat * cosLonDif);

			return { x, y };
		};

		ProjectedValueInverse ProjectInverseInternal(MyRealType x, MyRealType y) const
		{
			//double c = std::sqrt(x * x + y * y);
			double c = std::hypot(x, y);

			
			double cosC = std::cos(c);
			double sinC = std::sin(c);


			MyRealType lat = std::asin(cosC * sinStanParallel + (y * sinC * cosStanParallel) / c);

			MyRealType a = x * sinC;
			MyRealType b = c * cosStanParallel * cosC - y * sinStanParallel * sinC;

			MyRealType lon = centralLon.rad() + std::atan2(a, b);

			//note - code for cosStanParallel = 0 (+-90 deg) is missing

			return {
				Latitude::rad(lat),
				Longitude::rad(lon)
			};
		};

	};
}

#endif
