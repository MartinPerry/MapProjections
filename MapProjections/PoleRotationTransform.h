#ifndef POLE_ROTATION_TRANSFORM
#define POLE_ROTATION_TRANSFORM

#include <cmath>

#include "./GeoCoordinate.h"
#include "./IProjectionInfo.h"

namespace Projections
{
	

	/// <summary>
	/// Based on:
	/// https://confluence.govcloud.dk/pages/viewpage.action?pageId=76153193
	/// </summary>
	class PoleRotationTransform : public ITransform
	{
	public:
		
		PoleRotationTransform(const Coordinate& southPole) :
			southpoleLon(southPole.lon),
			southpoleLat(southPole.lat),
			southpoleLatSin(sin(southpoleLat.rad())),
			southpoleLatCos(cos(southpoleLat.rad()))
		{}

		PoleRotationTransform(const PoleRotationTransform& ot) :
			PoleRotationTransform(Coordinate(ot.southpoleLon, ot.southpoleLat))
		{}
		
		/// <summary>
		/// Original -> Rotated
		/// </summary>
		/// <param name="c"></param>
		/// <returns></returns>
		Coordinate Transform(const Coordinate& c) const
		{			
			MyRealType  lon_rad, sin_lon_rad, cos_lon_rad, sin_y_reg, cos_y_reg,
				sin_y_rot, cos_y_rot, cos_x_rot, sin_x_rot;

						
			lon_rad = (c.lon.rad() - southpoleLon.rad());
			sin_lon_rad = sin(lon_rad);
			cos_lon_rad = cos(lon_rad);
			sin_y_reg = sin(c.lat.rad());
			cos_y_reg = cos(c.lat.rad());

			sin_y_rot = southpoleLatCos * sin_y_reg - southpoleLatSin * cos_y_reg * cos_lon_rad;
			if (sin_y_rot < -1.0) sin_y_rot = -1.0;
			if (sin_y_rot > 1.0) sin_y_rot = 1.0;

			MyRealType rot_lat = (MyRealType)asin(sin_y_rot);

			cos_y_rot = cos(rot_lat);
			cos_x_rot = (southpoleLatCos * cos_y_reg * cos_lon_rad + southpoleLatSin * sin_y_reg) / cos_y_rot;
			if (cos_x_rot < -1.0) cos_x_rot = -1.0;
			if (cos_x_rot > 1.0) cos_x_rot = 1.0;
			sin_x_rot = cos_y_reg * sin_lon_rad / cos_y_rot;

			MyRealType  rot_lon = (MyRealType)acos(cos_x_rot);
			if (sin_x_rot < 0.0) rot_lon = -(rot_lon);


			return Coordinate(Longitude::rad(rot_lon), Latitude::rad(rot_lat));				
		};

		/// <summary>
		/// Rotated -> Original
		/// </summary>
		/// <param name="c"></param>
		/// <returns></returns>
		Coordinate TransformInverse(const Coordinate& c) const
		{			
			MyRealType  lon_rad, sin_lon_rad, cos_lon_rad, sin_y_reg, cos_y_reg,
				sin_y_rot, cos_y_rot, cos_x_rot, sin_x_rot;
		
			sin_x_rot = sin(c.lon.rad());
			cos_x_rot = cos(c.lon.rad());

			sin_y_rot = sin(c.lat.rad());
			cos_y_rot = cos(c.lat.rad());

			sin_y_reg = southpoleLatCos * sin_y_rot + southpoleLatSin * cos_y_rot * cos_x_rot;
			if (sin_y_reg < -1.0) sin_y_reg = -1.0;
			if (sin_y_reg > 1.0) sin_y_reg = 1.0;

			MyRealType reg_lat = (MyRealType)asin(sin_y_reg);

			cos_y_reg = cos(reg_lat);
			cos_lon_rad = (southpoleLatCos * cos_y_rot * cos_x_rot - southpoleLatSin * sin_y_rot) / cos_y_reg;
			if (cos_lon_rad < -1.0) cos_lon_rad = -1.0;
			if (cos_lon_rad > 1.0) cos_lon_rad = 1.0;
			sin_lon_rad = cos_y_rot * sin_x_rot / cos_y_reg;
			lon_rad = acos(cos_lon_rad);
			if (sin_lon_rad < 0.0) lon_rad = -lon_rad;

			MyRealType reg_lon = lon_rad + southpoleLon.rad();

			return Coordinate(Longitude::rad(reg_lon), Latitude::rad(reg_lat));
		};

	protected:
		const Longitude southpoleLon;
		const Latitude southpoleLat;

		const MyRealType southpoleLatSin;
		const MyRealType southpoleLatCos;

		inline static const Latitude LAT_MIN = -90.0_deg;
		inline static const Latitude LAT_MAX = 90.0_deg;

	};
}

#endif
