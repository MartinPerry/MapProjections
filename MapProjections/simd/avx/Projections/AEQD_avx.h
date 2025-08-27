#ifndef AEQD_SIMD_H
#define AEQD_SIMD_H

#ifdef ENABLE_SIMD

#include <immintrin.h>     //AVX2

#include "../avx_math_float.h"

#include "../../../GeoCoordinate.h"
#include "../../../MapProjectionStructures.h"

#include "../ProjectionInfo_avx.h"

#include "../../../Projections/AEQD.h"

namespace Projections::Avx
{

	class AEQD : public Projections::AEQD, public ProjectionInfoAvx<AEQD>
	{
	public:
		using Projections::AEQD::AEQD;

		using Projections::AEQD::ProjectInverse;
		using ProjectionInfoAvx<AEQD>::ProjectInverse;

		using Projections::AEQD::Project;
		using ProjectionInfoAvx<AEQD>::Project;


		friend class ProjectionInfoAvx<AEQD>;

	protected:
		ProjectedValueAvx ProjectInternal(const __m256& lonRad, const __m256& latRad) const
		{			
			__m256 cLon = _mm256_set1_ps(static_cast<float>(centerLon.rad()));
			__m256 earthRad = _mm256_set1_ps(static_cast<float>(ProjectionConstants::EARTH_RADIUS));
			__m256 sinCLat = _mm256_set1_ps(static_cast<float>(sinCenterLat));
			__m256 cosCLat = _mm256_set1_ps(static_cast<float>(cosCenterLat));

			__m256 cosLat;
			__m256 sinLat;
			_my_mm256_sincos_ps(latRad, &sinLat, &cosLat);

			__m256 lonDif = _mm256_sub_ps(lonRad, cLon);

			__m256 cosDifLon;
			__m256 sinDifLon;
			_my_mm256_sincos_ps(lonDif, &sinDifLon, &cosDifLon);

			auto tmp0 = _mm256_mul_ps(sinCLat, sinLat);
			auto tmp1 = _mm256_mul_ps(_mm256_mul_ps(cosCLat, cosLat), cosDifLon);
			auto cosPhiR = _mm256_add_ps(tmp0, tmp1);

			auto phiR = _my_mm256_acos_ps(cosPhiR);
			auto phi = _mm256_mul_ps(phiR, earthRad);

			tmp0 = _mm256_mul_ps(cosLat, sinDifLon);
			auto tmp11 = _mm256_mul_ps(cosCLat, sinLat);
			auto tmp12 = _mm256_mul_ps(_mm256_mul_ps(sinCLat, cosLat), cosDifLon);
			tmp1 = _mm256_sub_ps(tmp11, tmp12);

			auto tanDelta = _mm256_div_ps(tmp0, tmp1);			
			auto delta = _mm256_atan2_ps(tmp0, tmp1);

			__m256 sinDelta;
			__m256 cosDelta;
			_my_mm256_sincos_ps(delta, &sinDelta, &cosDelta);

			ProjectedValueAvx p;
			p.x = _mm256_mul_ps(phi, sinDelta);
			p.y = _mm256_mul_ps(phi, cosDelta);

			return p;
		};

		ProjectedValueInverseAvx ProjectInverseInternal(const __m256& x, const __m256& y) const
		{
			__m256 cLon = _mm256_set1_ps(static_cast<float>(centerLon.rad()));
			__m256 sinCLat = _mm256_set1_ps(static_cast<float>(sinCenterLat));
			__m256 cosCLat = _mm256_set1_ps(static_cast<float>(cosCenterLat));
			__m256 earthRad = _mm256_set1_ps(static_cast<float>(ProjectionConstants::EARTH_RADIUS));

			auto p = _mm256_hypot_ps(x, y);

			auto c = _mm256_div_ps(p, earthRad);

			__m256 sinC;
			__m256 cosC;
			_my_mm256_sincos_ps(c, &sinC, &cosC);

			auto tmp0 = _mm256_mul_ps(cosC, sinCLat);
			auto tmp1 = _mm256_div_ps(_mm256_mul_ps(_mm256_mul_ps(y, sinC), cosCLat), p);

			auto sinPhi = _mm256_add_ps(tmp0, tmp1);

			// Clamp for numeric safety
			//if (sinPhi > 1.0) sinPhi = 1.0;
			//if (sinPhi < -1.0) sinPhi = -1.0;
			
			auto lat = _my_mm256_asin_ps(sinPhi);

			auto numerator = _mm256_mul_ps(x, sinC);
			tmp0 = _mm256_mul_ps(_mm256_mul_ps(p, cosCLat), cosC);
			tmp1 = _mm256_mul_ps(_mm256_mul_ps(y, sinC), sinCLat);
			auto denominator = _mm256_sub_ps(tmp0, tmp1);
			auto lon = _mm256_add_ps(cLon, _mm256_atan2_ps(numerator, denominator));
			
			ProjectedValueInverseAvx res;
			res.lonRad = lon;
			res.latRad = lat;
			
			return res;
		};

	};
}

#endif //ENABLE_SIMD
#endif
