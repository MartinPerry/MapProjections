#ifndef PROJECTIONS_H
#define PROJECTIONS_H

#include "GeoCoordinate.h"
#include "ProjectionInfo.h"
#include "MapProjectionStructures.h"

namespace Projections
{

    //=======================================================================
    // Lambert-conic
    //
    // Based on:
    // http://mathworld.wolfram.com/LambertConformalConicProjection.html
    // https://en.wikipedia.org/wiki/Lambert_conformal_conic_projection (see only one standard parallel)
    //=======================================================================
	class LambertConic : public ProjectionInfo<LambertConic>
	{
	public:
		static const bool INDEPENDENT_LAT_LON = false; //Lat / Lon can be computed separatly. To compute one, we dont need the other

		LambertConic(Latitude latProjOrigin, Longitude lonCentMeridian, Latitude stanParallel) :
            ProjectionInfo(PROJECTION::LAMBERT_CONIC),
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

//================================================================================================================
//================================================================================================================
//================================================================================================================

    //=======================================================================
    // Mercator
    //
    // Based on:
    // http://mathworld.wolfram.com/MercatorProjection.html
    //=======================================================================
	class Mercator : public ProjectionInfo<Mercator>
	{
	public:
		static const bool INDEPENDENT_LAT_LON = true; //Lat / Lon can be computed separatly. To compute one, we dont need the other

		Mercator() : ProjectionInfo(PROJECTION::MERCATOR) { }

		friend class ProjectionInfo<Mercator>;

	protected:
        ProjectedValue ProjectInternal(Coordinate c) const
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

//==============================================================================================================
//==============================================================================================================
//==============================================================================================================
    
    //=======================================================================
    // Miller
    //
    // Based on:
    // https://en.wikipedia.org/wiki/Miller_cylindrical_projection
    //=======================================================================
    class Miller : public ProjectionInfo<Miller>
    {
    public:
		static const bool INDEPENDENT_LAT_LON = true; //Lat / Lon can be computed separatly. To compute one, we dont need the other

        Miller() : ProjectionInfo(PROJECTION::MILLER) {}
        
        friend class ProjectionInfo<Miller>;
        
    protected:
        ProjectedValue ProjectInternal(Coordinate c) const
        {
            ProjectedValue p;
            p.x = c.lon.rad();
            p.y = 1.25 * std::log(std::tan(ProjectionConstants::PI_4 + 0.4 * c.lat.rad()));
            
            return p;
        };
        
        ProjectedValueInverse ProjectInverseInternal(MyRealType x, MyRealType y) const
        {
            ProjectedValueInverse c;
            c.lon = Longitude::rad(x);
            c.lat = Latitude::rad(2.5 * std::atan(std::pow(ProjectionConstants::E, 0.8 * y)) - 0.625 * ProjectionConstants::PI);
            return c;
        };
        
    };

//===============================================================================================================
//===============================================================================================================
//===============================================================================================================

    //=======================================================================
    // Equirectangular
    //
    // Based on:
    // https://en.wikipedia.org/wiki/Equirectangular_projection
    //=======================================================================
	class Equirectangular : public ProjectionInfo<Equirectangular>
	{
	public:

		static const bool INDEPENDENT_LAT_LON = true; //Lat / Lon can be computed separatly. To compute one, we dont need the other

        Equirectangular() : Equirectangular(0.0_deg) {}
        Equirectangular(Longitude lonCentralMeridian) :
            ProjectionInfo(PROJECTION::EQUIRECTANGULAR),
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

//===============================================================================================================
//===============================================================================================================
//===============================================================================================================

    //=======================================================================
    // PolarSteregographic
    //
    // Full computation:
    // https://web.archive.org/web/20150723100408/http://www.knmi.nl/~beekhuis/rad_proj.html
    // with Eccentricity is 1.0 => Based on:
    // https://www.dwd.de/DE/leistungen/radolan/radolan_info/radolan_radvor_op_komposit_format_pdf.pdf?__blob=publicationFile&v=8
    //
    //=======================================================================
	class PolarSteregographic : public ProjectionInfo<PolarSteregographic>
	{
	public:

		static const bool INDEPENDENT_LAT_LON = false; //Lat / Lon can be computed separatly. To compute one, we dont need the other

        PolarSteregographic() : PolarSteregographic(10.0_deg, 60.0_deg) {}
        PolarSteregographic(Longitude lonCentralMeridian, Latitude latCentral) : ProjectionInfo(PROJECTION::POLAR_STEREOGRAPHICS),
            lonCentralMeridian(lonCentralMeridian),
            latCentral(latCentral)
        { }

		friend class ProjectionInfo<PolarSteregographic>;

	protected:
		
		Longitude lonCentralMeridian;
		Latitude latCentral;

        ProjectedValue ProjectInternal(Coordinate c) const
        {
            MyRealType m = (1.0 + std::sin(latCentral.rad())) / (1.0 + std::sin(c.lat.rad()));
            MyRealType cosLat = std::cos(c.lat.rad());
            
            ProjectedValue p;
            p.x = +ProjectionConstants::EARTH_RADIUS * m * cosLat * std::sin(c.lon.rad() - lonCentralMeridian.rad());
            p.y = -ProjectionConstants::EARTH_RADIUS * m * cosLat * std::cos(c.lon.rad() - lonCentralMeridian.rad());
            
            return p;
        };
        
        ProjectedValueInverse ProjectInverseInternal(MyRealType x, MyRealType y) const
        {
            ProjectedValueInverse c;
            
            c.lon = Longitude::rad(std::atan (-x / y) + lonCentralMeridian.rad());
            
            MyRealType tmpSin = std::sin(latCentral.rad());
            MyRealType tmp = ProjectionConstants::EARTH_RADIUS * ProjectionConstants::EARTH_RADIUS * (1.0 + tmpSin) * (1.0 + tmpSin);
            MyRealType tmp1 = tmp - (x * x + y * y);
            MyRealType tmp2 = tmp + (x * x + y * y);
            c.lat = Latitude::rad(std::asin(tmp1 / tmp2));
            
            
            return c;
        };

	};

};

#endif
