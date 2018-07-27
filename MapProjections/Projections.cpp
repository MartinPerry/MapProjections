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
	: ProjectionInfo(PROJECTION::LAMBERT_CONIC),
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
        double t1 = std::cos(standardParallel1.rad()) * ProjectionUtils::sec(standardParallel2.rad());
		double t2 = std::tan(ProjectionConstants::PI_4 + 0.5 * standardParallel2.rad());
        double t3 = ProjectionUtils::cot(ProjectionConstants::PI_4 + 0.5 * standardParallel1.rad());
		n = std::log(t1) / std::log(t2 * t3);
	}

	double t4 = std::tan(ProjectionConstants::PI_4 + 0.5 * standardParallel1.rad());
    double t5 = ProjectionUtils::cot(ProjectionConstants::PI_4 + 0.5 * latProjectionOrigin.rad());

	f = std::cos(standardParallel1.rad()) * std::pow(t4, n) / n;
	phi0 = f * std::pow(t5, n);

}


//=======================================================================
// Mercator
//
// Based on:
// http://mathworld.wolfram.com/MercatorProjection.html
//=======================================================================

Mercator::Mercator()
	: ProjectionInfo(PROJECTION::MERCATOR)
{

}

//=======================================================================
// Miller
//
// Based on:
// https://en.wikipedia.org/wiki/Miller_cylindrical_projection
//=======================================================================

Miller::Miller()
    : ProjectionInfo(PROJECTION::MILLER)
{
    
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
	: ProjectionInfo(PROJECTION::EQUIRECTANGULAR),
	lonCentralMeridian(lonCentralMeridian),
	standardParallel(0.0_deg),
	cosStandardParallel(std::cos(standardParallel.rad()))	
{
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
	: ProjectionInfo(PROJECTION::POLAR_STEREOGRAPHICS),
	lonCentralMeridian(lonCentralMeridian), latCentral(latCentral)
{
}


