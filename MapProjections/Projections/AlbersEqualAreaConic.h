#ifndef ALBERS_EQUAL_AREA_CONIC_H
#define ALBERS_EQUAL_AREA_CONIC_H

#include <cmath>

#include "../GeoCoordinate.h"
#include "../ProjectionInfo.h"
#include "../MapProjectionStructures.h"


namespace Projections
{

	/// <summary>
	/// Based on:
	/// https://mathworld.wolfram.com/AlbersEqual-AreaConicProjection.html
	/// 
	/// </summary>
    class AlbersEqualAreaConic : public ProjectionInfo<AlbersEqualAreaConic>
    {
    public:
        static const bool INDEPENDENT_LAT_LON = false; //can Lat / Lon be computed separatly. To compute one, we dont need the other
        static const bool ORTHOGONAL_LAT_LON = false; //is lat / lon is orthogonal to each other

        AlbersEqualAreaConic(const Longitude& centralLon, const Latitude& centralLat, 
            const Latitude& stanParallel1, const Latitude& stanParallel2) :
            ProjectionInfo(PROJECTION::ALBERS_EQUAL_AREA_CONIC),
            centralLon(centralLon),
            centralLat(centralLat),
            stanParallel1(stanParallel1),
            stanParallel2(stanParallel2),
            n(0.5 * (std::sin(stanParallel1.rad()) + std::sin(stanParallel2.rad()))),
            C(std::pow(std::cos(stanParallel1.rad()), 2.0) + 2.0 * n * std::sin(stanParallel2.rad())),
            rho0(std::sqrt(C - 2.0 * n * std::sin(centralLat.rad())) / n)
        {
        }

        AlbersEqualAreaConic(const AlbersEqualAreaConic& lc) :
            AlbersEqualAreaConic(lc.centralLon, lc.centralLat, lc.stanParallel1, lc.stanParallel2)
        {
            this->frame = lc.frame;
        }

        friend class ProjectionInfo<AlbersEqualAreaConic>;

    protected:
        const Longitude centralLon;
        const Latitude centralLat;
        const Latitude stanParallel1;
        const Latitude stanParallel2;


        const double n;
        const double C;
        const double rho0;

        const char* GetNameInternal() const
        {
            return "AlbersEqualAreaConic";
        }

        ProjectedValue ProjectInternal(const Coordinate& c) const
        {
            MyRealType lat = c.lat.rad();
            MyRealType lon = c.lon.rad();
            
            MyRealType theta = n * (lon - centralLon.rad());

            MyRealType rho = std::sqrt(MyRealType(C - 2.0 * n * std::sin(lat))) / n;

            MyRealType x = rho * std::sin(theta);
            MyRealType y = rho0 - rho * std::cos(theta);

            return { x, y };
        };

        ProjectedValueInverse ProjectInverseInternal(MyRealType x, MyRealType y) const
        {            
            MyRealType dy = rho0 - y;
            MyRealType rho = std::sqrt(x * x + dy * dy);
            //MyRealType rho = std::hypot(x, dy);

            MyRealType theta = std::atan2(x, dy);

            MyRealType lat = std::asin((C - rho * rho * n * n) / (2.0 * n));

            MyRealType lon = centralLon.rad() + theta / n;

            return {
                Latitude::rad(lat),
                Longitude::rad(lon)
            };
        };
    };
}

#endif
