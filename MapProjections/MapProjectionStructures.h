#ifndef MAP_PROJECTION_STRUCTURES_H
#define MAP_PROJECTION_STRUCTURES_H


#include <vector>
#include <string>
#include <tuple>

#include "GeoCoordinate.h"

#ifdef _MSC_VER
#	ifndef my_fopen 
#		define my_fopen(a, b, c) fopen_s(a, b, c)	
#	endif
#else
#	ifndef my_fopen 
#		define my_fopen(a, b, c) (*a = fopen(b, c))
#	endif
#endif

namespace Projections
{

	typedef enum PROJECTION
    {
		UNKNOWN_PROJ = -1,
		MERCATOR_PROJ = 0,
		LAMBERT_CONIC_PROJ = 1,
		EQUIRECTANGULAR_PROJ = 2,
		WEB_MERCATOR_PROJ = 3,
		POLAR_STEREOGRAPHICS_PROJ = 4,
        MILLER_PROJ = 5,
		GOES_PROJ = 6
	} PROJECTION;

	typedef enum STEP_TYPE
    {
		PIXEL_BORDER = 0,
		PIXEL_CENTER = 1
	} STEP_TYPE;

	template <typename PixelType = int,
    typename = typename std::enable_if<std::is_arithmetic<PixelType>::value, PixelType>::type>
    struct Pixel
    {
        PixelType x;
        PixelType y;
    };

	struct Coordinate
    {
		struct PrecomputedSinCos
		{
			MyRealType sinLat;
			MyRealType cosLat;
			MyRealType sinLon;
			MyRealType cosLon;
		};

		Longitude lon;
		Latitude lat;

		Coordinate() {};
		Coordinate(Longitude lon, Latitude lat) : lon(lon), lat(lat) {};

		PrecomputedSinCos PrecomputeSinCos() const;

		static Coordinate CreateFromCartesianLHSystem(MyRealType x, MyRealType y, MyRealType z);

		template <typename T = std::tuple<MyRealType, MyRealType, MyRealType>>
		T ConvertToCartesianLHSystem(MyRealType radius,
			const Coordinate::PrecomputedSinCos * precomp = nullptr) const;

		template <typename T = std::tuple<MyRealType, MyRealType, MyRealType>>
		T ConvertVectorToCartesianLHSystem(const Longitude & lonDif, const Latitude & latDif,
			const Coordinate::PrecomputedSinCos * precomp = nullptr) const;
	};

	struct Reprojection
    {
		int inW;
		int inH;
		int outW;
		int outH;
		std::vector<Pixel<int>> pixels; //[to] = from

		static Reprojection CreateFromFile(const std::string & fileName);
		void SaveToFile(const std::string & fileName);

	};

	struct ProjectionFrame
	{
		Coordinate min;
		Coordinate max;

		MyRealType minPixelOffsetX; //offset to move min corner to [0,0] and other corners accordingly
        MyRealType minPixelOffsetY;
        
		MyRealType w; //current frame width
		MyRealType h; //current frame height

		MyRealType wPadding;
		MyRealType hPadding;
		MyRealType wAR; //width AR
		MyRealType hAR; //height AR

		MyRealType projPrecomX;
		MyRealType projPrecomY;

		STEP_TYPE stepType;
	};

    struct ProjectionConstants
    {
        static const MyRealType PI;
        static const MyRealType PI_4;
        static const MyRealType PI_2;
        static const MyRealType E;
        static const MyRealType EARTH_RADIUS;
    };
    
	//=============================================================================

	/// <summary>
	/// Convert latitude / longitude to left-handed cartesian coordinate system
	/// Radius is used as scaled earth radius
	/// 
	/// Source:
	/// https://vvvv.org/blog/polar-spherical-and-geographic-coordinates
	/// </summary>
	/// <param name="radius"></param>
	/// <param name="precomp"></param>
	/// <returns></returns>
	template <typename T>
	T Coordinate::ConvertToCartesianLHSystem(MyRealType radius, const Coordinate::PrecomputedSinCos * precomp) const
	{
		MyRealType sinLat, cosLat, sinLon, cosLon;

		if (precomp != nullptr)
		{
			sinLat = precomp->sinLat;
			cosLat = precomp->cosLat;
			sinLon = precomp->sinLon;
			cosLon = precomp->cosLon;
		}
		else
		{
			sinLat = std::sin(lat.rad());
			cosLat = std::cos(lat.rad());
			sinLon = std::sin(lon.rad());
			cosLon = std::cos(lon.rad());
		}

		MyRealType x = radius * cosLat * sinLon;
		MyRealType y = radius * sinLat;
		MyRealType z = -radius * cosLat * cosLon;

		return { x, y, z };
	}

	/// <summary>
	/// Convert (lat, lon) vector to cartesian coordinate vector
	/// in left-handed system
	/// 
	/// Based on:
	/// https://www.brown.edu/Departments/Engineering/Courses/En221/Notes/Polar_Coords/Polar_Coords.htm
	/// (modified for left-handed system and for direct use of lat/lon)
	/// </summary>
	/// <param name="lonDif"></param>
	/// <param name="latDif"></param>
	/// <param name="precomp"></param>
	/// <returns></returns>
	template <typename T>
	T Coordinate::ConvertVectorToCartesianLHSystem(const Longitude & lonDif, const Latitude & latDif,
		const Coordinate::PrecomputedSinCos * precomp) const
	{
		MyRealType sinLat, cosLat, sinLon, cosLon;

		if (precomp != nullptr)
		{
			sinLat = precomp->sinLat;
			cosLat = precomp->cosLat;
			sinLon = precomp->sinLon;
			cosLon = precomp->cosLon;
		}
		else
		{
			sinLat = std::sin(lat.rad());
			cosLat = std::cos(lat.rad());
			sinLon = std::sin(lon.rad());
			cosLon = std::cos(lon.rad());
		}

		//full calculation if vector contains also movement in radius direction
		//double dx = cosLat * sinLon  * (r) - sinLat * sinLon * (latDif.rad()) + cosLon * (lonDif.rad());
		//double dy = sinLat * (r) + cosLat * (latDif.rad());
		//double dz = -cosLat * cosLon * (r) + sinLat * cosLon  * (latDif.rad()) + sinLon * (lonDif.rad());

		MyRealType dx = -sinLat * sinLon * (latDif.rad()) + cosLon * (lonDif.rad());
		MyRealType dy = cosLat * (latDif.rad());
		MyRealType dz = sinLat * cosLon  * (latDif.rad()) + sinLon * (lonDif.rad());

		return { dx, dy, dz };
	}
};

#endif
