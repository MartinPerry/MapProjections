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

const MyRealType ProjectionConstants::PI = MyRealType(std::acos(-1));
const MyRealType ProjectionConstants::PI_4 = MyRealType(0.25) * ProjectionConstants::PI;
const MyRealType ProjectionConstants::PI_2 = MyRealType(0.5) * ProjectionConstants::PI;
const MyRealType ProjectionConstants::E = MyRealType(std::exp(1.0));
const MyRealType ProjectionConstants::EARTH_RADIUS = MyRealType(6371);


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
Coordinate Coordinate::CreateFromCartesianLHSystem(MyRealType x, MyRealType y, MyRealType z)
{
	MyRealType radius = std::sqrt(x * x + y * y + z * z);
	MyRealType lat = std::asin(y / radius);
	MyRealType lon = std::atan2(x, -z);

	return Coordinate(Longitude::rad(lon), Latitude::rad(lat));
};

/// <summary>
/// Convert latitude / longitude to left-handed cartesian coordinate system
/// Radius is used as scaled earth radius
/// 
/// Source:
/// https://vvvv.org/blog/polar-spherical-and-geographic-coordinates
/// </summary>
/// <param name="radius"></param>
/// <returns></returns>
std::tuple<MyRealType, MyRealType, MyRealType> Coordinate::ConvertToCartesianLHSystem(MyRealType radius)
{
	MyRealType cosLat = std::cos(lat.rad());
	
	MyRealType x = radius * cosLat * std::sin(lon.rad());
	MyRealType y = radius * std::sin(lat.rad());
	MyRealType z = -radius * cosLat * std::cos(lon.rad());

	return { x, y, z };
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
