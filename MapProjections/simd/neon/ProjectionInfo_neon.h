#ifndef PROJECTION_INFO_NEON_H
#define PROJECTION_INFO_NEON_H

#ifdef HAVE_NEON

#include <array>

#include "../../GeoCoordinate.h"
#include "../../MapProjectionStructures.h"
#include "../../ProjectionInfo.h"

#include "./neon_utils.h"

#include "./MapProjectionUtils_neon.h"
#include "./MapProjectionStructures_neon.h"


#define RET_VAL_NEON(PixelType, enable_cond) \
typename std::enable_if<enable_cond<PixelType>::value, std::array<Projections::Pixel<PixelType>, 4>>::type

namespace Projections::Neon
{
    template <typename Proj>
    class ProjectionInfoNeon
    {
        public:
        virtual ~ProjectionInfoNeon() = default;
        
        template <typename PixelType = int>
        std::array<Projections::Pixel<PixelType>, 4> Project(const std::array<Projections::Coordinate, 4> & c) const;
        
        
        template <typename PixelType = int, bool Normalize = true>
        std::array<Projections::Coordinate, 4> ProjectInverse(const std::array<Projections::Pixel<PixelType>, 4> & p) const;
        
        PixelNeon Project(const CoordinateNeon & p) const;
        CoordinateNeon ProjectInverse(const PixelNeon & p) const;
        
        protected:
        struct ProjectedValueInverseNeon
        {
            float32x4_t latRad;
            float32x4_t lonRad;
        };
        
        struct ProjectedValueNeon
        {
            float32x4_t x;
            float32x4_t y;
        };
    };
    
    
    /// <summary>
    /// Project Coordinate point to pixel
    /// </summary>
    /// <param name="c"></param>
    /// <returns></returns>
    template <typename Proj>
    template <typename PixelType>
    std::array<Projections::Pixel<PixelType>, 4> ProjectionInfoNeon<Proj>::Project(const std::array<Projections::Coordinate, 4> & c) const
    {
        CoordinateNeon cNeon = CoordinateNeon::FromArray(c);
        auto raw = this->Project(cNeon);
        return PixelNeon::ToArray<PixelType>(raw);
    };
    
    template <typename Proj>
    PixelNeon ProjectionInfoNeon<Proj>::Project(const CoordinateNeon & p) const
    {
        const Proj * tmp = static_cast<const Proj*>(this);
        const auto& frame = tmp->GetFrame();
        
        //project value and get "pseudo" pixel coordinate
        auto raw = tmp->ProjectInternal(p.lonRad, p.latRad);
        
		PixelNeon res;

		res.x = vmulq_f32(raw.x, vdupq_n_f32(static_cast<float>(frame.wAR)));
		res.x = vsubq_f32(res.x, vdupq_n_f32(static_cast<float>(frame.projPrecomX)));

		res.y = vmulq_f32(raw.y, vdupq_n_f32(static_cast<float>(-frame.hAR)));
		res.y = vsubq_f32(res.y, vdupq_n_f32(static_cast<float>(frame.projPrecomY)));
		       
        return res;
    }
    
    /// <summary>
    /// Project 4 pixels to coordinate at once
    /// Return result as coordinate
    /// </summary>
    /// <param name="p"></param>
    /// <returns></returns>
    template <typename Proj>
    template <typename PixelType, bool Normalize>
    std::array<Projections::Coordinate, 4> ProjectionInfoNeon<Proj>::ProjectInverse(const std::array<Projections::Pixel<PixelType>, 4> & p) const
    {
        PixelNeon pNeon = PixelNeon::FromArray(p);
        auto cNeon = this->ProjectInverse(pNeon);
        
        return CoordinateNeon::ToArray<Normalize>(cNeon);
    };
    
    /// <summary>
    /// Project pixels stored in SIMD register to coordinate
    /// stored in SIMD register
    /// (Does not support normalization)
    /// </summary>
    /// <param name="p"></param>
    /// <returns></returns>
    template <typename Proj>
    CoordinateNeon ProjectionInfoNeon<Proj>::ProjectInverse(const PixelNeon & p) const
    {
        const Proj * tmp = static_cast<const Proj*>(this);
        const auto& frame = tmp->GetFrame();
        
        float32x4_t x = p.x;
        float32x4_t y = p.y;
        
        x = vaddq_f32(x, vdupq_n_f32(static_cast<float>(frame.projPrecomX)));        
#if defined(__aarch64__) || defined(__arm64__) || defined(vdivq_f32)
        x = vdivq_f32(x, vdupq_n_f32(static_cast<float>(frame.wAR)));
#else
        x = vmulq_f32(x, vrecpeq_f32(vdupq_n_f32(static_cast<float>(frame.wAR)))); //y / x => y * 1/x        
#endif        

        y = vaddq_f32(y, vdupq_n_f32(static_cast<float>(frame.projPrecomY)));             
#if defined(__aarch64__) || defined(__arm64__) || defined(vdivq_f32)
        y = vdivq_f32(y, vdupq_n_f32(static_cast<float>(-frame.hAR)));
#else
        y = vmulq_f32(y, vrecpeq_f32(vdupq_n_f32(static_cast<float>(-frame.hAR)))); //y / x => y * 1/x        
#endif

        auto pi = tmp->ProjectInverseInternal(x, y);
        
        return CoordinateNeon(pi.lonRad, pi.latRad);
    };

}

#endif //ENABLE_SIMD

#endif /* PROJECTION_INFO_SIMD_H */
