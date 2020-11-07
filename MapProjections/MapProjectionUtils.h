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
		/// 
		/// Template parameters:
		/// DataType - type of input data
		/// ReprojType - type of reprojected positions - 
		///				 each pixel has coordinate of this type
		/// Out - output structure - can be raw array of std::vector
		/// ChannelsCount - number of channels in input / output data
	
		/// </summary>
		/// <param name="reproj"></param>
		/// <param name="inputData"></param>
		/// <param name="NO_VALUE"></param>
		/// <returns></returns>
		template <typename DataType, typename ReprojType = int, typename Out = DataType * , size_t ChannelsCount = 1>
		static Out ReprojectData(const Reprojection<ReprojType> & reproj, DataType * inputData, const DataType NO_VALUE)
		{
			size_t count = reproj.outW * reproj.outH;

			Out output;

			if constexpr (std::is_same<Out, DataType *>::value)
			{
				output = new DataType[count * ChannelsCount];
			}
			else if constexpr (std::is_same<Out, std::vector<DataType>>::value)
			{
				output.resize(count * ChannelsCount);
			}

			for (size_t index = 0; index < count; index++)
			{
				if ((reproj.pixels[index].x == -1) || (reproj.pixels[index].y == -1))
				{
					//outside of the model - no data - put there NO_VALUE
					if constexpr (ChannelsCount == 1)
					{
						output[index] = NO_VALUE;
					}
					else
					{
						for (size_t i = 0; i < ChannelsCount; i++)
						{
							output[index * ChannelsCount + i] = NO_VALUE;
						}
					}					
				}
				else
				{
					size_t origIndex = reproj.pixels[index].x + reproj.pixels[index].y * reproj.inW;
					if constexpr (ChannelsCount == 1)
					{
						output[index] = inputData[origIndex];
					}
					else
					{
						for (size_t i = 0; i < ChannelsCount; i++)
						{
							output[index * ChannelsCount + i] = inputData[origIndex * ChannelsCount + i];
						}
					}
				}
			}

			return output;
		}

		/// <summary>
		/// Re-project data from -> to
		/// Calculates mapping: toData[index] = fromData[reprojection[index]]
		/// </summary>
		/// <param name="imProj"></param>
		/// <returns></returns>	
		template <typename FromProjection, typename ToProjection, typename ReprojType = int>
		static Reprojection<ReprojType> CreateReprojection(FromProjection * from, ToProjection * to)
		{
			
			Reprojection<ReprojType> reprojection;
			reprojection.pixels.resize(to->GetFrameHeight() * to->GetFrameWidth(), { -1, -1 });

			//if x and y are independent, simplify
			if ((from->INDEPENDENT_LAT_LON) && (to->INDEPENDENT_LAT_LON))
			{
				std::vector<ReprojType> cacheX;
				cacheX.resize(to->GetFrameWidth());
				std::vector<ReprojType> cacheY;
				cacheY.resize(to->GetFrameHeight());
				
				for (int x = 0; x < to->GetFrameWidth(); x++)
				{
					Projections::Pixel<ReprojType> p = Projections::ProjectionUtils::ReProject<int, ReprojType>({ x, 0 }, from, to);
					cacheX[x] = p.x;
				}
				
				for (int y = 0; y < to->GetFrameHeight(); y++)
				{
					Projections::Pixel<ReprojType> p = Projections::ProjectionUtils::ReProject<int, ReprojType>({ 0, y }, from, to);
					cacheY[y] = p.y;

				}

				for (int y = 0; y < to->GetFrameHeight(); y++)
				{
					int yw = y * to->GetFrameWidth();
					for (int x = 0; x < to->GetFrameWidth(); x++)
					{
						Pixel<ReprojType> p;
						p.x = cacheX[x];
						p.y = cacheY[y];

						if (p.x < 0) continue;
						if (p.y < 0) continue;
						if (p.x >= from->GetFrameWidth()) continue;
						if (p.y >= from->GetFrameHeight()) continue;

						reprojection.pixels[x + yw] = p;

					}
				}
			}
			else
			{
				for (int y = 0; y < to->GetFrameHeight(); y++)
				{
					int yw = y * to->GetFrameWidth();
					for (int x = 0; x < to->GetFrameWidth(); x++)
					{
						Pixel<ReprojType> p = ProjectionUtils::ReProject<int, ReprojType>({ x, y }, from, to);

						if (p.x < 0) continue;
						if (p.y < 0) continue;
						if (p.x >= from->GetFrameWidth()) continue;
						if (p.y >= from->GetFrameHeight()) continue;

						reprojection.pixels[x + yw] = p;

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
