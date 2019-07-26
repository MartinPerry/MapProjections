#ifndef MAP_PROJECTION_STRUCTURES_SIMD_H
#define MAP_PROJECTION_STRUCTURES_SIMD_H

#ifdef ENABLE_SIMD

#include <array>

#include <immintrin.h>     //AVX2

#include "../MapProjectionStructures.h"

#define RET_VAL_SIMD(PixelType, enable_cond) \
typename std::enable_if<enable_cond<PixelType>::value, std::array<Projections::Pixel<PixelType>, 8>>::type

namespace Projections::Simd
{
    struct PixelSimd
    {
        __m256 x;
        __m256 y;
        
        /*
        PixelSimd()
        {
            //not needed?
            //x = _mm256_setzero_ps();
            //y = _mm256_setzero_ps();
        }
         */
        
        template <typename PixelType>
        static PixelSimd FromArray(const std::array<Projections::Pixel<PixelType>, 8> & p)
        {
            PixelSimd pSimd;
            pSimd.x = _mm256_set_ps(static_cast<float>(p[7].x), 
				static_cast<float>(p[6].x), 
				static_cast<float>(p[5].x), 
				static_cast<float>(p[4].x), 
				static_cast<float>(p[3].x), 
				static_cast<float>(p[2].x), 
				static_cast<float>(p[1].x), 
				static_cast<float>(p[0].x));

            pSimd.y = _mm256_set_ps(static_cast<float>(p[7].y), 
				static_cast<float>(p[6].y), 
				static_cast<float>(p[5].y), 
				static_cast<float>(p[4].y), 
				static_cast<float>(p[3].y), 
				static_cast<float>(p[2].y), 
				static_cast<float>(p[1].y), 
				static_cast<float>(p[0].y));

            return pSimd;
        };
        
        template <typename PixelType>
        static RET_VAL_SIMD(PixelType, std::is_integral) ToArray(const PixelSimd & pSimd)
        {
            std::array<Pixel<PixelType>, 8> p;
            
            float resX[p.size()];
            _mm256_storeu_ps(resX, pSimd.x);
            
            float resY[p.size()];
            _mm256_storeu_ps(resY, pSimd.y);
            
            //calculate pixel in final frame
            for (size_t i = 0; i < p.size(); i++)
            {
                p[i].x = static_cast<PixelType>(std::round(resX[i]));
                p[i].y = static_cast<PixelType>(std::round(resY[i]));
            }
            return p;
        };
        
        template <typename PixelType>
        static RET_VAL_SIMD(PixelType, std::is_floating_point) ToArray(const PixelSimd & pSimd)
        {
            std::array<Pixel<PixelType>, 8> p;
            
            float resX[p.size()];
            _mm256_storeu_ps(resX, pSimd.x);
            
            float resY[p.size()];
            _mm256_storeu_ps(resY, pSimd.y);
            
            //calculate pixel in final frame
            for (size_t i = 0; i < p.size(); i++)
            {
                p[i].x = static_cast<PixelType>(resX[i]);
                p[i].y = static_cast<PixelType>(resY[i]);
            }
            return p;
        };
    };
    
    struct CoordinateSimd
    {
        __m256 lonRad;
        __m256 latRad;
        
        CoordinateSimd() {};
        CoordinateSimd(const __m256 & lonRad, const __m256 & latRad) : lonRad(lonRad), latRad(latRad) {};
        
        static CoordinateSimd FromArray(const std::array<Projections::Coordinate, 8> & c)
        {
            CoordinateSimd cSimd;
            cSimd.lonRad = _mm256_set_ps(static_cast<float>(c[7].lon.rad()), 
				static_cast<float>(c[6].lon.rad()), 
				static_cast<float>(c[5].lon.rad()), 
				static_cast<float>(c[4].lon.rad()), 
				static_cast<float>(c[3].lon.rad()), 
				static_cast<float>(c[2].lon.rad()), 
				static_cast<float>(c[1].lon.rad()), 
				static_cast<float>(c[0].lon.rad()));

            cSimd.latRad = _mm256_set_ps(static_cast<float>(c[7].lat.rad()), 
				static_cast<float>(c[6].lat.rad()), 
				static_cast<float>(c[5].lat.rad()), 
				static_cast<float>(c[4].lat.rad()), 
				static_cast<float>(c[3].lat.rad()), 
				static_cast<float>(c[2].lat.rad()), 
				static_cast<float>(c[1].lat.rad()), 
				static_cast<float>(c[0].lat.rad()));

            return cSimd;
        };
        
        template <bool Normalize>
        static std::array<Projections::Coordinate, 8> ToArray(const CoordinateSimd & cSimd)
        {
             std::array<Projections::Coordinate, 8> c;
            
            float resLatRad[c.size()];
            _mm256_storeu_ps(resLatRad, cSimd.latRad);
            
            float resLonRad[c.size()];
            _mm256_storeu_ps(resLonRad, cSimd.lonRad);
            
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
