#ifndef MERCATOR_NEON_H
#define MERCATOR_NEON_H

#ifdef HAVE_NEON


#include "../neon_math_float.h"

#include "../../../GeoCoordinate.h"
#include "../../../MapProjectionStructures.h"

#include "../ProjectionInfo_neon.h"

#include "../../../Projections/Mercator.h"

namespace Projections::Neon
{

	class Mercator : public Projections::Mercator, public ProjectionInfoNeon<Mercator>
	{
	public:
		using Projections::Mercator::ProjectInverse;
		using ProjectionInfoNeon<Mercator>::ProjectInverse;

		using Projections::Mercator::Project;
		using ProjectionInfoNeon<Mercator>::Project;


		friend class ProjectionInfoNeon<Mercator>;

	protected:
		ProjectedValueNeon ProjectInternal(const float32x4_t& lonRad, const float32x4_t& latRad) const
		{
			ProjectedValueNeon p;
			p.x = lonRad;

			p.y = vmulq_f32(latRad, vdupq_n_f32(0.5f));
			p.y = vaddq_f32(p.y, vdupq_n_f32(static_cast<float>(ProjectionConstants::PI_4)));
			p.y = my_tan_f32(p.y);
			p.y = my_log_f32(p.y);
			
			//p.x = c.lon.rad();
			//p.y = std::log(std::tan(ProjectionConstants::PI_4 + 0.5 * c.lat.rad()));
			return p;
		};

		ProjectedValueInverseNeon ProjectInverseInternal(const float32x4_t& x, const float32x4_t& y) const
		{
			ProjectedValueInverseNeon c;
			c.lonRad = x;

			c.latRad = my_pow_f32(vdupq_n_f32(static_cast<float>(ProjectionConstants::E)), y);
			c.latRad = my_atan_f32(c.latRad);
			c.latRad = vaddq_f32(c.latRad, c.latRad);
			c.latRad = vsubq_f32(c.latRad, vdupq_n_f32(static_cast<float>(ProjectionConstants::PI_2)));


			//c.lon = Longitude::rad(x);
			//c.lat = Latitude::rad(2.0 * std::atan(std::pow(ProjectionConstants::E, y)) - ProjectionConstants::PI_2);

			return c;
		};

	};
}

#endif //ENABLE_SIMD
#endif
