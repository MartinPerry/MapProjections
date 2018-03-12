#ifndef _PROJECTIONS_H_
#define _PROJECTIONS_H_

#include "GeoCoordinate.h"
#include "MapProjection.h"
#include "MapProjectionStructures.h"

namespace Projections
{

	class LambertConic : public ProjectionInfo<LambertConic>
	{
	public:
		LambertConic(GeoCoordinate latProjOrigin,
			GeoCoordinate lonCentMeridian,
			GeoCoordinate stanParallel);

		friend class ProjectionInfo<LambertConic>;

	protected:
		GeoCoordinate latProjectionOrigin;
		GeoCoordinate lonCentralMeridian;
		GeoCoordinate standardParallel1;
		GeoCoordinate standardParallel2;


		double f;
		double n;
		double phi0;

        ProjectedValue ProjectInternal(Coordinate c) const
        {
            double t = ProjectionUtils::cot(ProjectionConstants::PI_4 + 0.5 * c.lat.rad());
            double phi = f * std::pow(t, n);
            
            double x = phi * std::sin(n * (c.lon.rad() - lonCentralMeridian.rad()));
            double y = phi0 - phi * std::cos(n * (c.lon.rad() - lonCentralMeridian.rad()));
            
            ProjectedValue p;
            p.x = x;
            p.y = y;
            return p;
        };
        
        ProjectedValueInverse ProjectInverseInternal(double x, double y) const
        {
            double phi = ProjectionUtils::sgn(n) * std::sqrt(x * x + (phi0 - y) * (phi0 - y));
            double delta = std::atan(x / (phi0 - y));
            
            double t = std::pow(f / phi, (1.0 / n));
            
            double lat = 2 * std::atan(t) - ProjectionConstants::PI_2;
            double lon = lonCentralMeridian.rad() + delta / n;
            
			ProjectedValueInverse c;
            c.lat = GeoCoordinate::rad(lat);
            c.lon = GeoCoordinate::rad(lon);
            
            return c;
        };

	};

//============================================================================================================================
//============================================================================================================================
//============================================================================================================================

	class Mercator : public ProjectionInfo<Mercator>
	{
	public:
		Mercator();

		friend class ProjectionInfo<Mercator>;

	protected:
        ProjectedValue ProjectInternal(Coordinate c) const
        {
            ProjectedValue p;
            p.x = c.lon.rad();
            p.y = std::log(std::tan(ProjectionConstants::PI_4 + 0.5 * c.lat.rad()));
            
            return p;
        };
        
        ProjectedValueInverse ProjectInverseInternal(double x, double y) const
        {
			ProjectedValueInverse c;
            c.lon = GeoCoordinate::rad(x);
            c.lat = GeoCoordinate::rad(2.0 * std::atan(std::pow(ProjectionConstants::E, y)) - ProjectionConstants::PI_2);
            return c;
        };

	};


//============================================================================================================================
//============================================================================================================================
//============================================================================================================================

	class Equirectangular : public ProjectionInfo<Equirectangular>
	{
	public:
		Equirectangular();
		Equirectangular(GeoCoordinate lonCentralMeridian);

		friend class ProjectionInfo<Equirectangular>;

	protected:

		GeoCoordinate lonCentralMeridian;
		GeoCoordinate standardParallel;
		double cosStandardParallel;

        ProjectedValue ProjectInternal(Coordinate c) const
        {
            ProjectedValue p;
            p.x = (c.lon.rad() - lonCentralMeridian.rad()) * cosStandardParallel;
            p.y = c.lat.rad() - standardParallel.rad();
            
            return p;
        };
        
        ProjectedValueInverse ProjectInverseInternal(double x, double y) const
        {
			ProjectedValueInverse c;
            
            c.lat = GeoCoordinate::rad(y / cosStandardParallel + lonCentralMeridian.rad());
            c.lon = GeoCoordinate::rad(x + standardParallel.rad());
            
            return c;
        };

	};

//============================================================================================================================
//============================================================================================================================
//============================================================================================================================


	class PolarSteregographic : public ProjectionInfo<PolarSteregographic>
	{
	public:
		PolarSteregographic();
		PolarSteregographic(GeoCoordinate lonCentralMeridian, GeoCoordinate latCentral);

		friend class ProjectionInfo<PolarSteregographic>;

	protected:
		
		GeoCoordinate lonCentralMeridian;
		GeoCoordinate latCentral;

        ProjectedValue ProjectInternal(Coordinate c) const
        {
            double m = (1.0 + std::sin(latCentral.rad())) / (1.0 + std::sin(c.lat.rad()));
            double cosLat = std::cos(c.lat.rad());
            
            ProjectedValue p;
            p.x = +ProjectionConstants::EARTH_RADIUS * m * cosLat * std::sin(c.lon.rad() - lonCentralMeridian.rad());
            p.y = -ProjectionConstants::EARTH_RADIUS * m * cosLat * std::cos(c.lon.rad() - lonCentralMeridian.rad());
            
            return p;
        };
        
        ProjectedValueInverse ProjectInverseInternal(double x, double y) const
        {
            ProjectedValueInverse c;
            
            c.lon = GeoCoordinate::rad(std::atan (-x / y) + lonCentralMeridian.rad());
            
            double tmpSin = std::sin(latCentral.rad());
            double tmp = ProjectionConstants::EARTH_RADIUS * ProjectionConstants::EARTH_RADIUS * (1.0 + tmpSin) * (1.0 + tmpSin);
            double tmp1 = tmp - (x * x + y * y);
            double tmp2 = tmp + (x * x + y * y);
            c.lat = GeoCoordinate::rad(std::asin(tmp1 / tmp2));
            
            
            return c;
        };

	};

};

#endif
