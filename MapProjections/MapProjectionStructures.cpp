#include "MapProjectionStructures.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <limits>
#include <algorithm>
#include <string.h>
#include <errno.h>

#ifndef MY_LOG_ERROR
#define MY_LOG_ERROR(...) printf(__VA_ARGS__)
#endif

using namespace Projections;

const double ProjectionConstants::PI = double(std::acos(-1));
const double ProjectionConstants::PI_4 = double(0.25) * ProjectionConstants::PI;
const double ProjectionConstants::PI_2 = double(0.5) * ProjectionConstants::PI;
const double ProjectionConstants::E = double(std::exp(1.0));
const double ProjectionConstants::EARTH_RADIUS = double(6371);


/// <summary>
/// Create Coordinate from cartexian [x, y, z]. 
/// Input is assumed to be in left-handed coordinate system
/// 
/// Source:
/// https://vvvv.org/blog/polar-spherical-and-geographic-coordinates
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="z"></param>
/// <returns></returns>
Coordinate Coordinate::CreateFromCartesianLHSystem(double x, double y, double z)
{
	double radius;
	return Coordinate::CreateFromCartesianLHSystem(x, y, z, radius);
};

/// <summary>
/// Create Coordinate from cartexian [x, y, z]. 
/// Input is assumed to be in left-handed coordinate system
/// 
/// Return also radius of converted position
/// 
/// Source:
/// https://vvvv.org/blog/polar-spherical-and-geographic-coordinates
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="z"></param>
/// <param name="radius"></param>
/// <returns></returns>
Coordinate Coordinate::CreateFromCartesianLHSystem(double x, double y, double z, double & radius)
{
	radius = std::sqrt(x * x + y * y + z * z);
	double lat = std::asin(y / radius);
	double lon = std::atan2(x, -z);
	return Coordinate(Longitude::rad(lon), Latitude::rad(lat));
}

/// <summary>
/// Convert cartesian 3D vector to (lat, lon) vector
/// in left-handed system
/// 
/// Based on:
/// https://www.brown.edu/Departments/Engineering/Courses/En221/Notes/Polar_Coords/Polar_Coords.htm
/// (modified for left-handed system and for direct use of lat/lon)
/// </summary>
/// <param name="dx"></param>
/// <param name="dy"></param>
/// <param name="dz"></param>
/// <param name="precomp"></param>
/// <returns></returns>
Coordinate Coordinate::ConvertVectorFromCartesianLHSystem(double dx, double dy, double dz,
	const Coordinate::PrecomputedSinCos * precomp) const
{
	double rDif;
	return Coordinate::ConvertVectorFromCartesianLHSystem(dx, dy, dz, rDif, precomp);
}

/// <summary>
/// Convert cartesian 3D vector to (lat, lon) vector
/// in left-handed system
/// 
/// Return also rDif - difference vector of radius (= movement along the radius)
/// 
/// Based on:
/// https://www.brown.edu/Departments/Engineering/Courses/En221/Notes/Polar_Coords/Polar_Coords.htm
/// (modified for left-handed system and for direct use of lat/lon)
/// </summary>
/// <param name="dx"></param>
/// <param name="dy"></param>
/// <param name="dz"></param>
/// <param name="rDif"></param>
/// <param name="precomp"></param>
/// <returns></returns>
Coordinate Coordinate::ConvertVectorFromCartesianLHSystem(double dx, double dy, double dz,
	double & rDif,
	const Coordinate::PrecomputedSinCos * precomp) const
{
	double sinLat, cosLat, sinLon, cosLon;

	if (precomp != nullptr)
	{
		sinLat = precomp->sinLat;
		cosLat = precomp->cosLat;
		sinLon = precomp->sinLon;
		cosLon = precomp->cosLon;
	}
	else
	{
		sinLat = std::sin(lat.rad());
		cosLat = std::cos(lat.rad());
		sinLon = std::sin(lon.rad());
		cosLon = std::cos(lon.rad());
	}

	rDif = cosLat * sinLon * dx + sinLat * dy - cosLat * cosLon * dz;
	double latDif = -sinLat * sinLon * dx + cosLat * dy + sinLat * cosLon * dz;
	double lonDif = cosLon * dx + sinLon * dz;

	return Coordinate(Longitude::rad(lonDif), Latitude::rad(latDif));
}

/// <summary>
/// Precompute sin/cos for lat/lon that can be used for repeated 
/// multiple computations
/// 
/// </summary>
/// <returns></returns>
Coordinate::PrecomputedSinCos Coordinate::PrecomputeSinCos() const
{
	Coordinate::PrecomputedSinCos tmp;
	tmp.sinLat = std::sin(lat.rad());
	tmp.cosLat = std::cos(lat.rad());
	tmp.sinLon = std::sin(lon.rad());
	tmp.cosLon = std::cos(lon.rad());
	return tmp;
}



/// <summary>
/// Load reprojection info from file
/// </summary>
/// <param name="fileName"></param>
/// <returns></returns>
Reprojection Reprojection::CreateFromFile(const std::string & fileName)
{
	Reprojection r;
	r.inH = 0;
	r.inW = 0;
	r.outW = 0;
	r.outH = 0;

	FILE * f = nullptr;  //pointer to file we will read in
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

	r.pixels.resize(dataSize / sizeof(Pixel<int>));
	fread(&r.pixels[0], sizeof(Pixel<int>), r.pixels.size(), f);

	fclose(f);

	return r;
}

/// <summary>
/// Save reprojection info to file
/// </summary>
/// <param name="fileName"></param>
void Reprojection::SaveToFile(const std::string & fileName)
{
	FILE * f = nullptr;
	//my_fopen(&f, fileName.c_str(), "wb");
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
	fwrite(this->pixels.data(), sizeof(Pixel<int>), this->pixels.size(), f);
	fclose(f);

}
