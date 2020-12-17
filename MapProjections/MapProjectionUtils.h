#ifndef MAP_PROJECTION_UTILS_H
#define MAP_PROJECTION_UTILS_H

#include <vector>


#include "./MapProjectionStructures.h"
#include "./ProjectionInfo.h"

namespace Projections
{

	struct ProjectionUtils
	{
		

		/*
		template <typename InPixelType, typename OutPixelType,
			typename FromProjection, typename ToProjection>
		static Pixel<OutPixelType> ReProject(Pixel<InPixelType> p,
			const FromProjection & from, const ToProjection & to);
		*/

		static void ComputeAABB(const std::vector<Coordinate> & c,
			Coordinate & min, Coordinate & max)
		{
			if (c.size() == 0)
			{				
				return;
			}

			min = c[0];
			max = c[0];
			for (size_t i = 1; i < c.size(); i++)
			{
				if (c[i].lat.rad() < min.lat.rad()) min.lat = c[i].lat;
				if (c[i].lon.rad() < min.lon.rad()) min.lon = c[i].lon;


				if (c[i].lat.rad() > max.lat.rad()) max.lat = c[i].lat;
				if (c[i].lon.rad() > max.lon.rad()) max.lon = c[i].lon;

			}
		};
	
		static Coordinate CalcEndPointShortest(const Coordinate & start, const AngleValue & bearing, MyRealType dist);
		static Coordinate CalcEndPointDirect(const Coordinate & start, const AngleValue & bearing, MyRealType dist);
		static double Distance(const Coordinate & from, const Coordinate & to);
		
		static double CalcArea(const std::vector<Coordinate> & pts);
		
		template <typename PixelType, typename Projection>
		static double CalcArea(const std::vector<Pixel<PixelType>> & pxs, const Projection * from)
		{
			if (pxs.size() <= 2)
			{
				return 0.0;
			}

			std::vector<Coordinate> pts;
			for (auto & px : pxs)
			{
				auto gps = from->template  ProjectInverse<PixelType, false>(px);
				pts.push_back(gps);
			}

			return CalcArea(pts);
		}

        inline static MyRealType cot(MyRealType x) { return 1.0 / std::tan(x); };
        inline static MyRealType sec(MyRealType x) { return 1.0 / std::cos(x); };
        inline static MyRealType sinc(MyRealType x) { return std::sin(x) / x; };
        inline static MyRealType sgn(MyRealType x) { return (x < 0) ? -1 : (x > 0); };
              
	};

};

#endif
