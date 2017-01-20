#include "./MapProjection.h"

#include <limits>
#include <algorithm>

const double IProjectionInfo::PI = std::acos(-1);
const double IProjectionInfo::PI_4 = 0.25 * IProjectionInfo::PI;
const double IProjectionInfo::PI_2 = 0.5 * IProjectionInfo::PI;

const double IProjectionInfo::E = std::exp(1.0);

const double IProjectionInfo::EARTH_RADIUS = 6371;

//=======================================================================
//
// Main interface
//
//=======================================================================

IProjectionInfo::IProjectionInfo(IProjectionInfo::PROJECTION curProjection)
	: curProjection(curProjection),
	min({ std::numeric_limits<double>::max(), std::numeric_limits<double>::max() }),
	max({ std::numeric_limits<double>::min(), std::numeric_limits<double>::min() }),
	w(0), h(0),
	wPadding(0), hPadding(0),
	wAR(1), hAR(1),
	stepLat(0.0_deg), stepLon(0.0_deg)
{
}

/// <summary>
/// [Static] 
/// Normalize lon to be [-180, 180]
/// </summary>
/// <param name="lonDeg"></param>
/// <returns></returns>
double IProjectionInfo::NormalizeLon(double lonDeg)
{
	return std::fmod(lonDeg + 540, 360) - 180;
}

/// <summary>
/// [Static]
/// Normalize lat to be [-90, 90]
/// </summary>
/// <param name="latDeg"></param>
/// <returns></returns>
double IProjectionInfo::NormalizeLat(double latDeg)
{
	return (latDeg > 90) ? (latDeg - 180) : latDeg;
}

/// <summary>
/// [Static] 
/// Compute AABB from coordinates 
/// </summary>
/// <param name="c">input coordinates</param>
/// <param name="min">output AABB min</param>
/// <param name="max">output AABB max</param>
void IProjectionInfo::ComputeAABB(const std::vector<IProjectionInfo::Coordinate> & c,
	IProjectionInfo::Coordinate & min, IProjectionInfo::Coordinate & max)
{
	min = c[0];
	max = c[0];
	for (size_t i = 1; i < c.size(); i++)
	{
		if (c[i].lat.rad() < min.lat.rad()) min.lat = c[i].lat;
		if (c[i].lon.rad() < min.lon.rad()) min.lon = c[i].lon;


		if (c[i].lat.rad() > max.lat.rad()) max.lat = c[i].lat;
		if (c[i].lon.rad() > max.lon.rad()) max.lon = c[i].lon;

	}

}

