/**
 * @file WindowsFutex.cpp
 * @brief  implementation of FWindowsFutex.
 */

#include <Core/CoreDefinitions.hpp>

#if defined( LUMINA_PLATFORM_WINDOWS )

extern "C"
{
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
}

    #if defined( LUMINA_COMPILER_MSVC )
        #pragma comment( lib, "Synchronization.lib" )
    #elif defined( LUMINA_COMPILER_CLANG )
        #warning "WindowsFutex.cpp: clang: link with -lsynchronization"
    #endif

    #include <HAL/Windows/WindowsFutex.hpp>

/**
 * Windows
 */

void Lumina::HAL::FWindowsFutex::Wait ( TAtomic<UInt32> &InAddress, UInt32 InExpected ) noexcept
{
    UInt32 Expected = InExpected;
    static_cast<void>( WaitOnAddress( Raw( InAddress ), &Expected, sizeof( UInt32 ), INFINITE ) );
}

Lumina::Bool
Lumina::HAL::FWindowsFutex::WaitFor ( TAtomic<UInt32> &InAddress, UInt32 InExpected, Float64 InTimeoutSeconds ) noexcept
{
    /** INFO: INFINITE is 0xFFFFFFFF, so the longest finite wait is ~49 days. */
    constexpr Float64 MaxMilliseconds = 4294967294.0;

    const Float64 Milliseconds = std::ceil( ClampTimeout( InTimeoutSeconds ) * 1000.0 );
    const DWORD Timeout        = static_cast<DWORD>( Milliseconds < MaxMilliseconds ? Milliseconds : MaxMilliseconds );

    UInt32 Expected = InExpected;

    if ( WaitOnAddress( Raw( InAddress ), &Expected, sizeof( UInt32 ), Timeout ) != FALSE )
    {
        return true;
    }

    return GetLastError() != ERROR_TIMEOUT;
}

void Lumina::HAL::FWindowsFutex::WakeOne ( TAtomic<UInt32> &InAddress ) noexcept
{
    WakeByAddressSingle( Raw( InAddress ) );
}

void Lumina::HAL::FWindowsFutex::WakeAll ( TAtomic<UInt32> &InAddress ) noexcept
{
    WakeByAddressAll( Raw( InAddress ) );
}

/**
 * Private
 */

Lumina::UInt32 *Lumina::HAL::FWindowsFutex::Raw ( TAtomic<UInt32> &InAddress ) noexcept
{
    /** INFO: TAtomic<UInt32> is lock-free and 4 bytes (static_asserts in the header), so this is the usual futex idiom */
    return reinterpret_cast<UInt32 *>( &InAddress );
}

Lumina::Float64 Lumina::HAL::FWindowsFutex::ClampTimeout ( Float64 InSeconds ) noexcept
{
    constexpr Float64 MaxSeconds = 31536000.0; /** 1 year */

    /** INFO: Written so a NaN falls in the first branch */
    if ( not( InSeconds > 0.0 ) )
    {
        return 0.0;
    }

    return InSeconds < MaxSeconds ? InSeconds : MaxSeconds;
}

#endif
