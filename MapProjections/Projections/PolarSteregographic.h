#ifndef POLAR_STEREOGRAPHIC_H
#define POLAR_STEREOGRAPHIC_H

#include <cmath>

#include "../GeoCoordinate.h"
#include "../ProjectionInfo.h"
#include "../MapProjectionStructures.h"


namespace Projections
{	
	/// <summary>
	/// Full computation:
	/// https://web.archive.org/web/20150723100408/http://www.knmi.nl/~beekhuis/rad_proj.html
	/// with Eccentricity is 1.0 => Based on:
	/// https://www.dwd.de/DE/leistungen/radolan/radolan_info/radolan_radvor_op_komposit_format_pdf.pdf?__blob=publicationFile&v=8
	/// </summary>
	class PolarSteregographic : public ProjectionInfo<PolarSteregographic>
	{
	public:

		static const bool INDEPENDENT_LAT_LON = false; //can Lat / Lon be computed separatly. To compute one, we dont need the other
		static const bool ORTHOGONAL_LAT_LON = false; //is lat / lon is orthogonal to each other

		PolarSteregographic() : PolarSteregographic(Longitude::deg(10.0), Latitude::deg(60.0)) {}
		PolarSteregographic(const Longitude & lonCentralMeridian, const Latitude & latCentral) : ProjectionInfo(PROJECTION::POLAR_STEREOGRAPHICS),
			lonCentralMeridian(lonCentralMeridian),
			latCentral(latCentral)
		{ }

		friend class ProjectionInfo<PolarSteregographic>;

	protected:

		const Longitude lonCentralMeridian;
		const Latitude latCentral;

		const char* GetNameInternal() const
		{
			return "PolarSteregographic";
		}

		ProjectedValue ProjectInternal(const Coordinate & c) const
		{
			MyRealType m = (1.0 + std::sin(latCentral.rad())) / (1.0 + std::sin(c.lat.rad()));
			MyRealType cosLat = std::cos(c.lat.rad());

			return {
				ProjectionConstants::EARTH_RADIUS * m * cosLat * std::sin(c.lon.rad() - lonCentralMeridian.rad()),
				-ProjectionConstants::EARTH_RADIUS * m * cosLat * std::cos(c.lon.rad() - lonCentralMeridian.rad())
			};			
		};

		ProjectedValueInverse ProjectInverseInternal(MyRealType x, MyRealType y) const
		{			
			MyRealType tmpSin = std::sin(latCentral.rad());
			MyRealType tmp = ProjectionConstants::EARTH_RADIUS * ProjectionConstants::EARTH_RADIUS * (1.0 + tmpSin) * (1.0 + tmpSin);
			MyRealType tmp1 = tmp - (x * x + y * y);
			MyRealType tmp2 = tmp + (x * x + y * y);
		
			return {
				Latitude::rad(std::asin(tmp1 / tmp2)),
				Longitude::rad(std::atan(-x / y) + lonCentralMeridian.rad())
			};
		};

	};
}

#endif
