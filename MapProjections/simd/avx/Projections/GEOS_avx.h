#ifndef GEOS_SIMD_H
#define GEOS_SIMD_H

#ifdef ENABLE_SIMD

#include <immintrin.h>     //AVX2

#include "../avx_math_float.h"

#include "../../../GeoCoordinate.h"
#include "../../../MapProjectionStructures.h"

#include "../ProjectionInfo_avx.h"

#include "../../../Projections/GEOS.h"

namespace Projections::Avx
{

	class GEOS : public Projections::GEOS, public ProjectionInfoAvx<GEOS>
	{
	public:
		using Projections::GEOS::ProjectInverse;
		using ProjectionInfoAvx<GEOS>::ProjectInverse;

		using Projections::GEOS::Project;
		using ProjectionInfoAvx<GEOS>::Project;


		GEOS(const SatelliteSettings & sets) :
			Projections::GEOS(sets)			
		{}

		friend class ProjectionInfoAvx<GEOS>;

	protected:
		ProjectedValueAvx ProjectInternal(const __m256 & lonRad, const __m256 & latRad) const
		{			
			__m256 lonDif = _mm256_sub_ps(lonRad, _mm256_set1_ps(static_cast<float>(sat.lon.rad())));

			__m256 tanLat = _my_mm256_tan_ps(latRad);
			__m256 cLat = _my_mm256_atan_ps(_mm256_mul_ps(tanLat, _mm256_set1_ps(0.993305616f)));
			
			__m256 cosCLat;
			__m256 sinCLat;
			_my_mm256_sincos_ps(cLat, &sinCLat, &cosCLat);
			
			__m256 tmp1 = _mm256_mul_ps(cosCLat, cosCLat);
			tmp1 = _mm256_mul_ps(tmp1, _mm256_set1_ps(0.00669438444f));
			tmp1 = _mm256_sub_ps(_mm256_set1_ps(1.0f), tmp1);
			tmp1 = _mm256_sqrt_ps(tmp1);

			__m256 r = _mm256_div_ps(_mm256_set1_ps(static_cast<float>(RADIUS_POLAR)), tmp1);
			
			tmp1 = _my_mm256_cos_ps(lonDif);
			tmp1 = _mm256_mul_ps(cosCLat, tmp1);
			tmp1 = _mm256_mul_ps(r, tmp1);			
			__m256 r1 = _mm256_sub_ps(_mm256_set1_ps(static_cast<float>(SAT_DIST)), tmp1);

			tmp1 = _mm256_sub_ps(lonRad, _mm256_set1_ps(static_cast<float>(sat.lon.rad())));
			tmp1 = _my_mm256_sin_ps(lonDif);
			tmp1 = _mm256_mul_ps(cosCLat, tmp1);
			__m256 r2 = _mm256_mul_ps(r, tmp1);
						
			__m256 r3 = _mm256_mul_ps(r, sinCLat);


			ProjectedValueAvx p;
			
			if (sat.sweepY)
			{
				__m256 r12 = _mm256_mul_ps(r1, r1);
				__m256 r22 = _mm256_mul_ps(r2, r2);
				__m256 r32 = _mm256_mul_ps(r3, r3);
				tmp1 = _mm256_add_ps(r12, r22);
				tmp1 = _mm256_add_ps(tmp1, r32);

				__m256 rn = _mm256_sqrt_ps(tmp1);
				
				p.x = _my_mm256_atan_ps(_mm256_div_ps(r2, r1));
				p.y = _my_mm256_asin_ps(_mm256_mul_ps(_mm256_set1_ps(-1.0f), _mm256_div_ps(r3, rn)));
			}
			else
			{
				__m256 r12 = _mm256_mul_ps(r1, r1);				
				__m256 r32 = _mm256_mul_ps(r3, r3);
				tmp1 = _mm256_add_ps(r12, r32);
				
				__m256 rn = _mm256_sqrt_ps(tmp1);				
				p.x = _my_mm256_atan_ps(_mm256_div_ps(r2, rn));
				p.y = _my_mm256_atan_ps(_mm256_mul_ps(_mm256_set1_ps(-1.0f), _mm256_div_ps(r3, r1)));
			}
						
			tmp1 = _mm256_mul_ps(ProjectionUtils::radToDeg(p.x), _mm256_set1_ps(static_cast<float>(TWO_POW_MINUS_16 * sat.cfac)));
			p.x = _mm256_add_ps(_mm256_set1_ps(static_cast<float>(sat.coff)), tmp1);

			
			tmp1 = _mm256_mul_ps(ProjectionUtils::radToDeg(p.y), _mm256_set1_ps(static_cast<float>(TWO_POW_MINUS_16 * sat.lfac)));
			p.y = _mm256_add_ps(_mm256_set1_ps(static_cast<float>(sat.loff)), tmp1);
			
			return p;
		};

		ProjectedValueInverseAvx ProjectInverseInternal(const __m256 & x, const __m256 & y) const
		{
			ProjectedValueInverseAvx c;
			
			//todo

			return c;
		};

	};
}

#endif //ENABLE_SIMD
#endif
