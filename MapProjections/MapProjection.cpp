#include "./MapProjection.h"

#include <limits>
#include <algorithm>
#include <cstring>
#include <cassert>

#include "Projections.h"
#include "MapProjectionUtils.h"

using namespace Projections;


//=======================================================================
//
// Main interface
//
//=======================================================================

template <typename Proj>
ProjectionInfo<Proj>::ProjectionInfo(PROJECTION curProjection)
	: IProjectionInfo(curProjection)	
	//max({ std::numeric_limits<double>::min(), std::numeric_limits<double>::min() }),
{
	frame.minPixelOffset = { std::numeric_limits<double>::max(), std::numeric_limits<double>::max() };
	frame.w = 0;
	frame.h = 0;
	frame.wPadding = 0;
	frame.hPadding = 0;
	frame.wAR = 1;
	frame.hAR = 1;	
	this->frame.projInvPrecomW = 0.0;
	this->frame.projInvPrecomH = 0.0;
}



template <typename Proj>
void ProjectionInfo<Proj>::SetFrame(const ProjectionFrame & frame)
{
	this->frame.h = frame.h;
	this->frame.w = frame.w;
	this->frame.hAR = frame.hAR;
	this->frame.wAR = frame.wAR;
	this->frame.hPadding = frame.hPadding;
	this->frame.wPadding = frame.wPadding;
	this->frame.minPixelOffset = frame.minPixelOffset;	
	this->frame.projInvPrecomW = frame.projInvPrecomW;
	this->frame.projInvPrecomH = frame.projInvPrecomH;
}



/// <summary>
/// Set current data active frame based on AABB that si calculatef from set of points
/// </summary>
/// <param name="coord">set of GPS points</param>
/// <param name="w">frame width</param>
/// <param name="h">frame height</param>
/// <param name="keepAR">keep AR of data (default: true) 
/// if yes, data are enlarged beyond AABB to keep AR
/// </param>
template <typename Proj>
void ProjectionInfo<Proj>::SetFrame(std::vector<Coordinate> coord,
	double w, double h, bool keepAR)
{
	Coordinate minCoord = coord[0];
	Coordinate maxCoord = coord[0];
	

	for (Coordinate & c : coord)
	{
		if (c.lat.rad() > maxCoord.lat.rad())
		{
			maxCoord.lat = c.lat;
		}
		if (c.lon.rad() > maxCoord.lon.rad())
		{
			maxCoord.lon = c.lon;
		}


		if (c.lat.rad() < minCoord.lat.rad())
		{
			minCoord.lat = c.lat;
		}
		if (c.lon.rad() < minCoord.lon.rad())
		{
			minCoord.lon = c.lon;
		}
	}

	this->SetFrame(minCoord, maxCoord, w, h, keepAR);
}

