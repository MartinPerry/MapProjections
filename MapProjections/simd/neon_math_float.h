/* NEON implementation of sin, cos, exp and log

   Inspired by Intel Approximate Math library, and based on the
   corresponding algorithms of the cephes math library
*/

/* Copyright (C) 2011  Julien Pommier

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
	 claim that you wrote the original software. If you use this software
	 in a product, an acknowledgment in the product documentation would be
	 appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
	 misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  (this is the zlib license)
*/

#ifndef NEON_MATH_FLOAT_H
#define NEON_MATH_FLOAT_H

#include "./neon_utils.h"

//do we still HAVE_NEON ?
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

//=============================================================================


/* natural logarithm computed for 4 simultaneous float
   return NaN for x <= 0
*/
static float32x4_t my_log_f32(float32x4_t x)
{	
	const float c_cephes_SQRTHF = 0.707106781186547524f;
	const float c_cephes_log_p0 = 7.0376836292E-2f;
	const float c_cephes_log_p1 = -1.1514610310E-1f;
	const float c_cephes_log_p2 = 1.1676998740E-1f;
	const float c_cephes_log_p3 = -1.2420140846E-1f;
	const float c_cephes_log_p4 = +1.4249322787E-1f;
	const float c_cephes_log_p5 = -1.6668057665E-1f;
	const float c_cephes_log_p6 = +2.0000714765E-1f;
	const float c_cephes_log_p7 = -2.4999993993E-1f;
	const float c_cephes_log_p8 = +3.3333331174E-1f;
	const float c_cephes_log_q1 = -2.12194440e-4f;
	const float c_cephes_log_q2 = 0.693359375f;

	float32x4_t one = vdupq_n_f32(1);

	x = vmaxq_f32(x, vdupq_n_f32(0)); /* force flush to zero on denormal values */
	uint32x4_t invalid_mask = vcleq_f32(x, vdupq_n_f32(0));

	int32x4_t ux = vreinterpretq_s32_f32(x);

	int32x4_t emm0 = vshrq_n_s32(ux, 23);

	/* keep only the fractional part */
	ux = vandq_s32(ux, vdupq_n_s32(static_cast<uint32_t>(~0x7f80'0000)));
	ux = vorrq_s32(ux, vreinterpretq_s32_f32(vdupq_n_f32(0.5f)));
	x = vreinterpretq_f32_s32(ux);

	emm0 = vsubq_s32(emm0, vdupq_n_s32(0x7f));
	float32x4_t e = vcvtq_f32_s32(emm0);

	e = vaddq_f32(e, one);

	/* part2:
	   if( x < SQRTHF ) {
		 e -= 1;
		 x = x + x - 1.0;
	   } else { x = x - 1.0; }
	*/
	uint32x4_t mask = vcltq_f32(x, vdupq_n_f32(c_cephes_SQRTHF));
	float32x4_t tmp = vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(x), mask));
	x = vsubq_f32(x, one);
	e = vsubq_f32(e, vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(one), mask)));
	x = vaddq_f32(x, tmp);

	float32x4_t z = vmulq_f32(x, x);

	float32x4_t y = vdupq_n_f32(c_cephes_log_p0);
	y = vmulq_f32(y, x);
	y = vaddq_f32(y, vdupq_n_f32(c_cephes_log_p1));
	y = vmulq_f32(y, x);
	y = vaddq_f32(y, vdupq_n_f32(c_cephes_log_p2));
	y = vmulq_f32(y, x);
	y = vaddq_f32(y, vdupq_n_f32(c_cephes_log_p3));
	y = vmulq_f32(y, x);
	y = vaddq_f32(y, vdupq_n_f32(c_cephes_log_p4));
	y = vmulq_f32(y, x);
	y = vaddq_f32(y, vdupq_n_f32(c_cephes_log_p5));
	y = vmulq_f32(y, x);
	y = vaddq_f32(y, vdupq_n_f32(c_cephes_log_p6));
	y = vmulq_f32(y, x);
	y = vaddq_f32(y, vdupq_n_f32(c_cephes_log_p7));
	y = vmulq_f32(y, x);
	y = vaddq_f32(y, vdupq_n_f32(c_cephes_log_p8));
	y = vmulq_f32(y, x);

	y = vmulq_f32(y, z);


	tmp = vmulq_f32(e, vdupq_n_f32(c_cephes_log_q1));
	y = vaddq_f32(y, tmp);


	tmp = vmulq_f32(z, vdupq_n_f32(0.5f));
	y = vsubq_f32(y, tmp);

	tmp = vmulq_f32(e, vdupq_n_f32(c_cephes_log_q2));
	x = vaddq_f32(x, y);
	x = vaddq_f32(x, tmp);
	x = vreinterpretq_f32_u32(vorrq_u32(vreinterpretq_u32_f32(x), invalid_mask)); // negative arg will be NAN
	return x;
}


