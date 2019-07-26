#ifndef GEOCORDINATE_H
#define GEOCORDINATE_H

typedef float MyRealType;

template <typename T>
struct IAngle
{
	IAngle() : valRad(0), valDeg(0) {};
	static T deg(MyRealType val) { return T(val * 0.0174532925, val); };
	static T rad(MyRealType val) { return T(val, val * 57.2957795); };
	
	inline MyRealType deg() const { return valDeg; };
	inline MyRealType rad() const { return valRad; };

	inline T operator -() { return T(-valRad, -valDeg); };

protected:
	IAngle(MyRealType valRad, MyRealType valDeg) : valRad(valRad), valDeg(valDeg) {};
	MyRealType valRad;
	MyRealType valDeg;

};


struct Angle : public IAngle<Angle>
{
	Angle() : IAngle() {};

	friend struct IAngle<Angle>;

protected:
	Angle(MyRealType valRad, MyRealType valDeg) : IAngle(valRad, valDeg) {};
};

struct Latitude : public IAngle<Latitude>
{
	Latitude() : IAngle() {};
	Latitude(const Angle & a) : IAngle(a.rad(), a.deg()) {};

	friend struct IAngle<Latitude>;
protected:
	Latitude(MyRealType valRad, MyRealType valDeg) : IAngle(valRad, valDeg) {};
};

struct Longitude : public IAngle<Longitude>
{
	Longitude() : IAngle() {};
	Longitude(const Angle & a) : IAngle(a.rad(), a.deg()) {};

	friend struct IAngle<Longitude>;
protected:
	Longitude(MyRealType valRad, MyRealType valDeg) : IAngle(valRad, valDeg) {};
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
