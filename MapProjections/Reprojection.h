#ifndef REPROJECTION_H
#define REPROJECTION_H

#include <vector>


#include "./MapProjectionStructures.h"
#include "./ProjectionInfo.h"

namespace Projections
{

	/// <summary>
	/// Reprojection structure
	/// It holds info needed to reproject data from one projection
	/// to another
	/// pixels - reprojection info
	/// pixels[to] = from
	/// 
	/// Template type is type of reprojection Pixel
	/// By default its int -> reprojection can be in range of int
	/// However in many cases we dont have such big images
	/// and we can use short
	/// 
	/// Calculates mapping: toData[index] = fromData[reprojection[index]]
	/// </summary>
	/*
		template <typename T = int,
			typename = typename std::enable_if<
			std::is_same<T, int>::value ||
			std::is_same<T, short>::value>::type
		>
	*/
	template <typename T = int>
	struct Reprojection
	{	
		int inW;
		int inH;
		int outW;
		int outH;
		std::vector<Pixel<T>> pixels; //[to] = from

		Reprojection() : 
			inW(0),
			inH(0),
			outW(0),
			outH(0)
		{
		}

		/// <summary>
		/// Create cache name in format:
		/// reproj_from_w_h_to_w_h
		/// </summary>
		/// <param name="from"></param>
		/// <param name="to"></param>
		/// <returns></returns>
		template <typename FromProjection, typename ToProjection>
		static std::string CreateCacheName(FromProjection* from, ToProjection* to)
		{
			std::string tmp = "reproj_";
			tmp += from->GetName();
			tmp += "_";
#ifdef USE_VIRTUAL_INTERFACE
			tmp += std::to_string(static_cast<int>(from->GetFrame().w));
			tmp += "_";
			tmp += std::to_string(static_cast<int>(from->GetFrame().h));
			tmp += "_";
#endif
			tmp += to->GetName();
#ifdef USE_VIRTUAL_INTERFACE
			tmp += "_";
			tmp += std::to_string(static_cast<int>(to->GetFrame().w));
			tmp += "_";
			tmp += std::to_string(static_cast<int>(to->GetFrame().h));
#endif
			
			return tmp;
		}

		/// <summary>
		/// Load reprojection from file		
		/// </summary>
		/// <param name="imProj"></param>
		/// <returns></returns>	
		static Reprojection<T> CreateFromFile(const std::string& fileName);
		
