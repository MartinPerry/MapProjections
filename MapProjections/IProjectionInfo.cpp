#include "./IProjectionInfo.h"

#include "./MapProjectionUtils.h"

using namespace Projections;

/// <summary>
/// Calculate end point based on shortest path (on real earth surface)
/// see: http://www.movable-type.co.uk/scripts/latlong.html
/// </summary>
/// <param name="start"></param>
/// <param name="bearing"></param>
/// <param name="dist"></param>
/// <returns></returns>
Coordinate IProjectionInfo::CalcEndPointShortest(const Coordinate & start,
	const Angle & bearing, MyRealType dist)
{

	MyRealType dr = dist / ProjectionConstants::EARTH_RADIUS;

	MyRealType sinLat = std::sin(start.lat.rad());
	MyRealType cosLat = std::cos(start.lat.rad());
	MyRealType sinDr = std::sin(dr);
	MyRealType cosDr = std::cos(dr);
	MyRealType sinBear = std::sin(bearing.rad());
	MyRealType cosBear = std::cos(bearing.rad());

	MyRealType sinEndLat = sinLat * cosDr + cosLat * sinDr * cosBear;
	MyRealType endLat = std::asin(sinEndLat);
	MyRealType y = sinBear * sinDr * cosLat;
	MyRealType x = cosDr - sinLat * sinLat;
	MyRealType endLon = start.lon.rad() + std::atan2(y, x);


	Coordinate end;
	end.lat = Latitude::rad(endLat);
	end.lon = Longitude::deg(ProjectionUtils::NormalizeLon(ProjectionUtils::radToDeg(endLon)));

	return end;

}

/// <summary>
/// "Rhumb lines"
/// Calculate end point based on direct path (straight line between two points in projected earth)
/// </summary>
/// <param name="start"></param>
/// <param name="bearing"></param>
/// <param name="dist"></param>
/// <returns></returns>
Coordinate IProjectionInfo::CalcEndPointDirect(
	const Coordinate & start, const Angle & bearing, MyRealType dist)
{
	MyRealType dr = dist / ProjectionConstants::EARTH_RADIUS;

	MyRealType difDr = dr * std::cos(bearing.rad());
	MyRealType endLat = start.lat.rad() + difDr;

	// check for some daft bugger going past the pole, normalise latitude if so
	if (std::abs(endLat) > ProjectionConstants::PI_2) endLat = endLat > 0 ? ProjectionConstants::PI - endLat : -ProjectionConstants::PI - endLat;


	MyRealType projLatDif = std::log(std::tan(endLat / 2 + ProjectionConstants::PI_4) / std::tan(start.lat.rad() / 2 + ProjectionConstants::PI_4));
	MyRealType q = std::abs(projLatDif) > 10e-12 ? difDr / projLatDif : std::cos(start.lat.rad()); // E-W course becomes ill-conditioned with 0/0

	MyRealType difDrQ = dr * std::sin(bearing.rad()) / q;
	MyRealType endLon = start.lon.rad() + difDrQ;



	Coordinate end;
	end.lat = Latitude::rad(endLat);
	end.lon = Longitude::deg(ProjectionUtils::NormalizeLon(ProjectionUtils::radToDeg(endLon)));

	return end;
}
