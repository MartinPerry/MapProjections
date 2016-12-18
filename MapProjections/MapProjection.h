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
		EQUIRECTANGULAR = 2
	} PROJECTION;

	struct Pixel { int x; int y; };
	struct Coordinate { GeoCoordinate lat; GeoCoordinate lon; };

	const PROJECTION curProjection;

	IProjectionInfo::Pixel Project(Coordinate c) const;
	IProjectionInfo::Coordinate ProjectInverse(Pixel p) const;

	void SetFrame(IProjectionInfo * proj,
		int w, int h, bool keepAR = true);
	void SetFrame(Coordinate minCoord, Coordinate maxCoord,
		int w, int h, bool keepAR = true);
	
	IProjectionInfo::Coordinate GetTopLeftCorner() const;

	Angle GetStepLat() const { return this->stepLat;}
	Angle GetStepLon() const { return this->stepLon; }

	int GetFrameWidth() const { return this->w; }
	int GetFrameHeight() const { return this->h; }

	IProjectionInfo::Coordinate CalcEndPointShortest(IProjectionInfo::Coordinate start, Angle bearing, double dist) const;
	IProjectionInfo::Coordinate CalcEndPointDirect(IProjectionInfo::Coordinate start, Angle bearing, double dist) const;

	void LineBresenham(Pixel start, Pixel end, std::function<void(int x, int y)> callback) const;

	
	std::vector<IProjectionInfo::Pixel> CreateReprojection(IProjectionInfo * imProj) const;

	void ComputeAABB(IProjectionInfo::Coordinate & min, IProjectionInfo::Coordinate & max) const;

	static void ComputeAABB(const std::vector<IProjectionInfo::Coordinate> & c,
		IProjectionInfo::Coordinate & min, IProjectionInfo::Coordinate & max);

	static double NormalizeLon(double lonDeg);
	static double NormalizeLat(double latDeg);

	
protected:

	struct ProjectedValue
	{
		double x;
		double y;		
	};

	struct ProjectedValueInverse
	{
		double latDeg;
		double lonDeg;
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


	ProjectedValue min;
	ProjectedValue max;

	int w;
	int h;

	double wPadding;
	double hPadding;
	double wAR;
	double hAR;

	Angle stepLat;
	Angle stepLon;

	IProjectionInfo(PROJECTION curProjection);

	virtual ProjectedValue ProjectInternal(Coordinate c) const = 0;
	virtual ProjectedValueInverse ProjectInverseInternal(double x, double y) const = 0;
	
	
};










#endif
