#ifndef LAMBERT_CONIC_H
#define LAMBERT_CONIC_H

#include "../GeoCoordinate.h"
#include "../ProjectionInfo.h"
#include "../MapProjectionStructures.h"


namespace Projections
{

	/// <summary>
	/// 	// Based on:
	/// http://mathworld.wolfram.com/LambertConformalConicProjection.html
	/// https://en.wikipedia.org/wiki/Lambert_conformal_conic_projection (see only one standard parallel)
	/// </summary>
	class LambertConic : public ProjectionInfo<LambertConic>
	{
	public:
		static const bool INDEPENDENT_LAT_LON = false; //can Lat / Lon be computed separatly. To compute one, we dont need the other

		LambertConic(Latitude latProjOrigin, Longitude lonCentMeridian, Latitude stanParallel) :
			ProjectionInfo(PROJECTION::LAMBERT_CONIC_PROJ),
			latProjectionOrigin(latProjOrigin),
			lonCentralMeridian(lonCentMeridian),
			standardParallel1(stanParallel),
			standardParallel2(stanParallel)
		{
			if (standardParallel1.rad() == standardParallel2.rad())
			{
				n = std::sin(standardParallel1.rad());
			}
			else
			{
				MyRealType t1 = std::cos(standardParallel1.rad()) * ProjectionUtils::sec(standardParallel2.rad());
				MyRealType t2 = std::tan(ProjectionConstants::PI_4 + 0.5 * standardParallel2.rad());
				MyRealType t3 = ProjectionUtils::cot(ProjectionConstants::PI_4 + 0.5 * standardParallel1.rad());
				n = std::log(t1) / std::log(t2 * t3);
			}

			MyRealType t4 = std::tan(ProjectionConstants::PI_4 + 0.5 * standardParallel1.rad());
			MyRealType t5 = ProjectionUtils::cot(ProjectionConstants::PI_4 + 0.5 * latProjectionOrigin.rad());

			f = std::cos(standardParallel1.rad()) * std::pow(t4, n) / n;
			phi0 = f * std::pow(t5, n);
		}


		friend class ProjectionInfo<LambertConic>;

	protected:
		Latitude latProjectionOrigin;
		Longitude lonCentralMeridian;
		Latitude standardParallel1;
		Latitude standardParallel2;


		double f;
		double n;
		double phi0;

		ProjectedValue ProjectInternal(Coordinate c) const
		{
			MyRealType t = ProjectionUtils::cot(ProjectionConstants::PI_4 + 0.5 * c.lat.rad());
			MyRealType phi = f * std::pow(t, n);

			MyRealType x = phi * std::sin(n * (c.lon.rad() - lonCentralMeridian.rad()));
			MyRealType y = phi0 - phi * std::cos(n * (c.lon.rad() - lonCentralMeridian.rad()));

			ProjectedValue p;
			p.x = x;
			p.y = y;
			return p;
		};

		ProjectedValueInverse ProjectInverseInternal(MyRealType x, MyRealType y) const
		{
			MyRealType phi = ProjectionUtils::sgn(n) * std::sqrt(x * x + (phi0 - y) * (phi0 - y));
			MyRealType delta = std::atan(x / (phi0 - y));

			MyRealType t = std::pow(f / phi, (1.0 / n));

			MyRealType lat = 2.0 * std::atan(t) - ProjectionConstants::PI_2;
			MyRealType lon = lonCentralMeridian.rad() + delta / n;

			ProjectedValueInverse c;
			c.lat = Latitude::rad(lat);
			c.lon = Longitude::rad(lon);

			return c;
		};

	};
}

#endif
