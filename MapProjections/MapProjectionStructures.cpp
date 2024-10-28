#include "MapProjectionStructures.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <limits>
#include <algorithm>
#include <string.h>
#include <errno.h>

#ifndef MY_LOG_ERROR
#	define MY_LOG_ERROR(...) printf(__VA_ARGS__)
#endif

#ifdef HAVE_NEON
#	include "./simd/neon/neon_math_float.h"
#	include "./simd/neon/MapProjectionStructures_neon.h"
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
/// IS HAVE_NEON is defined - calculate 4 coordinates at once
/// 
/// Source:
/// https://vvvv.org/blog/polar-spherical-and-geographic-coordinates
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="z"></param>
/// <returns></returns>
std::array<Coordinate, 4> Coordinate::CreateFromCartesianLHSystem(
	const std::array<double, 4> & vx,
	const std::array<double, 4> & vy,
	const std::array<double, 4> & vz)
{
	std::array<Coordinate, 4> res;

#ifdef HAVE_NEON

	Projections::Neon::CoordinateNeon gpsNeon = Projections::Neon::CoordinateNeon::CreateFromCartesianLHSystem(vx, vy, vz);

	float resLat[4];
	float resLon[4];
	vst1q_f32(resLat, gpsNeon.latRad);
	vst1q_f32(resLon, gpsNeon.lonRad);
	
	res[0] = Coordinate(Longitude::rad(resLon[0]), Latitude::rad(resLat[0]));
	res[1] = Coordinate(Longitude::rad(resLon[1]), Latitude::rad(resLat[1]));
	res[2] = Coordinate(Longitude::rad(resLon[2]), Latitude::rad(resLat[2]));
	res[3] = Coordinate(Longitude::rad(resLon[3]), Latitude::rad(resLat[3]));
#else
	res[0] = Coordinate::CreateFromCartesianLHSystem(vx[0], vy[0], vz[0]);
	res[1] = Coordinate::CreateFromCartesianLHSystem(vx[1], vy[1], vz[1]);
	res[2] = Coordinate::CreateFromCartesianLHSystem(vx[2], vy[2], vz[2]);
	res[3] = Coordinate::CreateFromCartesianLHSystem(vx[3], vy[3], vz[3]);
#endif
	return res;
}

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
	return Coordinate::CreateFromCartesianLHSystem(x, y, z, &radius);
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
Coordinate Coordinate::CreateFromCartesianLHSystem(double x, double y, double z, double * radius)
{
	*radius = std::sqrt(x * x + y * y + z * z);
	return Coordinate::CreateFromCartesianLHSystem(x, y, z, *radius);
}

/// <summary>
/// Create Coordinate from cartexian [x, y, z] and precomputed radius
/// Input is assumed to be in left-handed coordinate system
/// 
/// Source:
/// https://vvvv.org/blog/polar-spherical-and-geographic-coordinates
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="z"></param>
/// <param name="radius"></param>
/// <returns></returns>
Coordinate Coordinate::CreateFromCartesianLHSystem(double x, double y, double z, double radius)
{
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
		Coordinate::PrecomputedSinCos tmp = this->PrecomputeSinCos();
		sinLat = tmp.sinLat;
		cosLat = tmp.cosLat;
		sinLon = tmp.sinLon;
		cosLon = tmp.cosLon;
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

#ifdef HAVE_NEON
	//we can use neon only to calculate 2 sin/cos at once
	//other two possible calculations are not used
	float32x4_t v = { 
		static_cast<float32_t>(lat.rad()), 
		static_cast<float32_t>(lon.rad()),
		static_cast<float32_t>(0.0), 
		static_cast<float32_t>(0.0) 
	};

	float32x4_t ysin;
	float32x4_t ycos;
	my_sincos_f32(v, &ysin, &ycos);

	float values[4];
	vst1q_f32(values, ysin);
	tmp.sinLat = values[0];
	tmp.sinLon = values[1];

	vst1q_f32(values, ycos);
	tmp.cosLat = values[0];
	tmp.cosLon = values[1];

#else
	tmp.sinLat = std::sin(lat.rad());
	tmp.cosLat = std::cos(lat.rad());
	tmp.sinLon = std::sin(lon.rad());
	tmp.cosLon = std::cos(lon.rad());
#endif
	return tmp;
}

/// <summary>
/// Prepare precalculated sin/cos - faster for NEON because we can utilise all
/// calculations
/// </summary>
/// <param name="coords"></param>
/// <returns></returns>
std::array<Coordinate::PrecomputedSinCos, 4> Coordinate::PrecalcMultipleSinCos(
	const std::array<Coordinate, 4> & coords)
{
	
#ifdef HAVE_NEON

	Projections::Neon::CoordinateNeon gpsNeon = Projections::Neon::CoordinateNeon::FromArray(coords);
	return Projections::Neon::CoordinateNeon::PrecalcMultipleSinCos(gpsNeon);

#else

	std::array<Coordinate::PrecomputedSinCos, 4> res;
	for (size_t i = 0; i < 4; i++)
	{
		res[i].sinLat = std::sin(coords[i].lat.rad());
		res[i].cosLat = std::cos(coords[i].lat.rad());
		res[i].sinLon = std::sin(coords[i].lon.rad());
		res[i].cosLon = std::cos(coords[i].lon.rad());
	}

	return res;

#endif	
}
