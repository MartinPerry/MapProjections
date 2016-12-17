#ifndef _PROJECTIONS_H_
#define _PROJECTIONS_H_

#include "MapProjection.h"

class LambertConic : public IProjectionInfo
{
public:
	LambertConic(GeoCoordinate latProjOrigin,
		GeoCoordinate lonCentMeridian,
		GeoCoordinate stanParallel);


protected:
	GeoCoordinate latProjectionOrigin;
	GeoCoordinate lonCentralMeridian;
	GeoCoordinate standardParallel1;
	GeoCoordinate standardParallel2;


	double f;
	double n;
	double phi0;

	virtual IProjectionInfo::ProjectedValue ProjectInternal(Coordinate c) const;
	virtual IProjectionInfo::ProjectedValueInverse ProjectInverseInternal(double x, double y) const;

};


class Mercator : public IProjectionInfo
{
public:
	Mercator();


protected:

	virtual IProjectionInfo::ProjectedValue ProjectInternal(Coordinate c) const;
	virtual IProjectionInfo::ProjectedValueInverse ProjectInverseInternal(double x, double y) const;

};


#endif
