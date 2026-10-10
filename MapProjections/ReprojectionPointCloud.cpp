#include "./ReprojectionPointCloud.h"

#ifndef MY_LOG_ERROR
#	define MY_LOG_ERROR(...) printf(__VA_ARGS__)
#endif

#include <string.h>

using namespace Projections;

ReprojectionPointCloud::ReprojectionPointCloud()
{
}


/// <summary>
/// Load reprojection info from file
/// </summary>
/// <param name="fileName"></param>
/// <returns></returns>
ReprojectionPointCloud ReprojectionPointCloud::CreateFromFile(const std::string& fileName)
{
	ReprojectionPointCloud r;
	
	FILE* f = nullptr;  //pointer to file we will read in
	my_fopen(&f, fileName.c_str(), "rb");
	if (f == nullptr)
	{
		MY_LOG_ERROR("Failed to open file: \"%s\"\n", fileName.c_str());
		return r;
	}

	fseek(f, 0L, SEEK_END);
	long size = ftell(f);
	fseek(f, 0L, SEEK_SET);

	long dataSize = size - 2 * sizeof(int);
	
	fread(&(r.outW), sizeof(int), 1, f);
	fread(&(r.outH), sizeof(int), 1, f);

	r.mapping.resize(dataSize / sizeof(Triangle));
	fread(&r.mapping[0], sizeof(Triangle), r.mapping.size(), f);

	fclose(f);

	return r;
}

/// <summary>
/// Save reprojection info to file
/// </summary>
/// <param name="fileName"></param>
void ReprojectionPointCloud::SaveToFile(const std::string& fileName)
{
	FILE* f = nullptr;
	my_fopen(&f, fileName.c_str(), "wb");

	if (f == nullptr)
	{
		MY_LOG_ERROR("Failed to open file %s (%s)", fileName.c_str(), strerror(errno));
		return;
	}

	fwrite(&this->outW, sizeof(int), 1, f);
	fwrite(&this->outH, sizeof(int), 1, f);

	fwrite(this->mapping.data(), sizeof(mapping), this->mapping.size(), f);
	fclose(f);

}


void ReprojectionPointCloud::Barycentric(
	const float ax, const float ay,
	const float bx, const float by,
	const float cx, const float cy,
	const float px, const float py,
	float* u, float* v, float* w)
{
	float v0x = bx - ax;
	float v0y = by - ay;

	float v1x = cx - ax;
	float v1y = cy - ay;

	float v2x = px - ax;
	float v2y = py - ay;

	float d00 = v0x * v0x + v0y * v0y;
	float d01 = v0x * v1x + v0y * v1y;
	float d11 = v1x * v1x + v1y * v1y;
	float d20 = v2x * v0x + v2y * v0y;
	float d21 = v2x * v1x + v2y * v1y;

	float denom = d00 * d11 - d01 * d01;
	*v = (d11 * d20 - d01 * d21) / denom;
	*w = (d00 * d21 - d01 * d20) / denom;
	*u = 1.0f - *v - *w;
}


float ReprojectionPointCloud::Sign(const float p1x, const float p1y,
	const float p2x, const float p2y,
	const float p3x, const float p3y)
{
	return (p1x - p3x) * (p2y - p3y) - (p2x - p3x) * (p1y - p3y);
}

/// <summary>
/// https://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle
/// </summary>
/// <param name="a"></param>
/// <param name="b"></param>
/// <param name="c"></param>
/// <param name="pt"></param>
/// <returns></returns>
bool ReprojectionPointCloud::PointInTriangle(
	const float ax, const float ay,
	const float bx, const float by,
	const float cx, const float cy,
	const float px, const float py
)
{

	float d1 = ReprojectionPointCloud::Sign(px, py, ax, ay, bx, by);
	float d2 = ReprojectionPointCloud::Sign(px, py, bx, by, cx, cy);
	float d3 = ReprojectionPointCloud::Sign(px, py, cx, cy, ax, ay);

	bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(has_neg && has_pos);
}
