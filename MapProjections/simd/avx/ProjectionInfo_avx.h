#ifndef PROJECTION_INFO_SIMD_H
#define PROJECTION_INFO_SIMD_H

#ifdef ENABLE_SIMD

#include <array>
#include <immintrin.h>     //AVX2

#include "../../GeoCoordinate.h"
#include "../../MapProjectionStructures.h"
#include "../../ProjectionInfo.h"

#include "./MapProjectionUtils_avx.h"
#include "./MapProjectionStructures_avx.h"


#define RET_VAL_SIMD(PixelType, enable_cond) \
typename std::enable_if<enable_cond<PixelType>::value, std::array<Projections::Pixel<PixelType>, 8>>::type

namespace Projections::Avx
{
    template <typename Proj>
    class ProjectionInfoAvx
    {
        public:
        virtual ~ProjectionInfoAvx() = default;
        
        template <typename PixelType = int>
        std::array<Projections::Pixel<PixelType>, 8> Project(const std::array<Projections::Coordinate, 8> & c) const;
        
        
        template <typename PixelType = int, bool Normalize = true>
        std::array<Projections::Coordinate, 8> ProjectInverse(const std::array<Projections::Pixel<PixelType>, 8> & p) const;
        
        PixelAvx Project(const CoordinateAvx & p) const;
        CoordinateAvx ProjectInverse(const PixelAvx & p) const;
        
        protected:
        struct ProjectedValueInverseAvx
        {
            __m256 latRad;
            __m256 lonRad;
        };
        
        struct ProjectedValueAvx
        {
            __m256 x;
            __m256 y;
        };
    };
    
    
    /// <summary>
    /// Project Coordinate point to pixel
    /// </summary>
    /// <param name="c"></param>
    /// <returns></returns>
    template <typename Proj>
    template <typename PixelType>
    std::array<Projections::Pixel<PixelType>, 8> ProjectionInfoAvx<Proj>::Project(const std::array<Projections::Coordinate, 8> & c) const
    {
        CoordinateAvx cAvx = CoordinateAvx::FromArray(c);
        auto raw = this->Project(cAvx);
        return PixelAvx::ToArray<PixelType>(raw);
    };
    
    template <typename Proj>
    PixelAvx ProjectionInfoAvx<Proj>::Project(const CoordinateAvx & p) const
    {
        const Proj * tmp = static_cast<const Proj*>(this);
        auto frame = tmp->GetFrame();
        
        //project value and get "pseudo" pixel coordinate
        auto raw = tmp->ProjectInternal(p.lonRad, p.latRad);
        
		PixelAvx res;

		res.x = _mm256_mul_ps(raw.x, _mm256_set1_ps(static_cast<float>(frame.wAR)));
		res.x = _mm256_sub_ps(res.x, _mm256_set1_ps(static_cast<float>(frame.projPrecomX)));

		res.y = _mm256_mul_ps(raw.y, _mm256_set1_ps(static_cast<float>(-frame.hAR)));
		res.y = _mm256_sub_ps(res.y, _mm256_set1_ps(static_cast<float>(frame.projPrecomY)));
		       
        return res;
    }
    
    /// <summary>
    /// Project 8 pixels to coordinate at once
    /// Return result as coordinate
    /// </summary>
    /// <param name="p"></param>
    /// <returns></returns>
    template <typename Proj>
    template <typename PixelType, bool Normalize>
    std::array<Projections::Coordinate, 8> ProjectionInfoAvx<Proj>::ProjectInverse(const std::array<Projections::Pixel<PixelType>, 8> & p) const
    {
        PixelAvx pAvx = PixelAvx::FromArray(p);
        auto cAvx = this->ProjectInverse(pAvx);
        
        return CoordinateAvx::ToArray<Normalize>(cAvx);
    };
    
    /// <summary>
    /// Project pixels stored in SIMD register to coordinate
    /// stored in SIMD register
    /// (Does not support normalization)
    /// </summary>
    /// <param name="p"></param>
    /// <returns></returns>
    template <typename Proj>
    CoordinateAvx ProjectionInfoAvx<Proj>::ProjectInverse(const PixelAvx & p) const
    {
        const Proj * tmp = static_cast<const Proj*>(this);
        const auto& frame = tmp->GetFrame();
        
        __m256 x = p.x;
        __m256 y = p.y;
        
        x = _mm256_add_ps(x, _mm256_set1_ps(static_cast<float>(frame.projPrecomX)));
        x = _mm256_div_ps(x, _mm256_set1_ps(static_cast<float>(frame.wAR)));
        
        y = _mm256_add_ps(y, _mm256_set1_ps(static_cast<float>(frame.projPrecomY)));
        y = _mm256_div_ps(y, _mm256_set1_ps(static_cast<float>(-frame.hAR)));
        
        auto pi = tmp->ProjectInverseInternal(x, y);
        
        return CoordinateAvx(pi.lonRad, pi.latRad);
    };

}

#endif //ENABLE_SIMD

#endif /* PROJECTION_INFO_SIMD_H */
