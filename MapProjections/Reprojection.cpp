#include "./Reprojection.h"

#ifndef MY_LOG_ERROR
#	define MY_LOG_ERROR(...) printf(__VA_ARGS__)
#endif

#include <string.h>

using namespace Projections;

template <typename T>
Reprojection<T>::Reprojection() :
	inW(0),
	inH(0),
	outW(0),
	outH(0)
{
}

/// <summary>
/// Clamp input fromData to map only from sub-image (sub-region)
/// point at startX, startY will become [0, 0]
/// Usefull when we calculate reprojection for large area image
/// but want to do actuall reprojection only for a smaller image
/// that was created from the large one
/// </summary>
/// <param name="startX"></param>
/// <param name="startY"></param>
/// <param name="endX"></param>
/// <param name="endY"></param>
template <typename T>
void Reprojection<T>::ClampInputToSubImage(int startX, int startY, int endX, int endY)
{
	int w = endX - startX;
	int h = endY - startY;

	for (auto& v : this->pixels)
	{
		v.x -= startX;
		v.y -= startY;

		if ((v.x < 0) || (v.y < 0) || (v.x >= w) || (v.y >= h))
		{
			v.x = -1;
			v.y = -1;
		}
	}
	this->inW = w;
	this->inH = h;
}


/// <summary>
/// Load reprojection info from file
/// </summary>
/// <param name="fileName"></param>
/// <returns></returns>
template <typename T>
Reprojection<T> Reprojection<T>::CreateFromFile(const std::string& fileName)
{
	Reprojection r;
	r.inH = 0;
	r.inW = 0;
	r.outW = 0;
	r.outH = 0;

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

	long dataSize = size - 4 * sizeof(int);

	fread(&(r.inW), sizeof(int), 1, f);
	fread(&(r.inH), sizeof(int), 1, f);
	fread(&(r.outW), sizeof(int), 1, f);
	fread(&(r.outH), sizeof(int), 1, f);

	r.pixels.resize(dataSize / sizeof(Pixel<T>));
	fread(&r.pixels[0], sizeof(Pixel<T>), r.pixels.size(), f);

	fclose(f);

	return r;
}

/// <summary>
/// Save reprojection info to file
/// </summary>
/// <param name="fileName"></param>
template <typename T>
void Reprojection<T>::SaveToFile(const std::string& fileName)
{
	FILE* f = nullptr;
	my_fopen(&f, fileName.c_str(), "wb");

	if (f == nullptr)
	{
		MY_LOG_ERROR("Failed to open file %s (%s)", fileName.c_str(), strerror(errno));
		return;
	}
	fwrite(&this->inW, sizeof(int), 1, f);
	fwrite(&this->inH, sizeof(int), 1, f);
	fwrite(&this->outW, sizeof(int), 1, f);
	fwrite(&this->outH, sizeof(int), 1, f);
	fwrite(this->pixels.data(), sizeof(Pixel<T>), this->pixels.size(), f);
	fclose(f);

}


template struct Projections::Reprojection<int>;
template struct Projections::Reprojection<short>;
template struct Projections::Reprojection<float>;
