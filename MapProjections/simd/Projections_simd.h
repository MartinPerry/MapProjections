#ifndef PROJECTIONS_SIMD_H
#define PROJECTIONS_SIMD_H

#include <immintrin.h>     //AVX2

#include "./avx_math_float.h"

#include "../GeoCoordinate.h"
#include "../MapProjectionStructures.h"
#include "../Projections.h"

#include "./ProjectionInfo_simd.h"

namespace Projections::Simd
{
    /*
    template <typename Proj>
    class SimdProjection : public Projections::Mercator, public ProjectionInfoSimd<Mercator>
    {
        public:
        using Projections::Mercator::ProjectInverse;
        using ProjectionInfoSimd<Mercator>::ProjectInverse;
        
        using Projections::Mercator::Project;
        using ProjectionInfoSimd<Mercator>::Project;
    };
    */
    
    class Mercator : public Projections::Mercator, public ProjectionInfoSimd<Mercator>
    {
        public:
        using Projections::Mercator::ProjectInverse;
        using ProjectionInfoSimd<Mercator>::ProjectInverse;
        
        using Projections::Mercator::Project;
        using ProjectionInfoSimd<Mercator>::Project;
        
        
        friend class ProjectionInfoSimd<Mercator>;
        
        protected:
        ProjectedValueSimd ProjectInternal(const __m256 & lonRad, const __m256 & latRad) const
        {
            ProjectedValueSimd p;
            p.x = lonRad;
        
            p.y = _mm256_mul_ps(latRad, _mm256_set1_ps(0.5));
            p.y = _mm256_add_ps(p.y, _mm256_set1_ps(ProjectionConstants::PI_4));
            p.y = _my_mm256_tan_ps(p.y);
            p.y = _my_mm256_log_ps(p.y);
            
            //p.x = c.lon.rad();
            //p.y = std::log(std::tan(ProjectionConstants::PI_4 + 0.5 * c.lat.rad()));
            return p;
        };
        
        ProjectedValueInverseSimd ProjectInverseInternal(const __m256 & x, const __m256 & y) const
        {			
			ProjectedValueInverseSimd c;
            c.lonRad = x;
            
            c.latRad = _my_mm256_pow_ps(_mm256_set1_ps(ProjectionConstants::E), y);
            c.latRad = _my_mm256_atan_ps(c.latRad);
            c.latRad = _mm256_add_ps(c.latRad, c.latRad);
            c.latRad = _mm256_sub_ps(c.latRad, _mm256_set1_ps(ProjectionConstants::PI_2));
            
			
            //c.lon = Longitude::rad(x);
            //c.lat = Latitude::rad(2.0 * std::atan(std::pow(ProjectionConstants::E, y)) - ProjectionConstants::PI_2);

            return c;
        };
        
    };
    
    
    
    
    class Miller : public Projections::Miller, public ProjectionInfoSimd<Miller>
    {
        public:
        using Projections::Miller::ProjectInverse;
        using ProjectionInfoSimd<Miller>::ProjectInverse;
        
        using Projections::Miller::Project;
        using ProjectionInfoSimd<Miller>::Project;
        
        
        friend class ProjectionInfoSimd<Miller>;
        
        protected:
        ProjectedValueSimd ProjectInternal(const __m256 & lonRad, const __m256 & latRad) const
        {
            ProjectedValueSimd p;
            p.x = lonRad;
            
            p.y = _mm256_mul_ps(latRad, _mm256_set1_ps(0.4));
            p.y = _mm256_add_ps(p.y, _mm256_set1_ps(ProjectionConstants::PI_4));
            p.y = _my_mm256_tan_ps(p.y);
            p.y = _my_mm256_log_ps(p.y);
            p.y = _mm256_mul_ps(p.y, _mm256_set1_ps(1.25));
            
            //p.x = c.lon.rad();
            //p.y = 1.25 * std::log(std::tan(ProjectionConstants::PI_4 + 0.4 * c.lat.rad()));
            
            return p;
        };
        
        ProjectedValueInverseSimd ProjectInverseInternal(const __m256 & x, const __m256 & y) const
        {
            ProjectedValueInverseSimd c;
            c.lonRad = x;
            
            c.latRad = _my_mm256_pow_ps(_mm256_set1_ps(ProjectionConstants::E), _mm256_mul_ps(y, _mm256_set1_ps(0.8)));
            c.latRad = _my_mm256_atan_ps(c.latRad);
            c.latRad = _mm256_mul_ps(c.latRad, _mm256_set1_ps(2.5));
            c.latRad = _mm256_sub_ps(c.latRad, _mm256_set1_ps(0.625 * ProjectionConstants::PI));
            
            //c.lon = Longitude::rad(x);
            //c.lat = Latitude::rad(2.5 * std::atan(std::pow(ProjectionConstants::E, 0.8 * y)) - 0.625 * ProjectionConstants::PI);
            return c;
        };
        
    };
}

#endif /* PROJECTIONS_SIMD_H */
