#ifndef _PROJECTIONS_H_
#define _PROJECTIONS_H_

#include "GeoCoordinate.h"
#include "MapProjection.h"

namespace Projections
{

	class LambertConic : public IProjectionInfo<LambertConic>
	{
	public:
		LambertConic(GeoCoordinate latProjOrigin,
			GeoCoordinate lonCentMeridian,
			GeoCoordinate stanParallel);

		friend class IProjectionInfo<LambertConic>;

	protected:
		GeoCoordinate latProjectionOrigin;
		GeoCoordinate lonCentralMeridian;
		GeoCoordinate standardParallel1;
		GeoCoordinate standardParallel2;


		double f;
		double n;
		double phi0;

		ProjectedValue ProjectInternal(Coordinate c) const;
		ProjectedValueInverse ProjectInverseInternal(double x, double y) const;

	};


	class Mercator : public IProjectionInfo<Mercator>
	{
	public:
		Mercator();

		friend class IProjectionInfo<Mercator>;

	protected:
		ProjectedValue ProjectInternal(Coordinate c) const;
		ProjectedValueInverse ProjectInverseInternal(double x, double y) const;

	};



	class Equirectangular : public IProjectionInfo<Equirectangular>
	{
	public:
		Equirectangular();
		Equirectangular(GeoCoordinate lonCentralMeridian);

		friend class IProjectionInfo<Equirectangular>;

	protected:

		GeoCoordinate lonCentralMeridian;
		GeoCoordinate standardParallel;
		double cosStandardParallel;

		ProjectedValue ProjectInternal(Coordinate c) const;
		ProjectedValueInverse ProjectInverseInternal(double x, double y) const;

	};


	class PolarSteregographic : public IProjectionInfo<PolarSteregographic>
	{
	public:
		PolarSteregographic();
		PolarSteregographic(GeoCoordinate lonCentralMeridian, GeoCoordinate latCentral);

		friend class IProjectionInfo<PolarSteregographic>;

	protected:
		const double earthRadius = 6370.04;

		GeoCoordinate lonCentralMeridian;
		GeoCoordinate latCentral;

		ProjectedValue ProjectInternal(Coordinate c) const;
		ProjectedValueInverse ProjectInverseInternal(double x, double y) const;

	};

};

#endif
