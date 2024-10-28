#ifndef REPROJECTION_NEON_H
#define REPROJECTION_NEON_H

#include <vector>

#include "./neon_utils.h"

#include "../../MapProjectionStructures.h"
#include "./ProjectionInfo_neon.h"

#include "../../Reprojection.h"

namespace Projections::Neon
{
	template <typename T = int>
	struct Reprojection : public Projections::Reprojection<T>
	{
		/// <summary>
		/// Re-project data from -> to
		/// Calculates mapping: toData[index] = fromData[reprojection[index]]
		/// </summary>
		/// <param name="imProj"></param>
		/// <returns></returns>
		template <typename FromProjection, typename ToProjection>
		static Reprojection<T> CreateReprojection(FromProjection* from, ToProjection* to)
		{
			//Latitude (y) is usually more complex to calculate

			Reprojection<T> reprojection;
			reprojection.pixels.resize(to->GetFrameHeight() * to->GetFrameWidth(), { -1, -1 });

			int wRest4 = to->GetFrameWidth() % 4;
			int w4 = to->GetFrameWidth() - wRest4;

			int hRest4 = to->GetFrameHeight() % 4;
			int h4 = to->GetFrameHeight() - hRest4;

			//if x and y are independent, simplify
			if ((from->IsIndependentLatLon()) && (to->IsIndependentLatLon()))
			{
				std::vector<T> cacheX;
				cacheX.resize(to->GetFrameWidth());

				std::vector<T> cacheY;
				cacheY.resize(to->GetFrameHeight());

				for (int x = 0; x < w4; x += 4)
				{
					std::array<Projections::Pixel<int>, 4> p;
					p[0] = { x, 0 };
					p[1] = { x + 1, 0 };
					p[2] = { x + 2, 0 };
					p[3] = { x + 3, 0 };
					
					std::array<Projections::Pixel<T>, 4> o = ReProject<int, T>(p, from, to);
					for (size_t i = 0; i < o.size(); i++)
					{
						cacheX[(x + i)] = o[i].x;
					}
				}

				for (int x = w4; x < to->GetFrameWidth(); x++)
				{
					Projections::Pixel<T> p = Projections::Reprojection<T>::ReProject<int, T>({ x, 0 }, from, to);
					cacheX[x] = p.x;
				}


				for (int y = 0; y < h4; y += 4)
				{
					std::array<Projections::Pixel<int>, 4> p;
					p[0] = { 0, y };
					p[1] = { 0, y + 1 };
					p[2] = { 0, y + 2 };
					p[3] = { 0, y + 3 };
					
					std::array<Projections::Pixel<T>, 4> o = ReProject<int, T>(p, from, to);
					for (size_t i = 0; i < o.size(); i++)
					{
						cacheY[(y + i)] = o[i].y;
					}
				}

				for (int y = h4; y < to->GetFrameHeight(); y++)
				{
					Projections::Pixel<T> p = Projections::Reprojection<T>::ReProject<int, T>({ 0, y }, from, to);
					cacheY[y] = p.y;

				}


				for (int y = 0; y < to->GetFrameHeight(); y++)
				{
					for (int x = 0; x < to->GetFrameWidth(); x++)
					{
						Pixel<T> p;
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
					for (int x = 0; x < w4; x += 4)
					{
						std::array<Projections::Pixel<int>, 4> p;
						p[0] = { x,y };
						p[1] = { x + 1,y };
						p[2] = { x + 2,y };
						p[3] = { x + 3,y };
						
						std::array<Projections::Pixel<T>, 4> o = ReProject<int, T>(p, from, to);

						for (size_t i = 0; i < o.size(); i++)
						{
							if (o[i].x < 0) continue;
							if (o[i].y < 0) continue;
							if (o[i].x >= from->GetFrameWidth()) continue;
							if (o[i].y >= from->GetFrameHeight()) continue;

							reprojection.pixels[(x + i) + y * to->GetFrameWidth()] = o[i];
						}
					}

					for (int x = w4; x < to->GetFrameWidth(); x++)
					{
						Projections::Pixel<T> p = Projections::Reprojection<T>::ReProject<int, T>({ x, y }, from, to);

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


		template <typename InPixelType, typename OutPixelType,
			typename FromProjection, typename ToProjection>
			static std::array<Projections::Pixel<OutPixelType>, 4> ReProject(const std::array<Projections::Pixel<InPixelType>, 4>& p,
				const FromProjection* from,
				const ToProjection* to)
		{
			PixelNeon pNeon = PixelNeon::FromArray<InPixelType>(p);

			auto cc = to->ProjectInverse(pNeon);
			auto tmp = from->Project(cc);

			return PixelNeon::ToArray<OutPixelType>(tmp);
		};

	};
}

#endif