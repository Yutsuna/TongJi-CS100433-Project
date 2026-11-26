/**
 * @file SIMD.hpp
 * @brief Declaration of SIMD utilities for mathematical operations.
 */

#pragma once

#include <Maths/SIMD/SIMDTypes.hpp>

#if defined( __x86_64__ ) || defined( _M_X64 ) || defined( __i386__ )
    #include <Maths/SIMD/Backend/SIMDSSE.hpp>
    #define LUMINA_SIMD_SSE 1
#elif defined( __aarch64__ ) || defined( _M_ARM64 )
    #include <Maths/SIMD/Backend/SIMDNeon.hpp>
    #define LUMINA_SIMD_NEON 1
#else
    #define LUMINA_SIMD_GENERIC 1
#endif

namespace Lumen
{

namespace Maths
{

    namespace SIMD
    {

#if defined( LUMINA_SIMD_SSE )
        using FSimdFloat = FSSEFloat32x4;
#elif defined( LUMINA_SIMD_NEON )
        using FSimdFloat = FNeonFloat32x4;
#else
        using FSimdFloat = FGenericFloat32x4;
#endif

    } // namespace SIMD

} // namespace Maths

} // namespace Lumen