		/// <summary>
		/// Re-project data from -> to
		/// Calculates mapping: toData[index] = fromData[reprojection[index]]
		/// </summary>
		/// <param name="imProj"></param>
		/// <returns></returns>	
		template <typename FromProjection, typename ToProjection>
		static Reprojection<T> CreateReprojection(FromProjection* from, ToProjection* to)
		{

			Reprojection<T> reprojection;
			reprojection.pixels.resize(to->GetFrameWidth() * to->GetFrameHeight(), { -1, -1 });

			const auto& f = to->GetFrame();

			if ((f.repeatNegCount != 0) || (f.repeatPosCount != 0))
			{
				//we have multiple wrap around of the world

				//calculate full size of from projection image
				Projections::Coordinate bbMin, bbMax;

				bbMin.lat = -90.0_deg;
				bbMin.lon = -180.0_deg;

				bbMax.lat = 90.06_deg;
				bbMax.lon = 180.0_deg;

				Pixel<MyRealType> pp1 = from->Project<MyRealType>(bbMin);
				Pixel<MyRealType> pp2 = from->Project<MyRealType>(bbMax);

				MyRealType ww = pp2.x - pp1.x;
				MyRealType hh = pp1.y - pp2.y;
				
				for (int y = 0; y < to->GetFrameHeight(); y++)
				{
					int yw = y * to->GetFrameWidth();
					for (int x = 0; x < to->GetFrameWidth(); x++)
					{
						Pixel<T> p = Reprojection<T>::ReProject<int, T>({ x, y }, from, to);

						if ((p.x >= 0) &&
							(p.y >= 0) &&
							(p.x < from->GetFrameWidth()) &&
							(p.y < from->GetFrameHeight()))
						{
							reprojection.pixels[x + yw] = p;
						}


						int offset = static_cast<int>(ww);

						int px = p.x;

						MyRealType nc = f.repeatNegCount;						
						while (nc > 0)
						{
							p.x += offset;

							if ((p.x >= 0) &&
								(p.y >= 0) &&
								(p.x < from->GetFrameWidth()) &&
								(p.y < from->GetFrameHeight()))
							{
								reprojection.pixels[x + yw] = p;
							}
							nc--;
						}

						p.x = px; //restore p.x

						MyRealType pc = f.repeatPosCount;						
						while (pc > 0)
						{
							p.x -= offset;

							if ((p.x >= 0) &&
								(p.y >= 0) &&
								(p.x < from->GetFrameWidth()) &&
								(p.y < from->GetFrameHeight()))
							{
								reprojection.pixels[x + yw] = p;
							}
							pc--;
						}

					}
				}

			}						
			else if ((from->INDEPENDENT_LAT_LON) && (to->INDEPENDENT_LAT_LON))
			{
				//if x and y are independent, simplify

				std::vector<T> cacheX;
				cacheX.resize(to->GetFrameWidth());

				std::vector<T> cacheY;
				cacheY.resize(to->GetFrameHeight());

				for (int x = 0; x < to->GetFrameWidth(); x++)
				{
					Projections::Pixel<T> p = Reprojection<T>::ReProject<int, T>({ x, 0 }, from, to);
					cacheX[x] = p.x;
				}

				for (int y = 0; y < to->GetFrameHeight(); y++)
				{
					Projections::Pixel<T> p = Reprojection<T>::ReProject<int, T>({ 0, y }, from, to);
					cacheY[y] = p.y;

				}

				for (int y = 0; y < to->GetFrameHeight(); y++)
				{
					int yw = y * to->GetFrameWidth();
					for (int x = 0; x < to->GetFrameWidth(); x++)
					{
						Pixel<T> p;
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
						Pixel<T> p = Reprojection<T>::ReProject<int, T>({ x, y }, from, to);

						if ((p.x >= 0) && 
							(p.y >= 0) && 
							(p.x < from->GetFrameWidth()) &&
							(p.y < from->GetFrameHeight()))
						{
							reprojection.pixels[x + yw] = p;
						}						
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
		/// Save reprojection to file
		/// </summary>
		/// <param name="fileName"></param>
		void SaveToFile(const std::string& fileName);


		/// <summary>
		/// Reproject inputData based on reproj.
		/// Output array has size reproj.outW * reproj.outH
		/// Output array must be released with delete[]
		/// 
		/// Template parameters:
		/// DataType - type of input data		
		/// Out - output structure - can be raw array of std::vector
		/// ChannelsCount - number of channels in input / output data
		/// </summary>
		/// <param name="reproj"></param>
		/// <param name="inputData"></param>
		/// <param name="NO_VALUE"></param>
		/// <returns></returns>
		template <typename DataType, typename Out = DataType*, size_t ChannelsCount = 1>
		Out ReprojectData(DataType* inputData, const DataType NO_VALUE) const
		{
			size_t count = this->outW * this->outH;

			Out output;

			if constexpr (std::is_same<Out, DataType*>::value)
			{
				output = new DataType[count * ChannelsCount];
			}
			else if constexpr (std::is_same<Out, std::vector<DataType>>::value)
			{
				output.resize(count * ChannelsCount);
			}

			for (size_t index = 0; index < count; index++)
			{
				if ((this->pixels[index].x == -1) || (this->pixels[index].y == -1))
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
					size_t origIndex = this->pixels[index].x + this->pixels[index].y * this->inW;
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
		/// Reproject single pixel from -> to
		/// </summary>
		/// <param name="p"></param>
		/// <param name="from"></param>
		/// <param name="to"></param>
		/// <returns></returns>
		template <typename InPixelType, typename OutPixelType,
			typename FromProjection, typename ToProjection>
			static Pixel<OutPixelType> ReProject(Pixel<InPixelType> p,
				const FromProjection* from, const ToProjection* to)
		{
			Coordinate cc = to->template ProjectInverse<InPixelType, false>(p);
			return from->template Project<OutPixelType>(cc);
		};
	};

}

#endif
