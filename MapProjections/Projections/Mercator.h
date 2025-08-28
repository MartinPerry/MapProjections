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
		inline static const Latitude  MERCATOR_MIN = -85.051_deg;
		inline static const Latitude  MERCATOR_MAX = 85.051_deg;

		static const bool INDEPENDENT_LAT_LON = true; //can Lat / Lon be computed separatly. To compute one, we dont need the other
		static const bool ORTHOGONAL_LAT_LON = true; //is lat / lon is orthogonal to each other

		Mercator() : ProjectionInfo(PROJECTION::MERCATOR)
		{ }

		Mercator(const Mercator& me) : Mercator()
		{ 
			this->frame = me.frame;
		}

		friend class ProjectionInfo<Mercator>;

	protected:

		const char* GetNameInternal() const
		{
			return "Mercator";
		}

		ProjectedValue ProjectInternal(const Coordinate & c) const
		{
			return {
				c.lon.rad(),
				std::log(std::tan(ProjectionConstants::PI_4 + MyRealType(0.5) * c.lat.rad()))
			};
		};

		ProjectedValueInverse ProjectInverseInternal(MyRealType x, MyRealType y) const
		{
			//https://www.johndcook.com/blog/2009/09/21/gudermannian/
			//return {
			//	Latitude::rad(std::asin(std::tanh(y))),
			//	Longitude::rad(x)
			//};

			return {
				Latitude::rad(2.0 * std::atan(std::pow(ProjectionConstants::E, y)) - ProjectionConstants::PI_2),
				Longitude::rad(x)
			};
		};

	};
}

#endif
