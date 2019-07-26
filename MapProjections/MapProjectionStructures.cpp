#include "MapProjectionStructures.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <limits>
#include <algorithm>
#include <string.h>
#include <errno.h>


using namespace Projections;

const MyRealType ProjectionConstants::PI = std::acos(-1);
const MyRealType ProjectionConstants::PI_4 = MyRealType(0.25) * ProjectionConstants::PI;
const MyRealType ProjectionConstants::PI_2 = MyRealType(0.5) * ProjectionConstants::PI;
const MyRealType ProjectionConstants::E = std::exp(1.0);
const MyRealType ProjectionConstants::EARTH_RADIUS = 6371;




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
        printf("Failed to open file: \"%s\"\n", fileName.c_str());
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
        printf("Failed to open file %s (%s)", fileName.c_str(), strerror(errno));
        return;
    }
    fwrite(&this->inW, sizeof(int), 1, f);
    fwrite(&this->inH, sizeof(int), 1, f);
    fwrite(&this->outW, sizeof(int), 1, f);
    fwrite(&this->outH, sizeof(int), 1, f);
    fwrite(this->pixels.data(), sizeof(Pixel<int>), this->pixels.size(), f);
    fclose(f);
    
}
