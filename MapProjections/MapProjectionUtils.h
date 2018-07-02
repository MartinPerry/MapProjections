#ifndef _MAP_PROJECTION_UTILS_H_
#define _MAP_PROJECTION_UTILS_H_


namespace Projections
{
	class IProjectionInfo;
}

#include <vector>


#include "./MapProjectionStructures.h"
#include "./MapProjection.h"

namespace Projections
{

	struct ProjectionUtils
	{
		
		static Reprojection CreateReprojection(IProjectionInfo * from, IProjectionInfo * to)
		{	
			//dynamic_cast<typeid(Mercator) *>(from);

			//TO DO !!!!!!

			
			Reprojection reprojection;
			return reprojection;
		};
		
		/// <summary>
		/// Re-project data from -> to
		/// Calculates mapping: toData[index] = fromData[reprojection[index]]
		/// </summary>
		/// <param name="imProj"></param>
		/// <returns></returns>	
		template <typename FromProjection, typename ToProjection>
		static Reprojection CreateReprojection(FromProjection * from, ToProjection * to)
		{
			
			Reprojection reprojection;
			reprojection.pixels.resize(to->GetFrameHeight() * to->GetFrameWidth(), { -1, -1 });

			for (int y = 0; y < to->GetFrameHeight(); y++)
			{
				for (int x = 0; x < to->GetFrameWidth(); x++)
				{
					Pixel<int> p = ProjectionUtils::ReProject<int, int>({ x, y }, from, to);

					//IProjectionInfo<Proj>::Coordinate cc = to->ProjectInverse({ x,y });
					//IProjectionInfo<Proj>::Pixel<int> p = from->Project<int>(cc);

					if (p.x < 0) continue;
					if (p.y < 0) continue;
					if (p.x >= from->GetFrameWidth()) continue;
					if (p.y >= from->GetFrameHeight()) continue;

					reprojection.pixels[x + y * to->GetFrameWidth()] = p;

				}
			}

			reprojection.inW = from->GetFrameWidth();
			reprojection.inH = from->GetFrameHeight();
			reprojection.outW = to->GetFrameWidth();
			reprojection.outH = to->GetFrameHeight();

			return reprojection;
		};


		template <typename InPixelType, typename OutPixelType,
			typename FromProjection, typename ToProjection>
			static Pixel<OutPixelType> ReProject(Pixel<InPixelType> p,
				const FromProjection * from, const ToProjection * to)
		{
            Coordinate cc = to->template ProjectInverse<InPixelType, false>(p);
            return from->template Project<OutPixelType>(cc);
		};

		/*
		template <typename InPixelType, typename OutPixelType,
			typename FromProjection, typename ToProjection>
		static Pixel<OutPixelType> ReProject(Pixel<InPixelType> p,
			const FromProjection & from, const ToProjection & to);
		*/

		static void ComputeAABB(const std::vector<Coordinate> & c,
			Coordinate & min, Coordinate & max)
		{
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

		static double NormalizeLon(double lonDeg)
		{
			return std::fmod(lonDeg + 540, 360) - 180;
		};

		static double NormalizeLat(double latDeg)
		{
			return (latDeg > 90) ? (latDeg - 180) : latDeg;
		};

		static double Distance(const Coordinate & from, const Coordinate & to)
		{
			//haversine distance in km
			//http://stackoverflow.com/questions/365826/calculate-distance-between-2-gps-coordinates

			double dlong = to.lon.rad() - from.lon.rad();
			double dlat = to.lat.rad() - from.lat.rad();

			double a = std::pow(std::sin(dlat / 2.0), 2) + std::cos(from.lat.rad()) * std::cos(to.lat.rad()) * std::pow(std::sin(dlong / 2.0), 2);
			double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
			double d = 6367 * c;

			return d;
		};
        
        inline static double cot(double x) { return 1.0 / std::tan(x); };
        inline static double sec(double x) { return 1.0 / std::cos(x); };
        inline static double sinc(double x) { return std::sin(x) / x; };
        inline static double sgn(double x) { return (x < 0) ? -1 : (x > 0); };
      
        inline static double degToRad(double x) { return x * 0.0174532925; }
        inline static double radToDeg(double x) { return x * 57.2957795; }
	};

};

#endif
