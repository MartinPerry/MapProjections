#ifndef MERCATOR_SIMD_H
#define MERCATOR_SIMD_H

#ifdef ENABLE_SIMD

#include <immintrin.h>     //AVX2

#include "../avx_math_float.h"

#include "../../../GeoCoordinate.h"
#include "../../../MapProjectionStructures.h"

#include "../ProjectionInfo_avx.h"

#include "../../../Projections/Mercator.h"

namespace Projections::Avx
{

	class Mercator : public Projections::Mercator, public ProjectionInfoAvx<Mercator>
	{
	public:
		using Projections::Mercator::ProjectInverse;
		using ProjectionInfoAvx<Mercator>::ProjectInverse;

		using Projections::Mercator::Project;
		using ProjectionInfoAvx<Mercator>::Project;


		friend class ProjectionInfoAvx<Mercator>;

	protected:
		ProjectedValueAvx ProjectInternal(const __m256 & lonRad, const __m256 & latRad) const
		{
			ProjectedValueAvx p;
			p.x = lonRad;

			p.y = _mm256_mul_ps(latRad, _mm256_set1_ps(0.5f));
			p.y = _mm256_add_ps(p.y, _mm256_set1_ps(static_cast<float>(ProjectionConstants::PI_4)));
			p.y = _my_mm256_tan_ps(p.y);
			p.y = _my_mm256_log_ps(p.y);

			//p.x = c.lon.rad();
			//p.y = std::log(std::tan(ProjectionConstants::PI_4 + 0.5 * c.lat.rad()));
			return p;
		};

		ProjectedValueInverseAvx ProjectInverseInternal(const __m256 & x, const __m256 & y) const
		{
			ProjectedValueInverseAvx c;
			c.lonRad = x;

			c.latRad = _my_mm256_pow_ps(_mm256_set1_ps(static_cast<float>(ProjectionConstants::E)), y);
			c.latRad = _my_mm256_atan_ps(c.latRad);
			c.latRad = _mm256_add_ps(c.latRad, c.latRad);
			c.latRad = _mm256_sub_ps(c.latRad, _mm256_set1_ps(static_cast<float>(ProjectionConstants::PI_2)));


			//c.lon = Longitude::rad(x);
			//c.lat = Latitude::rad(2.0 * std::atan(std::pow(ProjectionConstants::E, y)) - ProjectionConstants::PI_2);

			return c;
		};

	};
}

#endif //ENABLE_SIMD
#endif