/* exp() computed for 4 float at once */
static float32x4_t my_exp_f32(float32x4_t x)
{
	const float c_exp_hi = 88.3762626647949f;
	const float c_exp_lo = -88.3762626647949f;

	const float c_cephes_LOG2EF = 1.44269504088896341f;
	const float c_cephes_exp_C1 = 0.693359375f;
	const float c_cephes_exp_C2 = -2.12194440e-4f;

	const float c_cephes_exp_p0 = 1.9875691500E-4f;
	const float c_cephes_exp_p1 = 1.3981999507E-3f;
	const float c_cephes_exp_p2 = 8.3334519073E-3f;
	const float c_cephes_exp_p3 = 4.1665795894E-2f;
	const float c_cephes_exp_p4 = 1.6666665459E-1f;
	const float c_cephes_exp_p5 = 5.0000001201E-1f;

	float32x4_t tmp, fx;

	float32x4_t one = vdupq_n_f32(1);
	x = vminq_f32(x, vdupq_n_f32(c_exp_hi));
	x = vmaxq_f32(x, vdupq_n_f32(c_exp_lo));

	/* express exp(x) as exp(g + n*log(2)) */
	fx = vmlaq_f32(vdupq_n_f32(0.5f), x, vdupq_n_f32(c_cephes_LOG2EF));

	/* perform a floorf */
	tmp = vcvtq_f32_s32(vcvtq_s32_f32(fx));

	/* if greater, substract 1 */
	uint32x4_t mask = vcgtq_f32(tmp, fx);
	mask = vandq_u32(mask, vreinterpretq_u32_f32(one));


	fx = vsubq_f32(tmp, vreinterpretq_f32_u32(mask));

	tmp = vmulq_f32(fx, vdupq_n_f32(c_cephes_exp_C1));
	float32x4_t z = vmulq_f32(fx, vdupq_n_f32(c_cephes_exp_C2));
	x = vsubq_f32(x, tmp);
	x = vsubq_f32(x, z);

	static const float cephes_exp_p[6] = { c_cephes_exp_p0, c_cephes_exp_p1, c_cephes_exp_p2, c_cephes_exp_p3, c_cephes_exp_p4, c_cephes_exp_p5 };
	float32x4_t y = vld1q_dup_f32(cephes_exp_p + 0);
	float32x4_t c1 = vld1q_dup_f32(cephes_exp_p + 1);
	float32x4_t c2 = vld1q_dup_f32(cephes_exp_p + 2);
	float32x4_t c3 = vld1q_dup_f32(cephes_exp_p + 3);
	float32x4_t c4 = vld1q_dup_f32(cephes_exp_p + 4);
	float32x4_t c5 = vld1q_dup_f32(cephes_exp_p + 5);

	y = vmulq_f32(y, x);
	z = vmulq_f32(x, x);
	y = vaddq_f32(y, c1);
	y = vmulq_f32(y, x);
	y = vaddq_f32(y, c2);
	y = vmulq_f32(y, x);
	y = vaddq_f32(y, c3);
	y = vmulq_f32(y, x);
	y = vaddq_f32(y, c4);
	y = vmulq_f32(y, x);
	y = vaddq_f32(y, c5);

	y = vmulq_f32(y, z);
	y = vaddq_f32(y, x);
	y = vaddq_f32(y, one);

	/* build 2^n */
	int32x4_t mm;
	mm = vcvtq_s32_f32(fx);
	mm = vaddq_s32(mm, vdupq_n_s32(0x7f));
	mm = vshlq_n_s32(mm, 23);
	float32x4_t pow2n = vreinterpretq_f32_s32(mm);

	y = vmulq_f32(y, pow2n);
	return y;
}

