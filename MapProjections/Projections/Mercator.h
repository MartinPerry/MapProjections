#ifndef MERCATOR_H
#define MERCATOR_H

#include <cmath>

#include "../GeoCoordinate.h"
#include "../ProjectionInfo.h"
#include "../MapProjectionStructures.h"


namespace Projections
{

	/// <summary>
	/// Based on:
	/// http://mathworld.wolfram.com/MercatorProjection.html
	/// </summary>
	class Mercator : public ProjectionInfo<Mercator>
	{
	public:
		inline static const double  MERCATOR_MIN = -85.051;
		inline static const double  MERCATOR_MAX = 85.051;

		static const bool INDEPENDENT_LAT_LON = true; //can Lat / Lon be computed separatly. To compute one, we dont need the other
		static const bool ORTHOGONAL_LAT_LON = true; //is lat / lon is orthogonal to each other

		Mercator() : ProjectionInfo(PROJECTION::MERCATOR)
		{ }

		friend class ProjectionInfo<Mercator>;

	protected:
		ProjectedValue ProjectInternal(const Coordinate & c) const
		{
			ProjectedValue p;
			p.x = c.lon.rad();
			p.y = std::log(std::tan(ProjectionConstants::PI_4 + 0.5 * c.lat.rad()));

			return p;
		};

		ProjectedValueInverse ProjectInverseInternal(MyRealType x, MyRealType y) const
		{
			ProjectedValueInverse c;
			c.lon = Longitude::rad(x);
			c.lat = Latitude::rad(2.0 * std::atan(std::pow(ProjectionConstants::E, y)) - ProjectionConstants::PI_2);
			return c;
		};

	};
}

#endif
