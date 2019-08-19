#ifndef EQUIRECTANGULAR_H
#define EQUIRECTANGULAR_H

#include "../GeoCoordinate.h"
#include "../ProjectionInfo.h"
#include "../MapProjectionStructures.h"


namespace Projections
{

	/// <summary>
	/// Based on:
	/// https://en.wikipedia.org/wiki/Equirectangular_projection
	/// </summary>
	class Equirectangular : public ProjectionInfo<Equirectangular>
	{
	public:

		static const bool INDEPENDENT_LAT_LON = true; //can Lat / Lon be computed separatly. To compute one, we dont need the other

		Equirectangular() : Equirectangular(0.0_deg) {}
		Equirectangular(Longitude lonCentralMeridian) :
			ProjectionInfo(PROJECTION::EQUIRECTANGULAR_PROJ),
			lonCentralMeridian(lonCentralMeridian),
			standardParallel(0.0_deg),
			cosStandardParallel(std::cos(standardParallel.rad()))
		{ }

		friend class ProjectionInfo<Equirectangular>;

	protected:

		Longitude lonCentralMeridian;
		Latitude standardParallel;
		double cosStandardParallel;

		ProjectedValue ProjectInternal(Coordinate c) const
		{
			ProjectedValue p;
			p.x = (c.lon.rad() - lonCentralMeridian.rad()) * cosStandardParallel;
			p.y = c.lat.rad() - standardParallel.rad();

			return p;
		};

		ProjectedValueInverse ProjectInverseInternal(MyRealType x, MyRealType y) const
		{
			ProjectedValueInverse c;

			c.lat = Latitude::rad(y / cosStandardParallel + lonCentralMeridian.rad());
			c.lon = Longitude::rad(x + standardParallel.rad());

			return c;
		};

	};
}

#endif