/* pow() computed for 4 float at once from log and exp */
static float32x4_t my_pow_f32(const float32x4_t & x, const float32x4_t & y)
{
	float32x4_t tmp = my_log_f32(x);
	return my_exp_f32(vmulq_f32(y, tmp));
}

//=============================================================================

enum class NeonSinCos {
	Sin = 1,
	Cos = 2
};

/* evaluation of 4 sines & cosines at once.

   The code is the exact rewriting of the cephes sinf function.
   Precision is excellent as long as x < 8192 (I did not bother to
   take into account the special handling they have for greater values
   -- it does not return garbage for arguments over 8192, though, but
   the extra precision is missing).

   Note that it is such that sinf((float)M_PI) = 8.74e-8, which is the
   surprising but correct result.

   Note also that when you compute sin(x), cos(x) is available at
   almost no extra price so both sin_ps and cos_ps make use of
   sincos_ps..
  */
template<int Type = int(NeonSinCos::Sin) | int(NeonSinCos::Cos)>
static void my_sincos_f32(float32x4_t x, float32x4_t *ysin, float32x4_t *ycos)
{

	const float  c_minus_cephes_DP1 = -0.78515625f;
	const float  c_minus_cephes_DP2 = -2.4187564849853515625e-4f;
	const float  c_minus_cephes_DP3 = -3.77489497744594108e-8f;
	const float  c_sincof_p0 = -1.9515295891E-4f;
	const float  c_sincof_p1 = 8.3321608736E-3f;
	const float  c_sincof_p2 = -1.6666654611E-1f;
	const float  c_coscof_p0 = 2.443315711809948E-005f;
	const float  c_coscof_p1 = -1.388731625493765E-003f;
	const float  c_coscof_p2 = 4.166664568298827E-002f;
	const float  c_cephes_FOPI = 1.27323954473516f; // 4 / M_PI

	float32x4_t xmm1, xmm2, xmm3, y;

	uint32x4_t emm2;

	uint32x4_t sign_mask_sin, sign_mask_cos;
	if (Type & int(NeonSinCos::Sin))
	{
		sign_mask_sin = vcltq_f32(x, vdupq_n_f32(0));
	}
	x = vabsq_f32(x);

	/* scale by 4/Pi */
	y = vmulq_f32(x, vdupq_n_f32(c_cephes_FOPI));

	/* store the integer part of y in mm0 */
	emm2 = vcvtq_u32_f32(y);
	/* j=(j+1) & (~1) (see the cephes sources) */
	emm2 = vaddq_u32(emm2, vdupq_n_u32(1));
	emm2 = vandq_u32(emm2, vdupq_n_u32(~1));
	y = vcvtq_f32_u32(emm2);

	/* get the polynom selection mask
	   there is one polynom for 0 <= x <= Pi/4
	   and another one for Pi/4<x<=Pi/2

	   Both branches will be computed.
	*/
	uint32x4_t poly_mask = vtstq_u32(emm2, vdupq_n_u32(2));

	/* The magic pass: "Extended precision modular arithmetic"
	   x = ((x - y * DP1) - y * DP2) - y * DP3; */
	xmm1 = vmulq_n_f32(y, c_minus_cephes_DP1);
	xmm2 = vmulq_n_f32(y, c_minus_cephes_DP2);
	xmm3 = vmulq_n_f32(y, c_minus_cephes_DP3);
	x = vaddq_f32(x, xmm1);
	x = vaddq_f32(x, xmm2);
	x = vaddq_f32(x, xmm3);

	if (Type & int(NeonSinCos::Sin))
	{
		sign_mask_sin = veorq_u32(sign_mask_sin, vtstq_u32(emm2, vdupq_n_u32(4)));
	}

	if (Type & int(NeonSinCos::Cos))
	{
		sign_mask_cos = vtstq_u32(vsubq_u32(emm2, vdupq_n_u32(2)), vdupq_n_u32(4));
	}

	/* Evaluate the first polynom  (0 <= x <= Pi/4) in y1,
	   and the second polynom      (Pi/4 <= x <= 0) in y2 */
	float32x4_t z = vmulq_f32(x, x);
	float32x4_t y1, y2;

	y1 = vmulq_n_f32(z, c_coscof_p0);
	y2 = vmulq_n_f32(z, c_sincof_p0);
	y1 = vaddq_f32(y1, vdupq_n_f32(c_coscof_p1));
	y2 = vaddq_f32(y2, vdupq_n_f32(c_sincof_p1));
	y1 = vmulq_f32(y1, z);
	y2 = vmulq_f32(y2, z);
	y1 = vaddq_f32(y1, vdupq_n_f32(c_coscof_p2));
	y2 = vaddq_f32(y2, vdupq_n_f32(c_sincof_p2));
	y1 = vmulq_f32(y1, z);
	y2 = vmulq_f32(y2, z);
	y1 = vmulq_f32(y1, z);
	y2 = vmulq_f32(y2, x);
	y1 = vsubq_f32(y1, vmulq_f32(z, vdupq_n_f32(0.5f)));
	y2 = vaddq_f32(y2, x);
	y1 = vaddq_f32(y1, vdupq_n_f32(1));

	/* select the correct result from the two polynoms */
	if (Type & int(NeonSinCos::Sin))
	{
		float32x4_t ys = vbslq_f32(poly_mask, y1, y2);
		*ysin = vbslq_f32(sign_mask_sin, vnegq_f32(ys), ys);
	}

	if (Type & int(NeonSinCos::Cos))
	{
		float32x4_t yc = vbslq_f32(poly_mask, y2, y1);
		*ycos = vbslq_f32(sign_mask_cos, yc, vnegq_f32(yc));
	}
}

