#ifndef _MAP_PROJECTION_H_
#define _MAP_PROJECTION_H_



#include <cstdint>
#include <cmath>
#include <vector>
#include <complex>
#include <functional>


#include <string>

#include "GeoCoordinate.h"


class IProjectionInfo
{
public:
	typedef enum PROJECTION {
		MERCATOR = 0, 
		LAMBERT_CONIC = 1, 
		EQUIRECTANGULAR = 2,
		WEB_MERCATOR = 3
	} PROJECTION;

	template <typename PixelType = int, 
		typename = typename std::enable_if<std::is_arithmetic<PixelType>::value, PixelType>::type>
	struct Pixel { PixelType x; PixelType y; };

	struct Coordinate { GeoCoordinate lat; GeoCoordinate lon; };

	struct Reprojection {
		int inW;
		int inH;
		int outW;
		int outH;
		std::vector<IProjectionInfo::Pixel<int>> pixels;

		static Reprojection CreateFromFile(const std::string & fileName);
		void SaveToFile(const std::string & fileName);

	};

	const PROJECTION curProjection;

	virtual ~IProjectionInfo() = default;

	template <typename PixelType = int> 
	typename std::enable_if<std::is_integral<PixelType>::value, 
		IProjectionInfo::Pixel<PixelType>>::type
	Project(Coordinate c) const;


	template <typename PixelType = float>
	typename std::enable_if<std::is_floating_point<PixelType>::value, 
		IProjectionInfo::Pixel<PixelType>>::type
	Project(Coordinate c) const;
	



	template <typename PixelType = int>
	IProjectionInfo::Coordinate ProjectInverse(Pixel<PixelType> p) const;

	void SetFrame(IProjectionInfo * proj, bool keepAR = true);
	void SetFrame(IProjectionInfo * proj,
		double w, double h, bool keepAR = true);
	void SetFrame(Coordinate minCoord, Coordinate maxCoord,
		double w, double h, bool keepAR = true);
	
	IProjectionInfo::Coordinate GetTopLeftCorner() const;

	Angle GetStepLat() const { return this->stepLat;}
	Angle GetStepLon() const { return this->stepLon; }

	template <typename T = int>
	T GetFrameWidth() const { return static_cast<T>(this->w); }
	template <typename T = int>
	T GetFrameHeight() const { return static_cast<T>(this->h); }

	IProjectionInfo::Coordinate CalcEndPointShortest(IProjectionInfo::Coordinate start, Angle bearing, double dist) const;
	IProjectionInfo::Coordinate CalcEndPointDirect(IProjectionInfo::Coordinate start, Angle bearing, double dist) const;

	void LineBresenham(Pixel<int> start, Pixel<int> end, std::function<void(int x, int y)> callback) const;

	
	
	void ComputeAABB(IProjectionInfo::Coordinate & min, IProjectionInfo::Coordinate & max) const;
	
	static Reprojection CreateReprojection(IProjectionInfo * from, IProjectionInfo * to);

	static void ComputeAABB(const std::vector<IProjectionInfo::Coordinate> & c,
		IProjectionInfo::Coordinate & min, IProjectionInfo::Coordinate & max);

	static double NormalizeLon(double lonDeg);
	static double NormalizeLat(double latDeg);

    static double Distance(const Coordinate & from, const Coordinate & to);
	
protected:

	struct ProjectedValue
	{
		double x;
		double y;		
	};

	struct ProjectedValueInverse
	{
		GeoCoordinate lat;
		GeoCoordinate lon;
	};

		
	static const double PI;
	static const double PI_4;
	static const double PI_2;
	static const double E;
	static const double EARTH_RADIUS;

	inline static double cot(double x) { return 1.0 / std::tan(x); };
	inline static double sec(double x) { return 1.0 / std::cos(x); };
	inline static double sinc(double x) { return std::sin(x) / x; };
	inline static double sgn(double x) { return (x < 0) ? -1 : (x > 0); };
	
	inline static double degToRad(double x) { return x * 0.0174532925;}
	inline static double radToDeg(double x) { return x * 57.2957795; }


	ProjectedValue minPixelOffset; //offset to move min corner to [0,0] and other corners accordingly
	
	double w; //current frame width
	double h; //current frame height

	double wPadding;
	double hPadding;
	double wAR; //width AR
	double hAR; //height AR

	Angle stepLat;
	Angle stepLon;

	IProjectionInfo(PROJECTION curProjection);

	virtual ProjectedValue ProjectInternal(Coordinate c) const = 0;
	virtual ProjectedValueInverse ProjectInverseInternal(double x, double y) const = 0;
	
	
};



/// <summary>
/// Project Coordinate point to pixel
/// </summary>
/// <param name="c"></param>
/// <returns></returns>
template <typename PixelType>
typename std::enable_if<std::is_integral<PixelType>::value, 
	IProjectionInfo::Pixel<PixelType>>::type
IProjectionInfo::Project(Coordinate c) const
{

	ProjectedValue raw = this->ProjectInternal(c);


	raw.x = raw.x - this->minPixelOffset.x;
	raw.y = raw.y - this->minPixelOffset.y;

	IProjectionInfo::Pixel<PixelType> p;
	p.x = static_cast<PixelType>(std::round(this->wPadding + (raw.x * this->wAR)));
	p.y = static_cast<PixelType>(std::round(this->h - this->hPadding - (raw.y * this->hAR)));

	return p;
}

template <typename PixelType>
typename std::enable_if<std::is_floating_point<PixelType>::value, 
	IProjectionInfo::Pixel<PixelType>>::type
IProjectionInfo::Project(Coordinate c) const
{

	//project value and get "pseudo" pixel coordinate
	ProjectedValue rawPixel = this->ProjectInternal(c);

	//move our pseoude pixel to "origin"
	rawPixel.x = rawPixel.x - this->minPixelOffset.x;
	rawPixel.y = rawPixel.y - this->minPixelOffset.y;

	//calculate pixel in final frame
	IProjectionInfo::Pixel<PixelType> p;
	p.x = static_cast<PixelType>(this->wPadding + (rawPixel.x * this->wAR));
	p.y = static_cast<PixelType>(this->h - this->hPadding - (rawPixel.y * this->hAR));

	return p;
}

/// <summary>
/// Project pixel to coordinate
/// </summary>
/// <param name="p"></param>
/// <returns></returns>
template <typename PixelType>
IProjectionInfo::Coordinate IProjectionInfo::ProjectInverse(Pixel<PixelType> p) const
{

	double xx = (static_cast<double>(p.x) - this->wPadding + this->wAR * this->minPixelOffset.x);
	xx /= this->wAR;

	double yy = (static_cast<double>(p.y) - this->h + this->hPadding - this->hAR * this->minPixelOffset.y);
	yy /= -this->hAR;


	ProjectedValueInverse pi = this->ProjectInverseInternal(xx, yy);

	Coordinate c;
	c.lat = GeoCoordinate::deg(NormalizeLat(pi.lat.deg()));
	c.lon = GeoCoordinate::deg(NormalizeLon(pi.lon.deg()));
	return c;
}







#endif
