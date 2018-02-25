#ifndef _MAP_PROJECTION_STRUCTURES_H_
#define _MAP_PROJECTION_STRUCTURES_H_


#include <vector>

#include "GeoCoordinate.h"

namespace Projections
{

	typedef enum PROJECTION {
		MERCATOR = 0,
		LAMBERT_CONIC = 1,
		EQUIRECTANGULAR = 2,
		WEB_MERCATOR = 3,
		POLAR_STEREOGRAPHICS = 4
	} PROJECTION;

	typedef enum STEP_TYPE {
		PIXEL_CENTER = 0,
		PIXEL_BORDER = 1

	} STEP_TYPE;

	template <typename PixelType = int,
		typename = typename std::enable_if<std::is_arithmetic<PixelType>::value, PixelType>::type>
		struct Pixel { PixelType x; PixelType y; };

	struct Coordinate {
		GeoCoordinate lat;
		GeoCoordinate lon;

		Coordinate() {};
		Coordinate(GeoCoordinate lon, GeoCoordinate lat) : lon(lon), lat(lat) {};
	};

	struct Reprojection {
		int inW;
		int inH;
		int outW;
		int outH;
		std::vector<Pixel<int>> pixels; //[to] = from

		static Reprojection CreateFromFile(const std::string & fileName);
		void SaveToFile(const std::string & fileName);

	};

	struct ProjectedValue
	{
		double x;
		double y;
	};

	struct ProjectionFrame
	{
		Coordinate min;
		Coordinate max;

		ProjectedValue minPixelOffset; //offset to move min corner to [0,0] and other corners accordingly

		double w; //current frame width
		double h; //current frame height

		double wPadding;
		double hPadding;
		double wAR; //width AR
		double hAR; //height AR

		double projInvPrecomW;
		double projInvPrecomH;
	};

};

#endif
