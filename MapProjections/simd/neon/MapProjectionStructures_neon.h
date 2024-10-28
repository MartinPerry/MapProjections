#ifndef MAP_PROJECTION_STRUCTURES_NEON_H
#define MAP_PROJECTION_STRUCTURES_NEON_H

#ifdef HAVE_NEON

#include <array>

#include "./neon_utils.h"
#include "./neon_math_float.h"

#include "../../MapProjectionStructures.h"

#define RET_VAL_NEON(PixelType, enable_cond) \
typename std::enable_if<enable_cond<PixelType>::value, std::array<Projections::Pixel<PixelType>, 4>>::type

namespace Projections::Neon
{
    struct PixelNeon
    {
        float32x4_t x;
        float32x4_t y;
                
        
        template <typename PixelType>
        static PixelNeon FromArray(const std::array<Projections::Pixel<PixelType>, 4> & p)
        {
            PixelNeon pNeon;
            pNeon.x = {
               static_cast<float>(p[0].x),
               static_cast<float>(p[1].x),
               static_cast<float>(p[2].x),
               static_cast<float>(p[3].x)
            };
            pNeon.y = {
               static_cast<float>(p[0].y),
               static_cast<float>(p[1].y),
               static_cast<float>(p[2].y),
               static_cast<float>(p[3].y)
            };
            
            return pNeon;
        };
        
        template <typename PixelType>
        static RET_VAL_NEON(PixelType, std::is_integral) ToArray(const PixelNeon & pNeon)
        {
            std::array<Pixel<PixelType>, 4> p;
            
            std::array<float, 4> resX;
            vst1q_f32(resX.data(), pNeon.x);
            
            std::array<float, 4> resY;
            vst1q_f32(resY.data(), pNeon.y);
            
            //calculate pixel in final frame
            for (size_t i = 0; i < p.size(); i++)
            {
                p[i].x = static_cast<PixelType>(std::round(resX[i]));
                p[i].y = static_cast<PixelType>(std::round(resY[i]));
            }
            return p;
        };
        
        template <typename PixelType>
        static RET_VAL_NEON(PixelType, std::is_floating_point) ToArray(const PixelNeon & pNeon)
        {
            std::array<Pixel<PixelType>, 4> p;
            
            std::array<float, 4> resX;
            vst1q_f32(resX.data(), pNeon.x);
            
            std::array<float, 4> resY;
            vst1q_f32(resY.data(), pNeon.y);
            
            //calculate pixel in final frame
            for (size_t i = 0; i < p.size(); i++)
            {
                p[i].x = static_cast<PixelType>(resX[i]);
                p[i].y = static_cast<PixelType>(resY[i]);
            }
            return p;
        };
    };
    
    struct CoordinateNeon
    {
        float32x4_t lonRad;
        float32x4_t latRad;
        
        CoordinateNeon() :
            lonRad({}),
            latRad({})
        {};

        CoordinateNeon(const float32x4_t& lonRad, const float32x4_t& latRad) :
            lonRad(lonRad), 
            latRad(latRad) 
        {};
        
        static CoordinateNeon FromArray(const std::array<Projections::Coordinate, 4>& c)
        {
            CoordinateNeon cNeon;
            cNeon.lonRad = {
                static_cast<float>(c[0].lon.rad()),
                static_cast<float>(c[1].lon.rad()),
                static_cast<float>(c[2].lon.rad()),
                static_cast<float>(c[3].lon.rad())
            };
            cNeon.latRad = {
                static_cast<float>(c[0].lat.rad()),
                static_cast<float>(c[1].lat.rad()),
                static_cast<float>(c[2].lat.rad()),
                static_cast<float>(c[3].lat.rad())
            };

            return cNeon;
        };
        
        template <bool Normalize>
        static std::array<Projections::Coordinate, 4> ToArray(const CoordinateNeon & cNeon)
        {
            std::array<Projections::Coordinate, 4> c;
            
            std::array<float, 4> resLatRad;
            vst1q_f32(resLatRad.data(), cNeon.latRad);
            
            std::array<float, 4> resLonRad;
            vst1q_f32(resLonRad.data(), cNeon.lonRad);
            
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


        static std::array<Projections::Coordinate::PrecomputedSinCos, 4> PrecalcMultipleSinCos(
            const CoordinateNeon& coords)
        {
            std::array<Projections::Coordinate::PrecomputedSinCos, 4> res;

              
            float32x4_t ysin;
            float32x4_t ycos;
            float values[4];

            my_sincos_f32(coords.latRad, &ysin, &ycos);            

            vst1q_f32(values, ysin);
            res[0].sinLat = values[0];                
            res[1].sinLat = values[1];
            res[2].sinLat = values[2];
            res[3].sinLat = values[3];

            vst1q_f32(values, ycos);
            res[0].cosLat = values[0];
            res[1].cosLat = values[1];
            res[2].cosLat = values[2];
            res[3].cosLat = values[3];
            

            my_sincos_f32(coords.lonRad, &ysin, &ycos);

            vst1q_f32(values, ysin);
            res[0].sinLon = values[0];
            res[1].sinLon = values[1];
            res[2].sinLon = values[2];
            res[3].sinLon = values[3];

            vst1q_f32(values, ycos);
            res[0].cosLon = values[0];
            res[1].cosLon = values[1];
            res[2].cosLon = values[2];
            res[3].cosLon = values[3];

            return res;
        }
      
        static CoordinateNeon CreateFromCartesianLHSystem(
            const std::array<double, 4>& vx,
            const std::array<double, 4>& vy,
            const std::array<double, 4>& vz)
        {
            float32x4_t x =
            {
                static_cast<float32_t>(vx[0]),
                static_cast<float32_t>(vx[1]),
                static_cast<float32_t>(vx[2]),
                static_cast<float32_t>(vx[3])
            };

            float32x4_t y =
            {
                static_cast<float32_t>(vy[0]),
                static_cast<float32_t>(vy[1]),
                static_cast<float32_t>(vy[2]),
                static_cast<float32_t>(vy[3])
            };

            float32x4_t z =
            {
                static_cast<float32_t>(vz[0]),
                static_cast<float32_t>(vz[1]),
                static_cast<float32_t>(vz[2]),
                static_cast<float32_t>(vz[3])
            };


            float32x4_t sum = vmulq_f32(x, x);
            sum = vaddq_f32(sum, vmulq_f32(y, y));
            sum = vaddq_f32(sum, vmulq_f32(z, z));

#if defined(__aarch64__) || defined(__arm64__) || (defined(vdivq_f32) && defined(vsqrtq_f32))
            float32x4_t radius = vsqrtq_f32(sum);
            float32x4_t rInv = vdivq_f32(vmovq_n_f32(1), radius);
#else
            float32x4_t radius = vmulq_f32(vrsqrteq_f32(sum), sum);
            float32x4_t rInv = vrecpeq_f32(radius); //estimate 1.0 / radius
#endif	

            CoordinateNeon res;

            res.latRad = my_asin_f32(vmulq_f32(y, rInv));
            res.lonRad = my_atan2_f32(x, vmulq_n_f32(z, -1.0f));

            return res;
        }
    };
}

#endif //ENABLE_SIMD

#endif /* MAP_PROJECTION_STRUCTURES_SIMD_H */