static float32x4_t my_sin_f32(float32x4_t x)
{
	float32x4_t ysin, ycos;
	my_sincos_f32<int(NeonSinCos::Sin)>(x, &ysin, &ycos);
	return ysin;
}

static float32x4_t my_cos_f32(float32x4_t x)
{
	float32x4_t ysin, ycos;
	my_sincos_f32<int(NeonSinCos::Cos)>(x, &ysin, &ycos);
	return ycos;
}

static float32x4_t my_tan_f32(float32x4_t x)
{
	float32x4_t s, c;
	my_sincos_f32(x, &s, &c);
#if defined(__aarch64__) || defined(__arm64__) || defined(vdivq_f32)
	return vdivq_f32(s, c);
#else
	return vmulq_f32(s, vrecpeq_f32(c)); //s * 1/c
#endif
}

//=============================================================================

template<int Type = int(NeonSinCos::Sin) | int(NeonSinCos::Cos)>
static void my_asincos_f32(float32x4_t x, float32x4_t *s, float32x4_t *c)
{
	const float32x4_t PIO2F = vdupq_n_f32(1.5707963267948966192f);
	const float32x4_t c_05 = vdupq_n_f32(0.5);
	const float32x4_t c_1 = vdupq_n_f32(1.0);

	uint32x4_t signBit = vcltq_f32(x, vdupq_n_f32(0.0));

	x = vabsq_f32(x);

	//test if a is > 0.5
	uint32x4_t over05 = vcgtq_f32(x, c_05);

	float32x4_t z1 = vsubq_f32(c_1, x);
	z1 = vmulq_f32(z1, c_05);
#if defined(__aarch64__) || defined(__arm64__) || defined(vsqrtq_f32)
	float32x4_t x1 = vsqrtq_f32(z1);
#else
	float32x4_t x1 = vmulq_f32(vrsqrteq_f32(z1), z1);
#endif

	float32x4_t x2 = x;
	float32x4_t z2 = vmulq_f32(x2, x2);

	x = my_select_f32(over05, x1, x2);
	float32x4_t z = my_select_f32(over05, z1, z2);

	//Intel _mm256_fmadd_ps has different order than vmlaq_f32
	float32x4_t pz = vmlaq_f32(vdupq_n_f32(2.4181311049E-2f), z, vdupq_n_f32(4.2163199048E-2f));
	pz = vmlaq_f32(vdupq_n_f32(4.5470025998E-2f), pz, z);
	pz = vmlaq_f32(vdupq_n_f32(7.4953002686E-2f), pz, z);
	pz = vmlaq_f32(vdupq_n_f32(1.6666752422E-1f), pz, z);
	pz = vmulq_f32(pz, z);
	z = vmlaq_f32(x, pz, x);
	
	float32x4_t tmp2z = vaddq_f32(z, z);

	if (Type & int(NeonSinCos::Cos))
	{
		const float32x4_t PIF = vdupq_n_f32(3.14159265358979323846f);

		float32x4_t tmp = my_select_f32(signBit, vsubq_f32(PIF, tmp2z), tmp2z);
		float32x4_t tmp2 = vsubq_f32(PIO2F, my_select_f32(signBit, my_swap_sign_f32(z), z));
		*c = my_select_f32(over05, tmp, tmp2);
	}

	if (Type & int(NeonSinCos::Sin))
	{
		float32x4_t tmp = vsubq_f32(PIO2F, tmp2z);
		z1 = my_select_f32(over05, tmp, z);
		*s = my_select_f32(signBit, my_swap_sign_f32(z1), z1);
	}
}

