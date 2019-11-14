#include "./ProjectionInfo.h"

#include <limits>
#include <algorithm>
#include <cstring>
#include <cassert>

#include "./Projections/Mercator.h"
#include "./Projections/Miller.h"
#include "./Projections/LambertConic.h"
#include "./Projections/Equirectangular.h"
#include "./Projections/PolarSteregographic.h"
#include "./Projections/GOES.h"

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
{
	this->frame.minPixelOffsetX = std::numeric_limits<MyRealType>::max();
	this->frame.minPixelOffsetY = std::numeric_limits<MyRealType>::max();
	this->frame.w = 0;
	this->frame.h = 0;
	this->frame.wPadding = 0;
	this->frame.hPadding = 0;
	this->frame.wAR = 1;
	this->frame.hAR = 1;
	this->frame.projPrecomX = 0.0;
	this->frame.projPrecomY = 0.0;

	this->frame.min.lat = -90.0_deg;
	this->frame.max.lat = 90.0_deg;

	this->frame.min.lon = -180.0_deg;
	this->frame.max.lon = 180.0_deg;
}

template <typename Proj>
std::tuple<double, double, double, double> ProjectionInfo<Proj>::GetFrameBotLeftTopRight(
	const Coordinate & botLeft, const Coordinate & topRight)
{
	ProjectedValue tmpMinPixel = static_cast<Proj*>(this)->ProjectInternal(botLeft);
	ProjectedValue tmpMaxPixel = static_cast<Proj*>(this)->ProjectInternal(topRight);

	ProjectedValue minVal, maxVal;
	
	minVal.x = std::min(tmpMinPixel.x, tmpMaxPixel.x);
	minVal.y = std::min(tmpMinPixel.y, tmpMaxPixel.y);

	maxVal.x = std::max(tmpMinPixel.x, tmpMaxPixel.x);
	maxVal.y = std::max(tmpMinPixel.y, tmpMaxPixel.y);

	return std::make_tuple(minVal.x, minVal.y, maxVal.x, maxVal.y);
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
	this->frame.minPixelOffsetX = frame.minPixelOffsetX;
    this->frame.minPixelOffsetY = frame.minPixelOffsetY;
	this->frame.projPrecomX = frame.projPrecomX;
	this->frame.projPrecomY = frame.projPrecomY;

	this->ComputeAABB(this->frame.min, this->frame.max);
}



/// <summary>
/// Set current data active frame based on image bottom left and
/// top right coordinate
/// 
/// This assumes that data are plotted in 2D image and image has corners
/// These corners do not have to correspond to AABB of coordinates
/// For example: Lambert - image is square but corners are not AABB
/// 
/// </summary>
/// <param name="botLeft">Bottom left coordinate</param>
/// <param name="topRight">Top right coordinate</param>
/// <param name="w">frame width</param>
/// <param name="h">frame height</param>
/// <param name="keepAR">keep AR of data (default: true) 
/// if yes, data are enlarged beyond AABB to keep AR
/// </param>
template <typename Proj>
void ProjectionInfo<Proj>::SetFrame(const Coordinate & botLeft, const Coordinate & topRight,
	MyRealType w, MyRealType h, bool keepAR)
{		
	//calculate minimum internal projection value				

	auto [minX, minY, maxX, maxY] = static_cast<Proj*>(this)->GetFrameBotLeftTopRight(botLeft, topRight);


	//store minimal value of [x, y] from internal projection
	frame.minPixelOffsetX = minX;
	frame.minPixelOffsetY = minY;

	//Calculate width / height of internal projection		
	MyRealType projW = maxX - frame.minPixelOffsetX;
	MyRealType projH = maxY - frame.minPixelOffsetY;
			
	//----------------------------------------------------------
		
	this->frame.w = w;
	this->frame.h = h;

	//calculate scale in width and height
	this->frame.wAR = this->frame.w / projW;
	this->frame.hAR = this->frame.h / projH;
	
	this->frame.wPadding = 0;
	this->frame.hPadding = 0;

	// Using different ratios for width and height will cause the map to be stretched,
	// but fitting the desired region
	// If we want to keep AR of the map, the AR will be correct, but frame will no
	// be the one, we have set. One dimmension will be changed to keep AR	
	if (keepAR)
	{
		MyRealType globalAR = std::min(this->frame.wAR, this->frame.hAR);
		this->frame.wAR = globalAR;
		this->frame.hAR = globalAR;

		this->frame.wPadding = (this->frame.w - (this->frame.wAR * projW)) * MyRealType(0.5);
		this->frame.hPadding = (this->frame.h - (this->frame.hAR * projH)) * MyRealType(0.5);
	}
	
	this->frame.projPrecomX = -this->frame.wPadding + this->frame.wAR * this->frame.minPixelOffsetX;
	this->frame.projPrecomY = -this->frame.h + this->frame.hPadding - this->frame.hAR * this->frame.minPixelOffsetY;

	this->ComputeAABB(this->frame.min, this->frame.max);
}


