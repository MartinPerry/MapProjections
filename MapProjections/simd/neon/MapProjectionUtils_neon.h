#ifndef MAP_PROJECTION_UTILS_NEON_H
#define MAP_PROJECTION_UTILS_NEON_H

#ifdef HAVE_NEON

#include <vector>
#include <array>

#include "./neon_utils.h"

#include "./ProjectionInfo_neon.h"
#include "./MapProjectionStructures_neon.h"

namespace Projections::Neon
{
    
    struct ProjectionUtils
    {
                               
        inline static float32x4_t degToRad(const float32x4_t& x)
        { 
            return vmulq_f32(x, vdupq_n_f32(0.0174532925f));
        }

        inline static float32x4_t radToDeg(const float32x4_t& x)
        { 
            return vmulq_f32(x, vdupq_n_f32(57.2957795f));
        }
    };
    
};

#endif //ENABLE_SIMD

#endif
