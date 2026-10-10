#ifndef REPROJECTION_POINT_CLOUD_H
#define REPROJECTION_POINT_CLOUD_H

#include <vector>


#include "./MapProjectionStructures.h"
#include "./ProjectionInfo.h"

#include "./Triangulation/PointCloudTriangulation.h"

namespace Projections
{
	
	struct ReprojectionPointCloud
	{
		struct Triangle
		{
			float ax;
			float ay;
			float bx;
			float by;
			float cx;
			float cy;

			int ai;
			int bi;
			int ci;

			float u;
			float v;
			float w;
		};

		int outW;
		int outH;
		std::vector<Triangle> mapping;
		
		ReprojectionPointCloud();
		~ReprojectionPointCloud() = default;
		
		void SaveToFile(const std::string& fileName);

		static ReprojectionPointCloud CreateFromFile(const std::string& fileName);

	protected:
		static void Barycentric(
			const float ax, const float ay,
			const float bx, const float by,
			const float cx, const float cy,
			const float px, const float py,
			float* u, float* v, float* w);

		static bool PointInTriangle(
			const float ax, const float ay,
			const float bx, const float by,
			const float cx, const float cy,
			const float px, const float py
		);

		static float Sign(const float p1x, const float p1y,
			const float p2x, const float p2y,
			const float p3x, const float p3y);

		//====================================================================================

	public:

		/// <summary>
		/// Create cache name in format:
		/// reproj_from_w_h_to_w_h
		/// </summary>
		/// <param name="from"></param>
		/// <param name="to"></param>
		/// <returns></returns>
		template <typename ToProjection>
		static std::string CreateCacheName(ToProjection* to)
		{
			std::string tmp = "reproj_pc_";
						
			tmp += to->GetName();
#ifdef USE_VIRTUAL_INTERFACE
			tmp += "_";
			tmp += std::to_string(static_cast<int>(to->GetFrame().w));
			tmp += "_";
			tmp += std::to_string(static_cast<int>(to->GetFrame().h));
			tmp += std::to_string(to->GetFrame().GetId());
#endif

			return tmp;
		}


		template <typename ToProjection>
		static ReprojectionPointCloud CreateReprojection(size_t dataCount, float* latData, float* lonData, ToProjection* to)
		{
			ReprojectionPointCloud r;
			r.outW = to->GetFrameWidth();
			r.outH = to->GetFrameHeight();
			r.mapping = std::vector<Triangle>(r.outW * r.outH);

			std::vector<float> ptx;
			std::vector<float> pty;

			for (int index = 0; index < dataCount; index++)
			{
				double lon = lonData[index];
				double lat = latData[index];

				Projections::Coordinate c;
				c.lon = Longitude::deg(lon);
				c.lat = Latitude::deg(lat);
				auto px = to->Project<float>(c);

				if ((px.x < 0) || (px.x >= r.outW) ||
					(px.y < 0) || (px.y >= r.outH))
				{
					continue;
				}

				ptx.push_back(px.x);
				pty.push_back(px.y);
			}

			auto tris = PointCloudTriangulation::TriangulatePointCloud(ptx, pty);
			
			
			for (int y = 0; y < r.outH; y++)
			{
				//printf("%d / %d\n", y, r.outH);
				for (int x = 0; x < r.outW; x++)
				{
					int outIndex = x + y * r.outW;

					for (size_t i = 0; i < tris.size(); i++)
					{
						int ai = tris[i].index[0];

						if (std::abs(ptx[ai] - x) > 10)
						{
							//too far
							continue;
						}

						if (std::abs(pty[ai] - y) > 10)
						{
							//too far
							continue;
						}

						int bi = tris[i].index[1];
						int ci = tris[i].index[2];


						if (ReprojectionPointCloud::PointInTriangle(ptx[ai], pty[ai],
							ptx[bi], pty[bi],
							ptx[ci], pty[ci],
							x, y)
							)
						{
	
							float u, v, w;
							ReprojectionPointCloud::Barycentric(
								ptx[ai], pty[ai],
								ptx[bi], pty[bi],
								ptx[ci], pty[ci],
								x, y,
								&u, &v, &w);

							r.mapping[outIndex] = {
								ptx[ai], pty[ai],
								ptx[bi], pty[bi],
								ptx[ci], pty[ci],
								ai, bi, ci,
								u, v, w
							};
							break;
						}
					}
				}
			}

			return r;
		}
		
		template <typename DataType, typename Out = DataType*, size_t ChannelsCount = 1>
		Out ReprojectData(const DataType* inputData, const DataType NO_VALUE) const
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

			for (int y = 0; y < this->outH; y++)
			{
				for (int x = 0; x < this->outW; x++)
				{
					int index = x + y * this->outW;
					const auto& tri = mapping[index];

					DataType val_a = inputData[tri.ai];
					DataType val_b = inputData[tri.bi];
					DataType val_c = inputData[tri.ci];

					DataType val_p = tri.u * val_a + tri.v * val_b + tri.w * val_c;

					output[index] = val_p;
				}
			}

			return output;
		}
	};

}

#endif