static float32x4_t my_asin_f32(float32x4_t x)
{
	float32x4_t s, c;
	my_asincos_f32<int(NeonSinCos::Sin)>(x, &s, &c);
	return s;
}

static float32x4_t my_acos_f32(float32x4_t x)
{
	float32x4_t s, c;
	my_asincos_f32<int(NeonSinCos::Cos)>(x, &s, &c);
	return c;
}

static float32x4_t my_atan_f32(float32x4_t x)
{
	const float PIO2F = 1.5707963267948966192f;
	const float PIO4F = 0.7853981633974483096f;
	const float TAN_3_PI_DIV_8 = 2.414213562373095f;
	const float TAN_PI_DIV_8 = 0.4142135623730950f;

	const float32x4_t c_m1 = vdupq_n_f32(-1.0);

	uint32x4_t signBit = vcltq_f32(x, vdupq_n_f32(0.0));

	x = vabsq_f32(x);

	// small:  x < TAN_PI_DIV_8
	// medium: TAN_PI_DIV_8 <= x <= TAN_3_PI_DIV_8
	// big:    x > TAN_3_PI_DIV_8
	uint32x4_t big = vcgtq_f32(x, vdupq_n_f32(TAN_3_PI_DIV_8));
	uint32x4_t medium = vcgtq_f32(x, vdupq_n_f32(TAN_PI_DIV_8));

	float32x4_t s = my_select_f32(big, vdupq_n_f32(PIO2F),
		my_select_f32(medium, vdupq_n_f32(PIO4F), vdupq_n_f32(0.0)));

#if defined(__aarch64__) || defined(__arm64__) || defined(vdivq_f32)
	float32x4_t tDiv = vdivq_f32(c_m1, x); //-1./x
#else
	float32x4_t tDiv = vmulq_f32(c_m1, vrecpeq_f32(x)); //-1./x

#endif

	float32x4_t t1 = vaddq_f32(x, c_m1); //x+(-1) => x-1
	float32x4_t t2 = vsubq_f32(x, c_m1); //x-(-1) => x+1

#if defined(__aarch64__) || defined(__arm64__) || defined(vdivq_f32)
	float32x4_t t1t2Div = vdivq_f32(t1, t2);
#else
	float32x4_t t1t2Div = vmulq_f32(t1, vrecpeq_f32(t2)); //t1 * 1/t2
#endif
	x = my_select_f32(big, tDiv, my_select_f32(medium, t1t2Div, x));

	float32x4_t z = vmulq_f32(x, x);

	//s += ((( 8.05374449538e-2 * z - 1.38776856032E-1) * z + 1.99777106478E-1) * z - 3.33329491539E-1) * z * x + x;

	float32x4_t pz = vmlaq_f32(vdupq_n_f32(-1.38776856032E-1f), z, vdupq_n_f32(8.05374449538e-2f));
	pz = vmlaq_f32(vdupq_n_f32(1.99777106478E-1f), pz, z);
	pz = vmlaq_f32(vdupq_n_f32(-3.33329491539E-1f), pz, z);
	pz = vmulq_f32(pz, z);
	z = vmlaq_f32(x, pz, x);
	s = vaddq_f32(s, z);

	return my_select_f32(signBit, my_swap_sign_f32(s), s);
}

