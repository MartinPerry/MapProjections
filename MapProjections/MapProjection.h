#ifndef _MAP_PROJECTION_H_
#define _MAP_PROJECTION_H_



#include <cstdint>
#include <cmath>
#include <vector>
#include <complex>
#include <functional>

#include <string>

struct GeoCoordinate
{
	GeoCoordinate() : valRad(0) {};
	static GeoCoordinate deg(double val) { return GeoCoordinate(val * 0.0174532925); };
	static GeoCoordinate rad(double val) { return GeoCoordinate(val);};
	
	inline double deg() const
	{
		return valRad * 57.2957795;
	}

	inline double rad() const
	{
		return valRad;
	}
			

	inline GeoCoordinate operator -()
	{
		return GeoCoordinate(-valRad);
	};

	
private:
	GeoCoordinate(double val) : valRad(val) {};
	double valRad;
	
};




inline GeoCoordinate operator "" _deg(long double value)
{
	return GeoCoordinate::deg(value);
}

inline GeoCoordinate operator "" _rad(long double value)
{
	return  GeoCoordinate::rad(value);
}


typedef GeoCoordinate Angle;



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

	void SetFrame(Coordinate minCoord, Coordinate maxCoord,
		int w, int h, bool keepAR = true);


	int GetFrameWidth() const { return this->w; }
	int GetFrameHeight() const { return this->h; }

	IProjectionInfo::Coordinate CalcEndPointShortest(IProjectionInfo::Coordinate start, Angle bearing, double dist);
	IProjectionInfo::Coordinate CalcEndPointDirect(IProjectionInfo::Coordinate start, Angle bearing, double dist);

	void LineBresenham(Pixel start, Pixel end, std::function<void(int x, int y)> callback);

	void ComputeAABB(const std::vector<IProjectionInfo::Coordinate> & c,
		IProjectionInfo::Coordinate & min, IProjectionInfo::Coordinate & max);
	std::vector<IProjectionInfo::Pixel> CreateReprojection(IProjectionInfo * imProj);


	static double NormalizeLon(double lon);
	static double NormalizeLat(double lat);

	friend class ProjectionDebugger;

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

	IProjectionInfo(PROJECTION curProjection)
		: curProjection(curProjection),
		min({std::numeric_limits<double>::max(), std::numeric_limits<double>::max()}),
		max({std::numeric_limits<double>::min(), std::numeric_limits<double>::min()}),
		w(0), h(0), wPadding(0), hPadding(0), wAR(1), hAR(1)
	{}
	
	static const double PI;
	static const double PI_4;
	static const double PI_2;
	static const double E;
	static const double EARTH_RADIUS;

	inline double cot(double x) const { return 1.0 / std::tan(x); };
	inline double sec(double x) const { return 1.0 / std::cos(x); };
	inline double sinc(double x) const { return std::sin(x) / x; };
	inline double sgn(double x) const { return (x < 0) ? -1 : (x > 0); };
	
	inline double degToRad(double x) const { return x * 0.0174532925;}
	inline double radToDeg(double x) const { return x * 57.2957795; }


	ProjectedValue min;
	ProjectedValue max;

	int w;
	int h;

	double wPadding;
	double hPadding;
	double wAR;
	double hAR;
	//double globalAR;

	virtual ProjectedValue ProjectInternal(Coordinate c) const = 0;
	virtual ProjectedValueInverse ProjectInverseInternal(double x, double y) const = 0;
	
	
};










#endif
