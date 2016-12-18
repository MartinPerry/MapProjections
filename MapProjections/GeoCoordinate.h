#ifndef _GEOCORDINATE_H_
#define _GEOCORDINATE_H_

struct GeoCoordinate
{
	GeoCoordinate() : valRad(0) {};
	static GeoCoordinate deg(double val) { return GeoCoordinate(val * 0.0174532925); };
	static GeoCoordinate rad(double val) { return GeoCoordinate(val); };

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


#endif
