#ifndef EQUIRECTANGULAR_NEON_H
#define EQUIRECTANGULAR_NEON_H

#ifdef HAVE_NEON

#include "../neon_math_float.h"

#include "../../../GeoCoordinate.h"
#include "../../../MapProjectionStructures.h"

#include "../ProjectionInfo_neon.h"

#include "../../../Projections/Equirectangular.h"

namespace Projections::Neon
{

	class Equirectangular : public Projections::Equirectangular, public ProjectionInfoNeon<Equirectangular>
	{
	public:
		using Projections::Equirectangular::ProjectInverse;
		using ProjectionInfoNeon<Equirectangular>::ProjectInverse;

		using Projections::Equirectangular::Project;
		using ProjectionInfoNeon<Equirectangular>::Project;


		friend class ProjectionInfoNeon<Equirectangular>;

	protected:
		ProjectedValueNeon ProjectInternal(const float32x4_t& lonRad, const float32x4_t& latRad) const
		{
			auto stPar = vdupq_n_f32(static_cast<float>(standardParallel.rad()));
			auto cosStPar = vdupq_n_f32(static_cast<float>(cosStandardParallel));
			auto lonMer = vdupq_n_f32(static_cast<float>(lonCentralMeridian.rad()));

			ProjectedValueNeon p;
			p.x = vsubq_f32(lonRad, lonMer);
			p.x = vmulq_f32(p.x, cosStPar);

			p.y = vsubq_f32(latRad, stPar);

			return p;
		};

		ProjectedValueInverseNeon ProjectInverseInternal(const float32x4_t& x, const float32x4_t& y) const
		{
			
			auto stPar = vdupq_n_f32(static_cast<float>(standardParallel.rad()));
			auto cosStPar = vdupq_n_f32(static_cast<float>(cosStandardParallel));
			auto lonMer = vdupq_n_f32(static_cast<float>(lonCentralMeridian.rad()));
			
			ProjectedValueInverseNeon c;
			c.lonRad = vaddq_f32(x, stPar);

#if defined(__aarch64__) || defined(__arm64__) || defined(vdivq_f32)
			c.latRad = vdivq_f32(y, cosStPar);
#else
			c.latRad = vmulq_f32(y, vrecpeq_f32(cosStPar)); //y * 1/cosStPar
#endif			
			c.latRad = vaddq_f32(c.latRad, lonMer);

			return c;
		};

	};
}

#endif //ENABLE_SIMD
#endif
