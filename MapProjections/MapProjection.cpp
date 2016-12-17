#include "./MapProjection.h"


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


double IProjectionInfo::NormalizeLon(double lon)
{
	return std::fmod(lon + 540, 360) - 180;
}


double IProjectionInfo::NormalizeLat(double lat)
{
	return (lat > 90) ? (lat - 180) : lat;
}

void IProjectionInfo::SetFrame(Coordinate minCoord,
	uint32_t w, uint32_t h, bool keepAR)
{
	this->min = { std::numeric_limits<double>::max(), std::numeric_limits<double>::max() };
	this->max = { std::numeric_limits<double>::min(), std::numeric_limits<double>::min() };

	ProjectedValue minPixel = this->ProjectInternal(minCoord);
	//ProjectedValue maxPixel = this->ProjectInternal(maxCoord);


	this->min.x = std::min(this->min.x, minPixel.x);
	this->min.y = std::min(this->min.y, minPixel.y);


	//-----------------------------------------------------------

	minPixel.x = minPixel.x - this->min.x;
	minPixel.y = minPixel.y - this->min.y;

	this->w = w;
	this->h = h;
	
	auto ii = this->ProjectInverseInternal(minPixel.x + w, minPixel.y + h);
	printf("x");

}

void IProjectionInfo::SetFrame(Coordinate minCoord, Coordinate maxCoord, 
	uint32_t w, uint32_t h, bool keepAR)
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

}

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


IProjectionInfo::Coordinate IProjectionInfo::ProjectInverse(Pixel p) const
{
	
	double xx = (static_cast<double>(p.x) - this->wPadding + this->wAR * this->min.x);
	xx /= this->wAR;

	double yy = -1 * (static_cast<double>(p.y) - this->h + this->hPadding - this->hAR * this->min.y);
	yy /= this->hAR;
	

	ProjectedValueInverse pi = this->ProjectInverseInternal(xx, yy);

	Coordinate c;
	c.lat = GeoCoordinate::deg(pi.latDeg);
	c.lon = GeoCoordinate::deg(pi.lonDeg);
	return c;
}

//http://www.movable-type.co.uk/scripts/latlong.html
//Calculate end point based on shortest path (on real earth surface)
IProjectionInfo::Coordinate IProjectionInfo::CalcEndPointShortest(IProjectionInfo::Coordinate start,
	Angle bearing, double dist)
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

//"Rhumb lines"
//Calculate end point based on direct path (straight line between two points in projected earth)
IProjectionInfo::Coordinate IProjectionInfo::CalcEndPointDirect(
	IProjectionInfo::Coordinate start,
	Angle bearing, double dist)
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


void IProjectionInfo::LineBresenham(Pixel start, Pixel end,
	std::function<void(int x, int y)> callback)
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