#ifndef MILLER_H
#define MILLER_H

#include <cmath>

#include "../GeoCoordinate.h"
#include "../ProjectionInfo.h"
#include "../MapProjectionStructures.h"


namespace Projections
{

	/// <summary>
	/// Based on:
	/// https://en.wikipedia.org/wiki/Miller_cylindrical_projection
	/// </summary>
	class Miller : public ProjectionInfo<Miller>
	{
	public:
		static const bool INDEPENDENT_LAT_LON = true; //can Lat / Lon be computed separatly. To compute one, we dont need the other
		static const bool ORTHOGONAL_LAT_LON = true; //is lat / lon is orthogonal to each other

		Miller() : ProjectionInfo(PROJECTION::MILLER) {}

		friend class ProjectionInfo<Miller>;

	protected:

		const char* GetNameInternal() const
		{
			return "Miller";
		}

		ProjectedValue ProjectInternal(const Coordinate & c) const
		{			
			return {
				c.lon.rad(),
				1.25 * std::log(std::tan(ProjectionConstants::PI_4 + 0.4 * c.lat.rad()))
			};
		};

		ProjectedValueInverse ProjectInverseInternal(MyRealType x, MyRealType y) const
		{
			return {
				Latitude::rad(2.5 * std::atan(std::pow(ProjectionConstants::E, 0.8 * y)) - 0.625 * ProjectionConstants::PI),
				Longitude::rad(x)
			};
		};

	};
}

#endif
