/**
 * @file UnixSpinLockMutex.inl
 * @brief Inline implementation of the Unix spin lock mutex.
 */

#pragma once

#if not defined( LUMINA_PLATFORM_UNIX )
    #if not defined( LUMINA_CLANG_TIDY )
        #error "FUnixSpinLockMutex.hpp is only available on Unix platforms."
    #endif
#else

    #include <HAL/PlatformAtomic.hpp>

    #include <HAL/Unix/UnixSpinLockMutex.hpp>

    #include <sched.h>

/**
 * Public
 */

void Lumina::HAL::FUnixSpinLockMutex::Lock () noexcept
{
    UInt32 SpinCount = 0;

    /** INFO: On test_and_set failure, spin on a relaxed load until the flag looks free */
    while ( Flag.test_and_set( std::memory_order::acquire ) )
    {
        while ( Flag.test( std::memory_order::relaxed ) )
        {
            if ( SpinCount < GMaxRelaxSpins )
            {
                CpuRelax();
                ++SpinCount;
            }
            else
            {
                static_cast<void>( sched_yield() );
            }
        }
    }
}

void Lumina::HAL::FUnixSpinLockMutex::Unlock () noexcept
{
    Flag.clear( std::memory_order::release );
}

Lumina::Bool Lumina::HAL::FUnixSpinLockMutex::TryLock () noexcept
{
    return not Flag.test_and_set( std::memory_order::acquire );
}

Lumina::Bool Lumina::HAL::FUnixSpinLockMutex::IsLocked () const noexcept
{
    return Flag.test( std::memory_order::relaxed );
}

#endif
