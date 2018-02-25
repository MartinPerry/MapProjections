#include "./Projections.h"

using namespace Projections;

//=======================================================================
// Lambert-conic
//
// Based on:
// http://mathworld.wolfram.com/LambertConformalConicProjection.html
// https://en.wikipedia.org/wiki/Lambert_conformal_conic_projection (see only one standard parallel)
//=======================================================================

LambertConic::LambertConic(GeoCoordinate latProjOrigin, GeoCoordinate lonCentMeridian,
	GeoCoordinate stanParallel)
	: IProjectionInfo(PROJECTION::LAMBERT_CONIC),
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

ProjectedValue LambertConic::ProjectInternal(Coordinate c) const
{

	double t = cot(PI_4 + 0.5 * c.lat.rad());
	double phi = f * std::pow(t, n);

	double x = phi * std::sin(n * (c.lon.rad() - lonCentralMeridian.rad()));
	double y = phi0 - phi * std::cos(n * (c.lon.rad() - lonCentralMeridian.rad()));

	ProjectedValue p;
	p.x = x;
	p.y = y;
	return p;
}


IProjectionInfo<LambertConic>::ProjectedValueInverse LambertConic::ProjectInverseInternal(double x, double y) const
{

	double phi = sgn(n) * std::sqrt(x * x + (phi0 - y) * (phi0 - y));
	double delta = std::atan(x / (phi0 - y));

	double t = std::pow(f / phi, (1.0 / n));

	double lat = 2 * std::atan(t) - PI_2;
	double lon = lonCentralMeridian.rad() + delta / n;

	IProjectionInfo::ProjectedValueInverse c;
	c.lat = GeoCoordinate::rad(lat);
	c.lon = GeoCoordinate::rad(lon);

	return c;
}

//=======================================================================
// Mercator
//
// Based on:
// http://mathworld.wolfram.com/MercatorProjection.html
//=======================================================================

Mercator::Mercator()
	: IProjectionInfo(PROJECTION::MERCATOR)	
{

}

ProjectedValue Mercator::ProjectInternal(Coordinate c) const
{	
	ProjectedValue p;
	p.x = c.lon.rad();

	/*
	if (c.lat.rad() > 1.48352986) // > 85 deg
	{
		p.y = std::log(std::tan(PI_4 + 0.5 * 1.48352986));
	}
	else if (c.lat.rad() < -1.48352986) // < -85 deg
	{
		p.y = std::log(std::tan(PI_4 + 0.5 * -1.48352986));
	}
	else
	*/
	{
		p.y = std::log(std::tan(PI_4 + 0.5 * c.lat.rad()));
	}

	return p;
}


IProjectionInfo<Mercator>::ProjectedValueInverse Mercator::ProjectInverseInternal(double x, double y) const
{

	IProjectionInfo::ProjectedValueInverse c;
	c.lon = GeoCoordinate::rad(x);
	c.lat = GeoCoordinate::rad(2 * std::atan(std::pow(E, y)) - PI_2);
	

	return c;
}



//=======================================================================
// Equirectangular 
//
// Based on:
// https://en.wikipedia.org/wiki/Equirectangular_projection
//=======================================================================

Equirectangular::Equirectangular()
	: Equirectangular(0.0_deg)
{

}

Equirectangular::Equirectangular(GeoCoordinate lonCentralMeridian)
	: IProjectionInfo(PROJECTION::EQUIRECTANGULAR),
	lonCentralMeridian(lonCentralMeridian),
	standardParallel(0.0_deg),
	cosStandardParallel(std::cos(standardParallel.rad()))	
{
}

ProjectedValue Equirectangular::ProjectInternal(Coordinate c) const
{
	ProjectedValue p;
	p.x = (c.lon.rad() - lonCentralMeridian.rad()) * cosStandardParallel;
	p.y = c.lat.rad() - standardParallel.rad();

	return p;
}


IProjectionInfo<Equirectangular>::ProjectedValueInverse Equirectangular::ProjectInverseInternal(double x, double y) const
{

	IProjectionInfo::ProjectedValueInverse c;

	c.lat = GeoCoordinate::rad(y / cosStandardParallel + lonCentralMeridian.rad());
	c.lon = GeoCoordinate::rad(x + standardParallel.rad());

	return c;
}

//=======================================================================
// PolarSteregographic 
//
// Full computation:
// https://web.archive.org/web/20150723100408/http://www.knmi.nl/~beekhuis/rad_proj.html
// with Eccentricity is 1.0 => Based on:
// https://www.dwd.de/DE/leistungen/radolan/radolan_info/radolan_radvor_op_komposit_format_pdf.pdf?__blob=publicationFile&v=8
// 
//=======================================================================

PolarSteregographic::PolarSteregographic()
	: PolarSteregographic(10.0_deg, 60.0_deg)
{

}

PolarSteregographic::PolarSteregographic(GeoCoordinate lonCentralMeridian, GeoCoordinate latCentral)
	: IProjectionInfo(PROJECTION::POLAR_STEREOGRAPHICS),
	lonCentralMeridian(lonCentralMeridian), latCentral(latCentral)
{
}

ProjectedValue PolarSteregographic::ProjectInternal(Coordinate c) const
{
	double m = (1.0 + std::sin(latCentral.rad())) / (1.0 + std::sin(c.lat.rad()));
	double cosLat = std::cos(c.lat.rad());

	ProjectedValue p;
	p.x = +earthRadius * m * cosLat * std::sin(c.lon.rad() - lonCentralMeridian.rad());
	p.y = -earthRadius * m * cosLat * std::cos(c.lon.rad() - lonCentralMeridian.rad());

	return p;
}


IProjectionInfo<PolarSteregographic>::ProjectedValueInverse PolarSteregographic::ProjectInverseInternal(double x, double y) const
{

	ProjectedValueInverse c;

	c.lon = GeoCoordinate::rad(std::atan (-x / y) + lonCentralMeridian.rad());

	double tmpSin = std::sin(latCentral.rad());
	double tmp = earthRadius * earthRadius * (1.0 + tmpSin) * (1.0 + tmpSin);
	double tmp1 = tmp - (x * x + y * y);
	double tmp2 = tmp + (x * x + y * y);
	c.lat = GeoCoordinate::rad(std::asin(tmp1 / tmp2));
	

	return c;
}

