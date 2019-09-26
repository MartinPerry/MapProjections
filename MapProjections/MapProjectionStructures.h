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
		PIXEL_CENTER = 0,
		PIXEL_BORDER = 1

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
		Longitude lon;
		Latitude lat;

		Coordinate() {};
		Coordinate(Longitude lon, Latitude lat) : lon(lon), lat(lat) {};

		static Coordinate CreateFromCartesianLHSystem(MyRealType x, MyRealType y, MyRealType z);
		std::tuple<MyRealType, MyRealType, MyRealType> ConvertToCartesianLHSystem(MyRealType radius);
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
	};

    struct ProjectionConstants
    {
        static const MyRealType PI;
        static const MyRealType PI_4;
        static const MyRealType PI_2;
        static const MyRealType E;
        static const MyRealType EARTH_RADIUS;
    };
    
};

#endif
