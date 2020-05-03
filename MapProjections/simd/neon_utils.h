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


#endif