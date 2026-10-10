#ifndef POINTCLOUD_TRIANGULATION_H
#define POINTCLOUD_TRIANGULATION_H

#include <functional>
#include <vector>


namespace Projections
{

	class PointCloudTriangulation
	{
	public:

		//Result of triangulation
		struct TriangulationTriangle
		{
			int index[3];	 //index of vertices - a, b, c
			int neighbour[3];  // adjacent edges - index to neighbouring triangle - bc, ac, ab
		};

		~PointCloudTriangulation() = default;

		static std::vector<TriangulationTriangle> TriangulatePointCloud(const std::vector<float>& x, const std::vector<float>& y);
		static std::vector<TriangulationTriangle> TriangulatePointCloud(const float* x, const float* y, size_t ptsCount);


	private:

		static const int A = 0;
		static const int B = 1;
		static const int C = 2;

		PointCloudTriangulation() = default;

		std::vector<TriangulationTriangle> triangles;

		bool IsCW(const float ax, const float ay,
			const float bx, const float by,
			const float cx, const float cy) const;

		void RunDelaunator(const std::vector<float>& x, const std::vector<float>& y);
		void RunDelaunator(const float* x, const float* y, size_t ptsCount);
		void RunDelaunator(const std::vector<double>& pts);
	};
}


#endif

