#include "./PointCloudTriangulation.h"



#include "./delaunator.hpp"


using namespace Projections;


std::vector<PointCloudTriangulation::TriangulationTriangle> PointCloudTriangulation::TriangulatePointCloud(
	const std::vector<float>& x, 
	const std::vector<float>& y)
{
	PointCloudTriangulation tri;
	tri.RunDelaunator(x, y);

	return tri.triangles;
}

std::vector<PointCloudTriangulation::TriangulationTriangle> PointCloudTriangulation::TriangulatePointCloud(
	const float* x, 
	const float* y, 
	size_t ptsCount)
{
	PointCloudTriangulation tri;
	tri.RunDelaunator(x, y, ptsCount);

	return tri.triangles;
}



void PointCloudTriangulation::RunDelaunator(const std::vector<float>& x, const std::vector<float>& y)
{
	if (x.size() != y.size())
	{
		return;
	}


	this->RunDelaunator(x.data(), y.data(), x.size());
}

void PointCloudTriangulation::RunDelaunator(const float* x, const float* y, size_t ptsCount)
{	
	std::vector<double> pts;

	for (size_t i = 0; i < ptsCount; i++)
	{
		pts.push_back(x[i]);
		pts.push_back(y[i]);
	}

	this->RunDelaunator(pts);
	
	//repair order of vertices

	for (size_t i = 0; i < this->triangles.size(); i++)
	{
		TriangulationTriangle& t = this->triangles[i];

		if (this->IsCW(x[t.index[A]], y[t.index[A]],
			x[t.index[B]], y[t.index[B]],
			x[t.index[C]], y[t.index[C]]))
		{
			continue;
		}

		std::swap(t.index[A], t.index[B]);
	}
}

void PointCloudTriangulation::RunDelaunator(const std::vector<double>& pts)
{
	auto delTri = delaunator::Delaunator(pts);

	
	for (size_t i = 0; i < delTri.triangles.size(); i += 3)
	{
		TriangulationTriangle tri;
		tri.index[A] = static_cast<int>(delTri.triangles[i]);
		tri.index[B] = static_cast<int>(delTri.triangles[i + 1]);
		tri.index[C] = static_cast<int>(delTri.triangles[i + 2]);

		tri.neighbour[A] = -1;
		tri.neighbour[B] = -1;
		tri.neighbour[C] = -1;

		this->triangles.push_back(tri);
	}
	
}

bool PointCloudTriangulation::IsCW(const float ax, const float ay,
	const float bx, const float by,
	const float cx, const float cy) const
{
	auto crossz = (bx - ax) * (cy - by) - (by - ay) * (cx - bx);

	if (crossz > 0.0)
	{
		// clockwise
		return true;
	}

	// counter-clockwise
	return false;
}

