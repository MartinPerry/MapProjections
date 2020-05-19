#ifndef EQUIRECTANGULAR_SIMD_H
#define EQUIRECTANGULAR_SIMD_H

#ifdef ENABLE_SIMD

#include <immintrin.h>     //AVX2

#include "../avx_math_float.h"

#include "../../GeoCoordinate.h"
#include "../../MapProjectionStructures.h"

#include "../ProjectionInfo_simd.h"

#include "../../Projections/Equirectangular.h"

namespace Projections::Simd
{

	class Equirectangular : public Projections::Equirectangular, public ProjectionInfoSimd<Equirectangular>
	{
	public:
		using Projections::Equirectangular::ProjectInverse;
		using ProjectionInfoSimd<Equirectangular>::ProjectInverse;

		using Projections::Equirectangular::Project;
		using ProjectionInfoSimd<Equirectangular>::Project;


		friend class ProjectionInfoSimd<Equirectangular>;

	protected:
		ProjectedValueSimd ProjectInternal(const __m256 & lonRad, const __m256 & latRad) const
		{
			auto stPar = _mm256_set1_ps(static_cast<float>(standardParallel.rad()));
			auto cosStPar = _mm256_set1_ps(static_cast<float>(cosStandardParallel));
			auto lonMer = _mm256_set1_ps(static_cast<float>(lonCentralMeridian.rad()));

			ProjectedValueSimd p;
			p.x = _mm256_sub_ps(lonRad, lonMer);
			p.x = _mm256_mul_ps(p.x, cosStPar);

			p.y = _mm256_sub_ps(latRad, stPar);

			return p;
		};

		ProjectedValueInverseSimd ProjectInverseInternal(const __m256 & x, const __m256 & y) const
		{
			
			auto stPar = _mm256_set1_ps(static_cast<float>(standardParallel.rad()));
			auto cosStPar = _mm256_set1_ps(static_cast<float>(cosStandardParallel));
			auto lonMer = _mm256_set1_ps(static_cast<float>(lonCentralMeridian.rad()));


			ProjectedValueInverseSimd c;
			c.lonRad = _mm256_add_ps(x, stPar);

			c.latRad = _mm256_div_ps(y, cosStPar);
			c.latRad = _mm256_add_ps(c.latRad, lonMer);

			return c;
		};

	};
}

#endif //ENABLE_SIMD
#endif
