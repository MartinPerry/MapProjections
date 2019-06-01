#ifndef GEOCORDINATE_H
#define GEOCORDINATE_H

template <typename T>
struct IAngle
{
	IAngle() : valRad(0), valDeg(0) {};
	static T deg(double val) { return T(val, val * 0.0174532925); };
	static T rad(double val) { return T(val * 57.2957795, val); };
	
	inline double deg() const { return valDeg; };
	inline double rad() const { return valRad; };

	inline T operator -() { return T(-valRad, -valDeg); };

protected:
	IAngle(double valRad, double valDeg) : valRad(valRad), valDeg(valDeg) {};
	double valRad;
	double valDeg;

};


struct Angle : public IAngle<Angle>
{
	Angle() : IAngle() {};

	friend struct IAngle<Angle>;

protected:
	Angle(double valRad, double valDeg) : IAngle(valRad, valDeg) {};
};

struct Latitude : public IAngle<Latitude>
{
	Latitude() : IAngle() {};
	Latitude(const Angle & a) : IAngle(a.rad(), a.deg()) {};

	friend struct IAngle<Latitude>;
protected:
	Latitude(double valRad, double valDeg) : IAngle(valRad, valDeg) {};
};

struct Longitude : public IAngle<Longitude>
{
	Longitude() : IAngle() {};
	Longitude(const Angle & a) : IAngle(a.rad(), a.deg()) {};

	friend struct IAngle<Longitude>;
protected:
	Longitude(double valRad, double valDeg) : IAngle(valRad, valDeg) {};
};


//==============================================================
//String literall operator for Angle only
//Latitude and longitude can be created from Angle

inline Angle operator "" _deg(long double value)
{
	return Angle::deg(value);
}

inline Angle operator "" _rad(long double value)
{
	return  Angle::rad(value);
}




#endif
