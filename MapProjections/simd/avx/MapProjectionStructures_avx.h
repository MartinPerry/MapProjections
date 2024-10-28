#ifndef MAP_PROJECTION_STRUCTURES_SIMD_H
#define MAP_PROJECTION_STRUCTURES_SIMD_H

#ifdef ENABLE_SIMD

#include <array>

#include <immintrin.h>     //AVX2

#include "../../MapProjectionStructures.h"

#define RET_VAL_SIMD(PixelType, enable_cond) \
typename std::enable_if<enable_cond<PixelType>::value, std::array<Projections::Pixel<PixelType>, 8>>::type

namespace Projections::Avx
{
    //=======================================================================================
    // Pixel
    //=======================================================================================

    struct PixelAvx
    {
        __m256 x;
        __m256 y;
        
        /*
        PixelAvx()
        {
            //not needed?
            //x = _mm256_setzero_ps();
            //y = _mm256_setzero_ps();
        }
         */
        
        template <typename PixelType>
        static PixelAvx FromArray(const std::array<Projections::Pixel<PixelType>, 8> & p)
        {
            PixelAvx pAvx;
            pAvx.x = _mm256_set_ps(static_cast<float>(p[7].x), 
				static_cast<float>(p[6].x), 
				static_cast<float>(p[5].x), 
				static_cast<float>(p[4].x), 
				static_cast<float>(p[3].x), 
				static_cast<float>(p[2].x), 
				static_cast<float>(p[1].x), 
				static_cast<float>(p[0].x));

            pAvx.y = _mm256_set_ps(static_cast<float>(p[7].y), 
				static_cast<float>(p[6].y), 
				static_cast<float>(p[5].y), 
				static_cast<float>(p[4].y), 
				static_cast<float>(p[3].y), 
				static_cast<float>(p[2].y), 
				static_cast<float>(p[1].y), 
				static_cast<float>(p[0].y));

            return pAvx;
        };
        
        template <typename PixelType>
        static RET_VAL_SIMD(PixelType, std::is_integral) ToArray(const PixelAvx & pAvx)
        {
            std::array<Pixel<PixelType>, 8> p;
            
            std::array<float, 8> resX;
            _mm256_storeu_ps(resX.data(), pAvx.x);
            
            std::array<float, 8> resY;
            _mm256_storeu_ps(resY.data(), pAvx.y);
            
            //calculate pixel in final frame
            for (size_t i = 0; i < p.size(); i++)
            {
                p[i].x = static_cast<PixelType>(std::round(resX[i]));
                p[i].y = static_cast<PixelType>(std::round(resY[i]));
            }
            return p;
        };
        
        template <typename PixelType>
        static RET_VAL_SIMD(PixelType, std::is_floating_point) ToArray(const PixelAvx & pAvx)
        {
            std::array<Pixel<PixelType>, 8> p;
            
            std::array<float, 8> resX;
            _mm256_storeu_ps(resX.data(), pAvx.x);
            
            std::array<float, 8> resY;
            _mm256_storeu_ps(resY.data(), pAvx.y);
            
            //calculate pixel in final frame
            for (size_t i = 0; i < p.size(); i++)
            {
                p[i].x = static_cast<PixelType>(resX[i]);
                p[i].y = static_cast<PixelType>(resY[i]);
            }
            return p;
        };
    };
    
    //=======================================================================================
    // GPS
    //=======================================================================================

    struct CoordinateAvx
    {
        __m256 lonRad;
        __m256 latRad;
        
        CoordinateAvx() :
            lonRad({}),
            latRad({})
        {};

        CoordinateAvx(const __m256 & lonRad, const __m256 & latRad) : 
            lonRad(lonRad), 
            latRad(latRad) 
        {};
        
        static CoordinateAvx FromArray(const std::array<Projections::Coordinate, 8> & c)
        {
            CoordinateAvx cAvx;
            cAvx.lonRad = _mm256_set_ps(static_cast<float>(c[7].lon.rad()), 
				static_cast<float>(c[6].lon.rad()), 
				static_cast<float>(c[5].lon.rad()), 
				static_cast<float>(c[4].lon.rad()), 
				static_cast<float>(c[3].lon.rad()), 
				static_cast<float>(c[2].lon.rad()), 
				static_cast<float>(c[1].lon.rad()), 
				static_cast<float>(c[0].lon.rad()));

            cAvx.latRad = _mm256_set_ps(static_cast<float>(c[7].lat.rad()), 
				static_cast<float>(c[6].lat.rad()), 
				static_cast<float>(c[5].lat.rad()), 
				static_cast<float>(c[4].lat.rad()), 
				static_cast<float>(c[3].lat.rad()), 
				static_cast<float>(c[2].lat.rad()), 
				static_cast<float>(c[1].lat.rad()), 
				static_cast<float>(c[0].lat.rad()));

            return cAvx;
        };
        
        template <bool Normalize>
        static std::array<Projections::Coordinate, 8> ToArray(const CoordinateAvx & cAvx)
        {
            std::array<Projections::Coordinate, 8> c;
            
            std::array<float, 8> resLatRad;
            _mm256_storeu_ps(resLatRad.data(), cAvx.latRad);
            
            std::array<float, 8> resLonRad;
            _mm256_storeu_ps(resLonRad.data(), cAvx.lonRad);
            
            if (Normalize)
            {
                //todo
                //c.lat = Latitude::deg(ProjectionUtils::NormalizeLat(pi.lat.deg()));
                //c.lon = Longitude::deg(ProjectionUtils::NormalizeLon(pi.lon.deg()));
            }
            else
            {
                for (size_t i = 0; i < c.size(); i++)
                {
                    c[i].lat = Latitude::rad(resLatRad[i]);
                    c[i].lon = Longitude::rad(resLonRad[i]);
                }
            }
            return c;
        };
    };
}

#endif //ENABLE_SIMD

#endif /* MAP_PROJECTION_STRUCTURES_SIMD_H */