/// <summary>
/// Set current data active frame based on AABB (minCoord, maxCoord)
/// </summary>
/// <param name="minCoord">AABB min</param>
/// <param name="maxCoord">AABB max</param>
/// <param name="w">frame width</param>
/// <param name="h">frame height</param>
/// <param name="keepAR">keep AR of data (default: true) 
/// if yes, data are enlarged beyond AABB to keep AR
/// </param>
template <typename Proj>
void ProjectionInfo<Proj>::SetFrame(Coordinate minCoord, Coordinate maxCoord,
	double w, double h, bool keepAR)
{
		
	//calculate minimum internal projection value
	ProjectedValue  minPixel = { std::numeric_limits<double>::max(), std::numeric_limits<double>::max() };
	
	ProjectedValue tmpMinPixel = static_cast<Proj*>(this)->ProjectInternal(minCoord);
	ProjectedValue tmpMaxPixel = static_cast<Proj*>(this)->ProjectInternal(maxCoord);
	
	minPixel.x = std::min(tmpMinPixel.x, tmpMaxPixel.x);
	minPixel.y = std::min(tmpMinPixel.y, tmpMaxPixel.y);

	frame.minPixelOffset = minPixel;

	//-----------------------------------------------------------
	//calculate maximum internal projection value

	//move origin to [0, 0]
	tmpMinPixel.x = tmpMinPixel.x - minPixel.x;
	tmpMinPixel.y = tmpMinPixel.y - minPixel.y;

	//now, minPixel should be [0,0]
	assert(tmpMinPixel.x == 0);
	assert(tmpMinPixel.y == 0);

	//move max accordingly
	tmpMaxPixel.x = tmpMaxPixel.x - minPixel.x;
	tmpMaxPixel.y = tmpMaxPixel.y - minPixel.y;

	//calculate moved maximum
	ProjectedValue maxPixel = { std::numeric_limits<double>::min(), std::numeric_limits<double>::min() };

	maxPixel.x = std::max(tmpMinPixel.x, tmpMaxPixel.x);
	maxPixel.y = std::max(tmpMinPixel.y, tmpMaxPixel.y);

	//----------------------------------------------------------
	
	this->frame.min = minCoord;
	this->frame.max = maxCoord;

	this->frame.w = w;
	this->frame.h = h;

	//calculate scale in width and height
	this->frame.wAR = this->frame.w / maxPixel.x; //minPixel.x == 0 => no need to calc difference abs(min - max)
	this->frame.hAR = this->frame.h / maxPixel.y; //minPixel.y == 0 => no need to calc difference abs(min - max)
	
	this->frame.wPadding = 0;
	this->frame.hPadding = 0;

	// Using different ratios for width and height will cause the map to be stretched,
	// but fitting the desired region
	// If we want to keep AR of the map, the AR will be correct, but frame will no
	// be the one, we have set. One dimmension will be changed to keep AR	
	if (keepAR)
	{
		double globalAR = std::min(this->frame.wAR, this->frame.hAR);
		this->frame.wAR = globalAR;
		this->frame.hAR = globalAR;

		this->frame.wPadding = (this->frame.w - (this->frame.wAR * maxPixel.x)) * 0.5;
		this->frame.hPadding = (this->frame.h - (this->frame.hAR * maxPixel.y)) * 0.5;
	}
	
	this->frame.projInvPrecomW = -this->frame.wPadding + this->frame.wAR * this->frame.minPixelOffset.x;
	this->frame.projInvPrecomH = -this->frame.h + this->frame.hPadding - this->frame.hAR * this->frame.minPixelOffset.y;
}

/// <summary>
/// calculate how many "degrees" is for one "pixel" in lat / lon
/// cordinates are boundaries of pixels :
/// 
/// Eg:
/// Image is 2x2 px
///    -180  +180
///  +90 -------
///      |  |  |
///      -------
///      |  |  |
///  -90 -------
/// Step latitude : (90 - (-90)) / 2 = 90
/// Step longitude : (180 - (-180)) / 2 = 180
/// </summary>
/// <returns></returns>
template <typename Proj>
Coordinate ProjectionInfo<Proj>::CalcStep(STEP_TYPE type) const
{
	int dif = (type == STEP_TYPE::PIXEL_BORDER) ? 0 : 1;
	
	Coordinate step;
	step.lat = GeoCoordinate::rad((this->frame.max.lat.rad() - this->frame.min.lat.rad()) / (this->frame.h - dif));
	step.lon = GeoCoordinate::rad((this->frame.max.lon.rad() - this->frame.min.lon.rad()) / (this->frame.w - dif));

	return step;
}

/// <summary>
/// Get projection top left corner
/// </summary>
/// <returns></returns>
template <typename Proj>
Coordinate ProjectionInfo<Proj>::GetTopLeftCorner() const
{
	return this->ProjectInverse({ 0, 0 });
}

template <typename Proj>
const ProjectionFrame & ProjectionInfo<Proj>::GetFrame() const
{
	return this->frame;
}

