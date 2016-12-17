#include "./Projections.h"



//=======================================================================
// Lambert-conic
//
// Based on:
// http://mathworld.wolfram.com/LambertConformalConicProjection.html
// https://en.wikipedia.org/wiki/Lambert_conformal_conic_projection (see only one standard parallel)
//=======================================================================

LambertConic::LambertConic(GeoCoordinate latProjOrigin, GeoCoordinate lonCentMeridian,
	GeoCoordinate stanParallel)
	: IProjectionInfo(IProjectionInfo::PROJECTION::LAMBERT_CONIC),
	latProjectionOrigin(latProjOrigin),
	lonCentralMeridian(lonCentMeridian),
	standardParallel1(stanParallel),
	standardParallel2(stanParallel)
{


	if (standardParallel1.rad() == standardParallel2.rad())
	{
		n = std::sin(standardParallel1.rad());
	}
	else
	{
		double t1 = std::cos(standardParallel1.rad()) * sec(standardParallel2.rad());
		double t2 = std::tan(PI_4 + 0.5 * standardParallel2.rad());
		double t3 = cot(PI_4 + 0.5 * standardParallel1.rad());
		n = std::log(t1) / std::log(t2 * t3);
	}

	double t4 = std::tan(PI_4 + 0.5 * standardParallel1.rad());
	double t5 = cot(PI_4 + 0.5 * latProjectionOrigin.rad());

	f = std::cos(standardParallel1.rad()) * std::pow(t4, n) / n;
	phi0 = f * std::pow(t5, n);

}

IProjectionInfo::ProjectedValue LambertConic::ProjectInternal(Coordinate c) const
{

	double t = cot(PI_4 + 0.5 * c.lat.rad());
	double phi = f * std::pow(t, n);

	double x = phi * std::sin(n * (c.lon.rad() - lonCentralMeridian.rad()));
	double y = phi0 - phi * std::cos(n * (c.lon.rad() - lonCentralMeridian.rad()));

	IProjectionInfo::ProjectedValue p;
	p.x = x;
	p.y = y;
	return p;
}


IProjectionInfo::ProjectedValueInverse LambertConic::ProjectInverseInternal(double x, double y) const
{

	double phi = sgn(n) * std::sqrt(x * x + (phi0 - y) * (phi0 - y));
	double delta = std::atan(x / (phi0 - y));

	double t = std::pow(f / phi, (1.0 / n));

	double lat = 2 * std::atan(t) - PI_2;
	double lon = lonCentralMeridian.rad() + delta / n;

	IProjectionInfo::ProjectedValueInverse c;
	c.lat = radToDeg(lat);
	c.lon = radToDeg(lon);

	return c;
}

//=======================================================================
// Mercator
//
// Based on:
// http://mathworld.wolfram.com/MercatorProjection.html
//=======================================================================

Mercator::Mercator()
	: IProjectionInfo(IProjectionInfo::PROJECTION::MERCATOR)
{

}

IProjectionInfo::ProjectedValue Mercator::ProjectInternal(Coordinate c) const
{
	IProjectionInfo::ProjectedValue p;
	p.x = c.lon.rad();
	p.y = std::log(std::tan(PI_4 + 0.5 * c.lat.rad()));

	return p;
}


IProjectionInfo::ProjectedValueInverse Mercator::ProjectInverseInternal(double x, double y) const
{

	IProjectionInfo::ProjectedValueInverse c;

	c.lat = 2 * std::atan(std::pow(E, y)) - PI_2;
	c.lon = x;

	return c;
}


