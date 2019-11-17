#ifndef MAP_PROJECTION_UTILS_H
#define MAP_PROJECTION_UTILS_H

#include <vector>


#include "./MapProjectionStructures.h"
#include "./ProjectionInfo.h"

namespace Projections
{

	struct ProjectionUtils
	{
		/// <summary>
		/// Reproject inputData based on reproj.
		/// Output array has size reproj.outW * reproj.outH
		/// Output array must be released with delete[]
		/// </summary>
		/// <param name="reproj"></param>
		/// <param name="inputData"></param>
		/// <param name="NO_VALUE"></param>
		/// <returns></returns>
		template <typename DataType>
		static DataType * ReprojectData(const Reprojection & reproj, DataType * inputData, DataType NO_VALUE)
		{			
			size_t count = reproj.outW * reproj.outH;

			DataType * output = new DataType[count];

			for (size_t index = 0; index < count; index++)
			{
				if ((reproj.pixels[index].x == -1) || (reproj.pixels[index].y == -1))
				{
					//outside of the model - no data - put there NO_VALUE
					output[index] = NO_VALUE;
					continue;
				}
				size_t origIndex = reproj.pixels[index].x + reproj.pixels[index].y * reproj.inW;
				output[index] = inputData[origIndex];
			}
			
			return output;
		}

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

			//if x and y are independent, simplify
			if ((from->INDEPENDENT_LAT_LON) && (to->INDEPENDENT_LAT_LON))
			{
				std::vector<int> cacheX;
				cacheX.resize(to->GetFrameWidth());
				std::vector<int> cacheY;
				cacheY.resize(to->GetFrameHeight());
				
				for (int x = 0; x < to->GetFrameWidth(); x++)
				{
					Projections::Pixel<int> p = Projections::ProjectionUtils::ReProject<int, int>({ x, 0 }, from, to);
					cacheX[x] = p.x;
				}
				
				for (int y = 0; y < to->GetFrameHeight(); y++)
				{
					Projections::Pixel<int> p = Projections::ProjectionUtils::ReProject<int, int>({ 0, y }, from, to);
					cacheY[y] = p.y;

				}

				for (int y = 0; y < to->GetFrameHeight(); y++)
				{
					for (int x = 0; x < to->GetFrameWidth(); x++)
					{
						Pixel<int> p;
						p.x = cacheX[x];
						p.y = cacheY[y];

						if (p.x < 0) continue;
						if (p.y < 0) continue;
						if (p.x >= from->GetFrameWidth()) continue;
						if (p.y >= from->GetFrameHeight()) continue;

						reprojection.pixels[x + y * to->GetFrameWidth()] = p;

					}
				}
			}
			else
			{
				for (int y = 0; y < to->GetFrameHeight(); y++)
				{
					for (int x = 0; x < to->GetFrameWidth(); x++)
					{
						Pixel<int> p = ProjectionUtils::ReProject<int, int>({ x, y }, from, to);

						if (p.x < 0) continue;
						if (p.y < 0) continue;
						if (p.x >= from->GetFrameWidth()) continue;
						if (p.y >= from->GetFrameHeight()) continue;

						reprojection.pixels[x + y * to->GetFrameWidth()] = p;

					}
				}
			}

			reprojection.inW = from->GetFrameWidth();
			reprojection.inH = from->GetFrameHeight();
			reprojection.outW = to->GetFrameWidth();
			reprojection.outH = to->GetFrameHeight();

			return reprojection;
		};


		/// <summary>
		/// Reproject single pixel from -> to
		/// </summary>
		/// <param name="p"></param>
		/// <param name="from"></param>
		/// <param name="to"></param>
		/// <returns></returns>
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
	
		static Coordinate CalcEndPointShortest(const Coordinate & start, const Angle & bearing, MyRealType dist);
		static Coordinate CalcEndPointDirect(const Coordinate & start, const Angle & bearing, MyRealType dist);
		static double Distance(const Coordinate & from, const Coordinate & to);
        
        inline static MyRealType cot(MyRealType x) { return 1.0 / std::tan(x); };
        inline static MyRealType sec(MyRealType x) { return 1.0 / std::cos(x); };
        inline static MyRealType sinc(MyRealType x) { return std::sin(x) / x; };
        inline static MyRealType sgn(MyRealType x) { return (x < 0) ? -1 : (x > 0); };
              
	};

};

#endif
