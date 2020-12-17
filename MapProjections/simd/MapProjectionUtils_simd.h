#ifndef MAP_PROJECTION_UTILS_SIMD_H
#define MAP_PROJECTION_UTILS_SIMD_H

#ifdef ENABLE_SIMD

#include <vector>
#include <array>
#include <immintrin.h>     //AVX2

#include "./ProjectionInfo_simd.h"
#include "./MapProjectionStructures_simd.h"

namespace Projections::Simd
{
    
    struct ProjectionUtils
    {
                               
        inline static __m256 degToRad(const __m256 & x) { return _mm256_mul_ps(x, _mm256_set1_ps(0.0174532925f)); }
        inline static __m256 radToDeg(const __m256 & x) { return _mm256_mul_ps(x, _mm256_set1_ps(57.2957795f)); }
    };
    
};

#endif //ENABLE_SIMD

#endif