/// <summary>
/// Set current data active frame based on existing projection
/// </summary>
/// <param name="proj">existing projection</param>
/// <param name="w">frame width</param>
/// <param name="h">frame height</param>
/// <param name="keepAR">keep AR of data (default: true) 
/// if yes, data are enlarged and not 1:1 to bounding box to keep AR
/// </param>
void IProjectionInfo::SetFrame(IProjectionInfo * proj,
	int w, int h, bool keepAR)
{
	IProjectionInfo::Coordinate cMin, cMax;
	proj->ComputeAABB(cMin, cMax);

	this->SetFrame(cMin, cMax, w, h, keepAR);
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
void IProjectionInfo::SetFrame(Coordinate minCoord, Coordinate maxCoord, 
	int w, int h, bool keepAR)
{
		
	this->min = { std::numeric_limits<double>::max(), std::numeric_limits<double>::max() };
	this->max = { std::numeric_limits<double>::min(), std::numeric_limits<double>::min() };

	ProjectedValue minPixel = this->ProjectInternal(minCoord);
	ProjectedValue maxPixel = this->ProjectInternal(maxCoord);
	
	this->min.x = std::min(std::min(this->min.x, minPixel.x), maxPixel.x);
	this->min.y = std::min(std::min(this->min.y, minPixel.y), maxPixel.y);


	//-----------------------------------------------------------

	minPixel.x = minPixel.x - this->min.x;
	minPixel.y = minPixel.y - this->min.y;

	maxPixel.x = maxPixel.x - this->min.x;
	maxPixel.y = maxPixel.y - this->min.y;


	this->max.x = std::max(std::max(this->max.x, minPixel.x), maxPixel.x);
	this->max.y = std::max(std::max(this->max.y, minPixel.y), maxPixel.y);

	//----------------------------------------------------------
	
	this->w = w;
	this->h = h;

	this->wAR = this->w / this->max.x;
	this->hAR = this->h / this->max.y;


	// Using different ratios for width and height will cause the map to be stretched,
	// but fitting the desired region
	// If we want to keep AR of the map, the AR will be correct, but frame will no
	// be the one, we have set. One dimmension will be changed to keep AR	
	if (keepAR)
	{
		double globalAR = std::min(this->wAR, this->hAR);
		this->wAR = globalAR;
		this->hAR = globalAR;
	}
	
	this->wPadding = (this->w - (this->wAR * this->max.x)) * 0.5;
	this->hPadding = (this->h - (this->hAR * this->max.y)) * 0.5;


	this->stepLat = GeoCoordinate::rad((maxCoord.lat.rad() - minCoord.lat.rad()) / h);
	this->stepLon = GeoCoordinate::rad((maxCoord.lon.rad() - minCoord.lon.rad()) / w);
}

/// <summary>
/// Get projection top left corner
/// </summary>
/// <returns></returns>
IProjectionInfo::Coordinate IProjectionInfo::GetTopLeftCorner() const
{
	return this->ProjectInverse({ 0, 0 });
}

/// <summary>
/// Project Coordinate point to pixel
/// </summary>
/// <param name="c"></param>
/// <returns></returns>
IProjectionInfo::Pixel IProjectionInfo::Project(Coordinate c) const
{
	
	ProjectedValue raw = this->ProjectInternal(c);


	raw.x = raw.x - this->min.x;
	raw.y = raw.y - this->min.y;

	IProjectionInfo::Pixel p;
	p.x = static_cast<int>(this->wPadding + (raw.x * this->wAR));
	p.y = static_cast<int>(this->h - this->hPadding - (raw.y * this->hAR));
	
	return p;
}

/// <summary>
/// Project pixel to coordinate
/// </summary>
/// <param name="p"></param>
/// <returns></returns>
IProjectionInfo::Coordinate IProjectionInfo::ProjectInverse(Pixel p) const
{
	
	double xx = (static_cast<double>(p.x) - this->wPadding + this->wAR * this->min.x);
	xx /= this->wAR;

	double yy = -1 * (static_cast<double>(p.y) - this->h + this->hPadding - this->hAR * this->min.y);
	yy /= this->hAR;
	

	ProjectedValueInverse pi = this->ProjectInverseInternal(xx, yy);

	Coordinate c;
	c.lat = GeoCoordinate::deg(NormalizeLat(pi.latDeg));
	c.lon = GeoCoordinate::deg(NormalizeLon(pi.lonDeg));
	return c;
}

/// <summary>
/// Calculate end point based on shortest path (on real earth surface)
/// see: http://www.movable-type.co.uk/scripts/latlong.html
/// </summary>
/// <param name="start"></param>
/// <param name="bearing"></param>
/// <param name="dist"></param>
/// <returns></returns>
IProjectionInfo::Coordinate IProjectionInfo::CalcEndPointShortest(IProjectionInfo::Coordinate start,
	Angle bearing, double dist) const
{
	
	double dr = dist / EARTH_RADIUS;
	
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

	
	IProjectionInfo::Coordinate end;
	end.lat = GeoCoordinate::rad(endLat);
	end.lon = GeoCoordinate::deg(NormalizeLon(radToDeg(endLon)));

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
IProjectionInfo::Coordinate IProjectionInfo::CalcEndPointDirect(
	IProjectionInfo::Coordinate start,
	Angle bearing, double dist) const
{	
	double dr = dist / EARTH_RADIUS;

	double difDr = dr * std::cos(bearing.rad());
	double endLat = start.lat.rad() + difDr;

	// check for some daft bugger going past the pole, normalise latitude if so
	if (std::abs(endLat) > PI_2) endLat = endLat > 0 ? PI - endLat : -PI - endLat;


	double projLatDif = std::log(std::tan(endLat / 2 + PI_4) / std::tan(start.lat.rad() / 2 + PI_4));
	double q = std::abs(projLatDif) > 10e-12 ? difDr / projLatDif : std::cos(start.lat.rad()); // E-W course becomes ill-conditioned with 0/0

	double difDrQ = dr * std::sin(bearing.rad()) / q;
	double endLon = start.lon.rad() + difDrQ;

	

	IProjectionInfo::Coordinate end;
	end.lat = GeoCoordinate::rad(endLat);
	end.lon = GeoCoordinate::deg(NormalizeLon(radToDeg(endLon)));

	return end;
}

/// <summary>
/// Get pixels on line. For each pixel, callback function is called
/// </summary>
/// <param name="start"></param>
/// <param name="end"></param>
/// <param name="callback"></param>
void IProjectionInfo::LineBresenham(Pixel start, Pixel end,
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
/// Re-project data from current to imProj. 
/// Calculates mapping: newData[index] = oldData[reprojection[index]]
/// </summary>
/// <param name="imProj"></param>
/// <returns></returns>
std::vector<IProjectionInfo::Pixel> IProjectionInfo::CreateReprojection(IProjectionInfo * imProj) const
{
	std::vector<IProjectionInfo::Pixel> reprojection;

	for (int y = 0; y < this->GetFrameHeight(); y++)
	{
		for (int x = 0; x < this->GetFrameWidth(); x++)
		{
			reprojection.push_back({ -1, -1 });
		}
	}


	for (int y = 0; y < this->GetFrameHeight(); y++)
	{
		for (int x = 0; x < this->GetFrameWidth(); x++)
		{

			IProjectionInfo::Coordinate cc = this->ProjectInverse({ x,y });
			IProjectionInfo::Pixel p = imProj->Project(cc);

			if (p.x < 0) continue;
			if (p.y < 0) continue;
			if (p.x >= imProj->GetFrameWidth()) continue;
			if (p.y >= imProj->GetFrameHeight()) continue;

			reprojection[x + y * this->GetFrameWidth()] = p;

		}
	}

	return reprojection;
}

/// <summary>
/// Compute AABB for current active frame
/// It uses Bresenham lines around the image 
/// E.g.: [0,0] -> [0, h] and reproject each pixel, and from those, AABB is calculated
/// </summary>
/// <param name="min"></param>
/// <param name="max"></param>
void IProjectionInfo::ComputeAABB(IProjectionInfo::Coordinate & min, IProjectionInfo::Coordinate & max) const
{
	int ww = this->w - 1;
	int hh = this->h - 1;

	std::vector<IProjectionInfo::Coordinate> border;
	this->LineBresenham({ 0,0 }, { 0, hh },
		[&](int x, int y) -> void {
		IProjectionInfo::Coordinate c = this->ProjectInverse({ x, y });
		border.push_back(c);
	});
	this->LineBresenham({ 0,0 }, { ww, 0 },
		[&](int x, int y) -> void {
		IProjectionInfo::Coordinate c = this->ProjectInverse({ x, y });
		border.push_back(c);
	});
	this->LineBresenham({ ww, hh }, { 0, hh },
		[&](int x, int y) -> void {
		IProjectionInfo::Coordinate c = this->ProjectInverse({ x, y });
		border.push_back(c);
	});
	this->LineBresenham({ ww, hh }, { 0, hh },
		[&](int x, int y) -> void {
		IProjectionInfo::Coordinate c = this->ProjectInverse({ x, y });
		border.push_back(c);
	});

	
	IProjectionInfo::ComputeAABB(border, min, max);
}