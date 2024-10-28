#ifndef NEON_UTILS_H
#define NEON_UTILS_H


#ifdef HAVE_NEON
#	if defined(_WIN32) || defined(__i386__) || defined(__AVX__)
#		if __has_include("./NEON_2_SSE.h")
#			include "./NEON_2_SSE.h"
#		else
//we are on x86/x84 platform without NEON_2_SSE
//disable NEON
#			undef HAVE_NEON
#		endif
#	else
#		include <arm_neon.h>
#	endif
#endif



#ifdef HAVE_NEON



static inline float32x4_t my_swap_sign_f32(const float32x4_t & v)
{
	uint32_t c = static_cast<uint32_t>(~0x7FFF'FFFF);
	auto res = veorq_u32(vreinterpretq_u32_f32(v), vdupq_n_u32(c));
	return vreinterpretq_f32_u32(res);
	//return _mm256_xor_ps(v, *(__m256*)_i32_sign_mask); //~0x7FFF'FFFF
}

///<summary>
/// Select and return a or b based on mask value.
/// If mask value is 1, return a; else return b
///</summary>
static inline float32x4_t my_select_f32(const uint32x4_t & mask, const float32x4_t & a, const float32x4_t & b)
{
	// mask != 0 ? a : b
	return vbslq_f32(mask, a, b);
}

/// <summary>
/// Linear interpolation (lerp) value between start and end
/// </summary>
/// <param name="start"></param>
/// <param name="end"></param>
/// <param name="v"></param>
/// <returns></returns>
static inline float32x4_t my_lerp_f32(const float32x4_t & start, const float32x4_t & end, const float32x4_t & v)
{
	float32x4_t tmp = vsubq_f32(start, vmulq_f32(v, start));
	return vaddq_f32(tmp, vmulq_f32(v, end));
}

/// <summary>
/// Test if all parts of int32x4_t are in range [0, upper]
/// 1 -> in range
/// 0 -> not in range
/// Based on:
/// https://stackoverflow.com/questions/22078303/in-c-binary-testing-to-see-if-a-number-is-in-range
/// </summary>
/// <param name="x"></param>
/// <param name="upper"></param>
/// <returns></returns>
static inline int32x4_t my_in_range_0_upper(int32x4_t x, int32x4_t upper)
{
	int32x4_t ux = vsubq_s32(upper, x);
	int32x4_t p = vshrq_n_s32(x, 31);

	int32x4_t tmp1 = veorq_s32(upper, x);
	int32x4_t tmp2 = veorq_s32(ux, upper);
	int32x4_t tmp = vandq_s32(tmp1, tmp2);
	tmp = veorq_s32(ux, tmp);
	int32x4_t q = vshrq_n_s32(tmp, 31);

	int32x4_t neg = vmvnq_u32(vorrq_s32(p, q));
	return vandq_s32(vdupq_n_s32(1), neg);
}

#endif


#endif