static float32x4_t my_atan2_f32(float32x4_t y, float32x4_t x)
{
	const float PIF = 3.14159265358979323846f;
	const float PIO2F = 1.5707963267948966192f;
	const float PIO4F = 0.7853981633974483096f;
	const float TAN_PI_DIV_8 = 0.4142135623730950f;

	const float32x4_t c_m1 = vdupq_n_f32(-1.0);
	const float32x4_t c_0 = vdupq_n_f32(0.0);

	uint32x4_t signBitX = vcltq_f32(x, c_0);
	uint32x4_t signBitY = vcltq_f32(y, c_0);

	float32x4_t x1 = vabsq_f32(x);
	float32x4_t y1 = vabsq_f32(y);

	uint32x4_t swapxy = vcgtq_f32(y1, x1);
	float32x4_t x2 = my_select_f32(swapxy, y1, x1);
	float32x4_t y2 = my_select_f32(swapxy, x1, y1);
#if defined(__aarch64__) || defined(__arm64__) || defined(vdivq_f32)
	float32x4_t t = vdivq_f32(y2, x2);
#else
	float32x4_t t = vmulq_f32(y2, vrecpeq_f32(x2)); //y2 * 1/x2
#endif

	uint32x4_t medium = vcgtq_f32(t, vdupq_n_f32(TAN_PI_DIV_8));
	float32x4_t s = my_select_f32(medium, vdupq_n_f32(PIO4F), c_0);
	float32x4_t t1 = vaddq_f32(t, c_m1); //t+(-1) => t-1
	float32x4_t t2 = vsubq_f32(t, c_m1); //t-(-1) => t+1
#if defined(__aarch64__) || defined(__arm64__) || defined(vdivq_f32)
	float32x4_t z = my_select_f32(medium, vdivq_f32(t1, t2), t);
#else
	float32x4_t estimateDiv = vmulq_f32(t1, vrecpeq_f32(t2)); //t1 * 1/t2
	float32x4_t z = my_select_f32(medium, estimateDiv, t);
#endif
	float32x4_t zz = vmulq_f32(z, z);

	float32x4_t pz = vmlaq_f32(vdupq_n_f32(-1.38776856032E-1f), zz, vdupq_n_f32(8.05374449538e-2f));
	pz = vmlaq_f32(vdupq_n_f32(1.99777106478E-1f), pz, zz);
	pz = vmlaq_f32(vdupq_n_f32(-3.33329491539E-1f), pz, zz);
	pz = vmulq_f32(pz, zz);
	z = vmlaq_f32(z, pz, z);
	s = vaddq_f32(s, z);

	s = my_select_f32(swapxy, vsubq_f32(vdupq_n_f32(PIO2F), s), s);
	s = my_select_f32(signBitX, vsubq_f32(vdupq_n_f32(PIF), s), s);

	//re = select((x | y) == 0.f, 0.f, re);    // atan2(0,0) = 0 by convention

	return my_select_f32(signBitY, my_swap_sign_f32(s), s);
}


#endif

#endif