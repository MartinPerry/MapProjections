#ifndef GEOCORDINATE_H
#define GEOCORDINATE_H

template <typename T>
struct IAngle
{
	IAngle() : valRad(0) {};
	static T deg(double val) { return T(val * 0.0174532925); };
	static T rad(double val) { return T(val); };

	inline double deg() const
	{
		return valRad * 57.2957795;
	}

	inline double rad() const
	{
		return valRad;
	}


	inline T operator -()
	{
		return T(-valRad);
	};


protected:
	IAngle(double val) : valRad(val) {};	
	double valRad;

};

struct Angle : public IAngle<Angle>
{
	Angle() : IAngle() {};

	friend struct IAngle<Angle>;

protected:
	Angle(double val) : IAngle(val) {};
};

struct Latitude : public IAngle<Latitude>
{
	Latitude() : IAngle() {};
	Latitude(const Angle & a) : IAngle(a.rad()) {};

	friend struct IAngle<Latitude>;
protected:
	Latitude(double val) : IAngle(val) {};
};

struct Longitude : public IAngle<Longitude>
{
	Longitude() : IAngle() {};
	Longitude(const Angle & a) : IAngle(a.rad()) {};

	friend struct IAngle<Longitude>;
protected:
	Longitude(double val) : IAngle(val) {};
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