/// <summary>
/// Calculate end point based on shortest path (on real earth surface)
/// see: http://www.movable-type.co.uk/scripts/latlong.html
/// </summary>
/// <param name="start"></param>
/// <param name="bearing"></param>
/// <param name="dist"></param>
/// <returns></returns>
template <typename Proj>
Coordinate ProjectionInfo<Proj>::CalcEndPointShortest(Coordinate start,
	Angle bearing, double dist) const
{
	
    double dr = dist / ProjectionConstants::EARTH_RADIUS;
	
	double sinLat = std::sin(start.lat.rad());
	double cosLat = std::cos(start.lat.rad());
	double sinDr = std::sin(dr);
	double cosDr = std::cos(dr);
	double sinBear = std::sin(bearing.rad());
	double cosBear = std::cos(bearing.rad());

	double sinEndLat = sinLat * cosDr + cosLat * sinDr * cosBear;
	double endLat = std::asin(sinEndLat);
	double y = sinBear * sinDr * cosLat;
	double x = cosDr - sinLat * sinLat;
	double endLon = start.lon.rad() + std::atan2(y, x);

	
	Coordinate end;
	end.lat = GeoCoordinate::rad(endLat);
    end.lon = GeoCoordinate::deg(ProjectionUtils::NormalizeLon(ProjectionUtils::radToDeg(endLon)));

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
template <typename Proj>
Coordinate ProjectionInfo<Proj>::CalcEndPointDirect(
	Coordinate start, Angle bearing, double dist) const
{	
	double dr = dist / ProjectionConstants::EARTH_RADIUS;

	double difDr = dr * std::cos(bearing.rad());
	double endLat = start.lat.rad() + difDr;

	// check for some daft bugger going past the pole, normalise latitude if so
	if (std::abs(endLat) > ProjectionConstants::PI_2) endLat = endLat > 0 ? ProjectionConstants::PI - endLat : -ProjectionConstants::PI - endLat;


	double projLatDif = std::log(std::tan(endLat / 2 + ProjectionConstants::PI_4) / std::tan(start.lat.rad() / 2 + ProjectionConstants::PI_4));
	double q = std::abs(projLatDif) > 10e-12 ? difDr / projLatDif : std::cos(start.lat.rad()); // E-W course becomes ill-conditioned with 0/0

	double difDrQ = dr * std::sin(bearing.rad()) / q;
	double endLon = start.lon.rad() + difDrQ;

	

	Coordinate end;
	end.lat = GeoCoordinate::rad(endLat);
	end.lon = GeoCoordinate::deg(ProjectionUtils::NormalizeLon(ProjectionUtils::radToDeg(endLon)));

	return end;
}

/// <summary>
/// Get pixels on line. For each pixel, callback function is called
/// </summary>
/// <param name="start"></param>
/// <param name="end"></param>
/// <param name="callback"></param>
template <typename Proj>
void ProjectionInfo<Proj>::LineBresenham(Pixel<int> start, Pixel<int> end,
	std::function<void(int x, int y)> callback) const
{
	if ((start.x >= static_cast<int>(this->GetFrameWidth()))
		|| (start.y >= static_cast<int>(this->GetFrameHeight())))
	{
		return;
	}

	if ((end.x >= static_cast<int>(this->GetFrameWidth()))
		|| (end.y >= static_cast<int>(this->GetFrameHeight())))
	{
		return;
	}

	if ((start.x < 0)
		|| (start.y < 0))
	{
		return;
	}

	if ((end.x < 0)
		|| (end.y < 0))
	{
		return;
	}




	int dx = std::abs(end.x - start.x);
	int dy = std::abs(end.y - start.y);
	int sx, sy, e2;


	(start.x < end.x) ? sx = 1 : sx = -1;
	(start.y < end.y) ? sy = 1 : sy = -1;
	int err = dx - dy;

	while (1)
	{		
		callback(start.x, start.y);
		if ((start.x == end.x) && (start.y == end.y))
		{
			break;
		}
		e2 = 2 * err;
		if (e2 > -dy)
		{
			err = err - dy;
			start.x = start.x + sx;
		}
		if (e2 < dx)
		{
			err = err + dx;
			start.y = start.y + sy;
		}
	}
}



/// <summary>
/// Compute AABB for current active frame
/// It uses Bresenham lines around the image 
/// E.g.: [0,0] -> [0, h] and reproject each pixel, and from those, AABB is calculated
/// </summary>
/// <param name="min"></param>
/// <param name="max"></param>
template <typename Proj>
void ProjectionInfo<Proj>::ComputeAABB(Coordinate & min, Coordinate & max) const
{
	int ww = static_cast<int>(this->frame.w - 1);
	int hh = static_cast<int>(this->frame.h - 1);

	std::vector<Coordinate> border;
	this->LineBresenham({ 0,0 }, { 0, hh },
		[&](int x, int y) -> void {
		Coordinate c = this->ProjectInverse({ x, y });
		border.push_back(c);
	});
	this->LineBresenham({ 0,0 }, { ww, 0 },
		[&](int x, int y) -> void {
		Coordinate c = this->ProjectInverse({ x, y });
		border.push_back(c);
	});
	this->LineBresenham({ ww, hh }, { 0, hh },
		[&](int x, int y) -> void {
		Coordinate c = this->ProjectInverse({ x, y });
		border.push_back(c);
	});
	this->LineBresenham({ ww, hh }, { 0, hh },
		[&](int x, int y) -> void {
		Coordinate c = this->ProjectInverse({ x, y });
		border.push_back(c);
	});

	
	ProjectionUtils::ComputeAABB(border, min, max);
}




//=====

template class Projections::ProjectionInfo<LambertConic>;
template class Projections::ProjectionInfo<Mercator>;
template class Projections::ProjectionInfo<Equirectangular>;
template class Projections::ProjectionInfo<PolarSteregographic>;
