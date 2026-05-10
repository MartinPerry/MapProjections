#ifndef TRANSVERSE_MERCATOR_H
#define TRANSVERSE_MERCATOR_H

#include <cmath>

#include "../GeoCoordinate.h"
#include "../ProjectionInfo.h"
#include "../MapProjectionStructures.h"


namespace Projections
{

	/// <summary>
	/// Based on:
	/// https://en.wikipedia.org/wiki/Transverse_Mercator_projection
	/// </summary>
	class TransverseMercator : public ProjectionInfo<TransverseMercator>
	{
	public:
		
		static const bool INDEPENDENT_LAT_LON = true; //can Lat / Lon be computed separatly. To compute one, we dont need the other
		static const bool ORTHOGONAL_LAT_LON = false; //is lat / lon is orthogonal to each other

		TransverseMercator(const Longitude& centralLon, const Latitude& centralLat) :
			ProjectionInfo(PROJECTION::TRANSVERSE_MERCATOR),
			centralLon(centralLon),
			centralLat(centralLat)
		{
		}

		TransverseMercator(const TransverseMercator& tme) :
			TransverseMercator(tme.centralLon, tme.centralLat)
		{
			this->frame = tme.frame;
		}

		friend class ProjectionInfo<TransverseMercator>;

	protected:

		const MyRealType RADIUS_EQUATOR = MyRealType(6378.1370); //in km

		const Longitude centralLon;
		const Latitude centralLat;

		const char* GetNameInternal() const
		{
			return "TransverseMercator";
		}

		ProjectedValue ProjectInternal(const Coordinate& c) const
		{
			//centralLon / centralLat added by ChatGPT

			auto dLon = c.lon.rad() - centralLon.rad();
			auto tmp = std::sin(dLon) * std::cos(c.lat.rad());

			//ses(x) = 1.0 / cos(x)

			return {
				0.5 * RADIUS_EQUATOR * std::log((1 + tmp) / (1 - tmp)),
				RADIUS_EQUATOR * (std::atan(ProjectionUtils::sec(dLon) * std::tan(c.lat.rad())) - centralLat.rad())
			};
		};

		ProjectedValueInverse ProjectInverseInternal(MyRealType x, MyRealType y) const
		{	
			//centralLon / centralLat added by ChatGPT

			const auto D = x / RADIUS_EQUATOR;
			const auto E = y / RADIUS_EQUATOR + centralLat.rad();

			const auto lat = std::asin(std::sin(E) / std::cosh(D));
			const auto lon = centralLon.rad() + std::atan2(std::sinh(D), std::cos(E));

			return {
				Latitude::rad(lat),
				Longitude::rad(lon)
			};
		};

	};
}

#endif
