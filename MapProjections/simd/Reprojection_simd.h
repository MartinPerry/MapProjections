#ifndef REPROJECTION_SIMD_H
#define REPROJECTION_SIMD_H

#include <vector>


#include "../MapProjectionStructures.h"
#include "./ProjectionInfo_simd.h"

#include "../Reprojection.h"

namespace Projections::Simd
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

			int wRest8 = to->GetFrameWidth() % 8;
			int w8 = to->GetFrameWidth() - wRest8;

			int hRest8 = to->GetFrameHeight() % 8;
			int h8 = to->GetFrameHeight() - hRest8;

			//if x and y are independent, simplify
			if ((from->IsIndependentLatLon()) && (to->IsIndependentLatLon()))
			{
				std::vector<T> cacheX;
				cacheX.resize(to->GetFrameWidth());

				std::vector<T> cacheY;
				cacheY.resize(to->GetFrameHeight());

				for (int x = 0; x < w8; x += 8)
				{
					std::array<Projections::Pixel<int>, 8> p;
					p[0] = { x, 0 };
					p[1] = { x + 1, 0 };
					p[2] = { x + 2, 0 };
					p[3] = { x + 3, 0 };
					p[4] = { x + 4, 0 };
					p[5] = { x + 5, 0 };
					p[6] = { x + 6, 0 };
					p[7] = { x + 7, 0 };

					std::array<Projections::Pixel<T>, 8> o = ReProject<int, T>(p, from, to);
					for (size_t i = 0; i < o.size(); i++)
					{
						cacheX[(x + i)] = o[i].x;
					}
				}

				for (int x = w8; x < to->GetFrameWidth(); x++)
				{
					Projections::Pixel<T> p = Projections::Reprojection<T>::ReProject<int, T>({ x, 0 }, from, to);
					cacheX[x] = p.x;
				}


				for (int y = 0; y < h8; y += 8)
				{
					std::array<Projections::Pixel<int>, 8> p;
					p[0] = { 0, y };
					p[1] = { 0, y + 1 };
					p[2] = { 0, y + 2 };
					p[3] = { 0, y + 3 };
					p[4] = { 0, y + 4 };
					p[5] = { 0, y + 5 };
					p[6] = { 0, y + 6 };
					p[7] = { 0, y + 7 };

					std::array<Projections::Pixel<T>, 8> o = ReProject<int, T>(p, from, to);
					for (size_t i = 0; i < o.size(); i++)
					{
						cacheY[(y + i)] = o[i].y;
					}
				}

				for (int y = h8; y < to->GetFrameHeight(); y++)
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
					for (int x = 0; x < w8; x += 8)
					{
						std::array<Projections::Pixel<int>, 8> p;
						p[0] = { x,y };
						p[1] = { x + 1,y };
						p[2] = { x + 2,y };
						p[3] = { x + 3,y };
						p[4] = { x + 4,y };
						p[5] = { x + 5,y };
						p[6] = { x + 6,y };
						p[7] = { x + 7,y };

						std::array<Projections::Pixel<T>, 8> o = ReProject<int, T>(p, from, to);

						for (size_t i = 0; i < o.size(); i++)
						{
							if (o[i].x < 0) continue;
							if (o[i].y < 0) continue;
							if (o[i].x >= from->GetFrameWidth()) continue;
							if (o[i].y >= from->GetFrameHeight()) continue;

							reprojection.pixels[(x + i) + y * to->GetFrameWidth()] = o[i];
						}
					}

					for (int x = w8; x < to->GetFrameWidth(); x++)
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
			static std::array<Projections::Pixel<OutPixelType>, 8> ReProject(const std::array<Projections::Pixel<InPixelType>, 8>& p,
				const FromProjection* from,
				const ToProjection* to)
		{
			PixelSimd pSimd = PixelSimd::FromArray<InPixelType>(p);

			auto cc = to->ProjectInverse(pSimd);
			auto tmp = from->Project(cc);

			return PixelSimd::ToArray<OutPixelType>(tmp);
		};

	};
}

#endif