#ifndef PROJECTION_INFO_SIMD_H
#define PROJECTION_INFO_SIMD_H

#ifdef ENABLE_SIMD

#include <array>
#include <immintrin.h>     //AVX2

#include "../GeoCoordinate.h"
#include "../MapProjectionStructures.h"
#include "../ProjectionInfo.h"

#include "./MapProjectionUtils_simd.h"
#include "./MapProjectionStructures_simd.h"

#define RET_VAL_SIMD(PixelType, enable_cond) \
typename std::enable_if<enable_cond<PixelType>::value, std::array<Projections::Pixel<PixelType>, 8>>::type

namespace Projections::Simd
{
    template <typename Proj>
    class ProjectionInfoSimd
    {
        public:
        virtual ~ProjectionInfoSimd() = default;
        
        template <typename PixelType = int>
        std::array<Projections::Pixel<PixelType>, 8> Project(const std::array<Projections::Coordinate, 8> & c) const;
        
        
        template <typename PixelType = int, bool Normalize = true>
        std::array<Projections::Coordinate, 8> ProjectInverse(const std::array<Projections::Pixel<PixelType>, 8> & p) const;
        
        PixelSimd Project(const CoordinateSimd & p) const;
        CoordinateSimd ProjectInverse(const PixelSimd & p) const;
        
        protected:
        struct ProjectedValueInverseSimd
        {
            __m256 latRad;
            __m256 lonRad;
        };
        
        struct ProjectedValueSimd
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
    std::array<Projections::Pixel<PixelType>, 8> ProjectionInfoSimd<Proj>::Project(const std::array<Projections::Coordinate, 8> & c) const
    {
        CoordinateSimd cSimd = CoordinateSimd::FromArray(c);
        auto raw = this->Project(cSimd);
        return PixelSimd::ToArray<PixelType>(raw);
    };
    
    template <typename Proj>
    PixelSimd ProjectionInfoSimd<Proj>::Project(const CoordinateSimd & p) const
    {
        const Proj * tmp = static_cast<const Proj*>(this);
        auto frame = tmp->GetFrame();
        
        //project value and get "pseudo" pixel coordinate
        auto raw = tmp->ProjectInternal(p.lonRad, p.latRad);
        
        //========
        //move our pseoude pixel to "origin"
        raw.x = _mm256_sub_ps(raw.x, _mm256_set1_ps(static_cast<float>(frame.minPixelOffsetX)));
        raw.y = _mm256_sub_ps(raw.y, _mm256_set1_ps(static_cast<float>(frame.minPixelOffsetY)));
        
        PixelSimd res;
        
        res.x = _mm256_mul_ps(raw.x, _mm256_set1_ps(static_cast<float>(frame.wAR)));
        res.x = _mm256_add_ps(res.x, _mm256_set1_ps(static_cast<float>(frame.wPadding)));
        
        res.y = _mm256_mul_ps(raw.y, _mm256_set1_ps(static_cast<float>(frame.hAR)));
        res.y = _mm256_sub_ps(_mm256_set1_ps(static_cast<float>(frame.h - frame.hPadding)), res.y);
        
        //move our pseoude pixel to "origin"
        //rawPixel.x = rawPixel.x - frame.minPixelOffset.x;
        //rawPixel.y = rawPixel.y - frame.minPixelOffset.y;
        
        //calculate pixel in final frame
        //p.x = static_cast<PixelType>(frame.wPadding + (rawPixel.x * frame.wAR));
        //p.y = static_cast<PixelType>(frame.h - frame.hPadding - (rawPixel.y * frame.hAR));
        //=======
        
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
    std::array<Projections::Coordinate, 8> ProjectionInfoSimd<Proj>::ProjectInverse(const std::array<Projections::Pixel<PixelType>, 8> & p) const
    {
        PixelSimd pSimd = PixelSimd::FromArray(p);
        auto cSimd = this->ProjectInverse(pSimd);
        
        return CoordinateSimd::ToArray<Normalize>(cSimd);
    };
    
    /// <summary>
    /// Project pixels stored in SIMD register to coordinate
    /// stored in SIMD register
    /// (Does not support normalization)
    /// </summary>
    /// <param name="p"></param>
    /// <returns></returns>
    template <typename Proj>
    CoordinateSimd ProjectionInfoSimd<Proj>::ProjectInverse(const PixelSimd & p) const
    {
        const Proj * tmp = static_cast<const Proj*>(this);
        auto frame = tmp->GetFrame();
        
        __m256 x = p.x;
        __m256 y = p.y;
        
        x = _mm256_add_ps(x, _mm256_set1_ps(static_cast<float>(frame.projInvPrecomW)));
        x = _mm256_div_ps(x, _mm256_set1_ps(static_cast<float>(frame.wAR)));
        
        y = _mm256_add_ps(y, _mm256_set1_ps(static_cast<float>(frame.projInvPrecomH)));
        y = _mm256_div_ps(y, _mm256_set1_ps(static_cast<float>(-frame.hAR)));
        
        auto pi = tmp->ProjectInverseInternal(x, y);
        
        /*
         MyRealType xx = (static_cast<MyRealType>(p.x) + this->frame.projInvPrecomW);
         xx /= this->frame.wAR;
         
         MyRealType yy = (static_cast<MyRealType>(p.y) + this->frame.projInvPrecomH);
         yy /= -this->frame.hAR;
         
         ProjectedValueInverse pi = static_cast<const Proj*>(this)->ProjectInverseInternal(xx, yy);
         */
        
        CoordinateSimd c;
        c.lonRad = pi.lonRad;
        c.latRad = pi.latRad;
        
        return c;
    };

}

#endif //ENABLE_SIMD

#endif /* PROJECTION_INFO_SIMD_H */
