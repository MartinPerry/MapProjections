#ifndef LAMBERT_AZIMUTHAL_H
#define LAMBERT_AZIMUTHAL_H

#include <cmath>

#include "../GeoCoordinate.h"
#include "../ProjectionInfo.h"
#include "../MapProjectionStructures.h"


namespace Projections
{

	/// <summary>
	/// Based on:
	/// https://mathworld.wolfram.com/LambertAzimuthalEqual-AreaProjection.html
	/// 
	/// </summary>
	class LambertAzimuthal : public ProjectionInfo<LambertAzimuthal>
	{
	public:
		static const bool INDEPENDENT_LAT_LON = false; //can Lat / Lon be computed separatly. To compute one, we dont need the other
		static const bool ORTHOGONAL_LAT_LON = false; //is lat / lon is orthogonal to each other

		LambertAzimuthal(const Longitude& centralLon, const Latitude& stanParallel) :
			ProjectionInfo(PROJECTION::LAMBERT_AZIMUTHAL),			
			centralLon(centralLon),
			stanParallel(stanParallel),
			sinStanParallel(std::sin(stanParallel.rad())),
			cosStanParallel(std::cos(stanParallel.rad()))
		{			
		}

		LambertAzimuthal(const LambertAzimuthal& lc) :
			LambertAzimuthal(lc.centralLon, lc.stanParallel)
		{
			this->frame = lc.frame;
		}


		friend class ProjectionInfo<LambertAzimuthal>;

	protected:		
		const Longitude centralLon;
		const Latitude stanParallel;
		
		const double sinStanParallel;
		const double cosStanParallel;

		
		const char* GetNameInternal() const
		{
			return "LambertAzimuthal";
		}

		ProjectedValue ProjectInternal(const Coordinate& c) const
		{
			//vrtule = lat

			double sinLat = std::sin(c.lat.rad());
			double cosLat = std::cos(c.lat.rad());
			double cosLonDif = std::cos(c.lon.rad() - centralLon.rad());

			double tmp0 = sinStanParallel * sinLat;
			double tmp1 = cosStanParallel * cosLat * cosLonDif;

			double k = std::sqrt(2.0 / (1.0 + tmp0 + tmp1));

			MyRealType x = k * cosLat * std::sin(c.lon.rad() - centralLon.rad());
			MyRealType y = k * (cosStanParallel * sinLat - sinStanParallel * cosLat * cosLonDif);
			
			return { x, y };
		};

		ProjectedValueInverse ProjectInverseInternal(MyRealType x, MyRealType y) const
		{
			double ro = std::sqrt(x * x + y * y);
			double c = 2 * std::asin(0.5 * ro);

			double cosC = std::cos(c);
			double sinC = std::sin(c);


			MyRealType lat = std::asin(cosC * sinStanParallel + (y * sinC * cosStanParallel) / ro);

			MyRealType lon = centralLon.rad() + std::atan((x * sinC) / (ro * cosStanParallel * cosC - y * sinStanParallel * sinC));

			return {
				Latitude::rad(lat),
				Longitude::rad(lon)
			};
		};

	};
}

#endif