/// <summary>
/// Set current data active frame based on AABB
/// For projections that have orthogonal lat/lon,
/// this is same as SetFrame, since botLeft = min and topRight = max
/// 
/// However, for other projections like Lambert, this will behave differently
/// 
/// </summary>
/// <param name="min"></param>
/// <param name="max"></param>
/// <param name="w"></param>
/// <param name="h"></param>
/// <param name="keepAR"></param>
template <typename Proj>
void ProjectionInfo<Proj>::SetFrameFromAABB(const Coordinate & min, const Coordinate & max,
	MyRealType w, MyRealType h, bool keepAR)
{
	//not working correctly for non-orthogonal lat/lon projections !!!!!
	//todo
	this->SetFrame(min, max, w, h, keepAR);
}

/// <summary>
/// calculate how many "degrees" is for one "pixel" in lat / lon
/// cordinates are boundaries of pixels :
/// This is correct only for projections that have orthogonal lat / lon
/// (like Mercator), but not for eg. Lambert
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
	step.lat = Latitude::rad((this->frame.max.lat.rad() - this->frame.min.lat.rad()) / (this->frame.h - dif));
	step.lon = Longitude::rad((this->frame.max.lon.rad() - this->frame.min.lon.rad()) / (this->frame.w - dif));

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
Coordinate ProjectionInfo<Proj>::CalcEndPointShortest(const Coordinate & start,
	const Angle & bearing, MyRealType dist) const
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
template <typename Proj>
Coordinate ProjectionInfo<Proj>::CalcEndPointDirect(
	const Coordinate & start, const Angle & bearing, MyRealType dist) const
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
	//[0, 0] is at pixel corner
	//[w, h] is at pixel corner
	//If we use [w - 1, h - 1] it correspond to [0, 0] at pixel center
	int ww = static_cast<int>(this->frame.w);
	int hh = static_cast<int>(this->frame.h);
	
	std::vector<Coordinate> border;

	if (static_cast<const Proj*>(this)->ORTHOGONAL_LAT_LON)
	{
		Coordinate c = this->ProjectInverse({ 0, 0 });
		border.push_back(std::move(c));

		c = this->ProjectInverse({ ww, hh });
		border.push_back(std::move(c));
	}
	else
	{
		this->LineBresenham({ 0,0 }, { 0, hh },
			[&](int x, int y) -> void {
				Coordinate c = this->ProjectInverse({ x, y });
				border.push_back(std::move(c));
			});
		this->LineBresenham({ 0,0 }, { ww, 0 },
			[&](int x, int y) -> void {
				Coordinate c = this->ProjectInverse({ x, y });
				border.push_back(std::move(c));
			});
		this->LineBresenham({ ww, hh }, { 0, hh },
			[&](int x, int y) -> void {
				Coordinate c = this->ProjectInverse({ x, y });
				border.push_back(std::move(c));
			});
		this->LineBresenham({ ww, hh }, { 0, hh },
			[&](int x, int y) -> void {
				Coordinate c = this->ProjectInverse({ x, y });
				border.push_back(std::move(c));
			});
	}
	
	ProjectionUtils::ComputeAABB(border, min, max);
}




//=====

template class Projections::ProjectionInfo<LambertConic>;
template class Projections::ProjectionInfo<Mercator>;
template class Projections::ProjectionInfo<Miller>;
template class Projections::ProjectionInfo<Equirectangular>;
template class Projections::ProjectionInfo<PolarSteregographic>;
template class Projections::ProjectionInfo<GOES>;
