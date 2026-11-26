/**
 * @file UnixMutex.inl
 * @brief Inline implementation of Unix Mutex
 */

#pragma once

#if not defined( LUMINA_PLATFORM_UNIX )
    #if not defined( LUMINA_CLANG_TIDY )
        #error "UnixRecursiveMutex.hpp is only available on Unix platforms."
    #endif
#else

    #include <HAL/Unix/UnixMutex.hpp>

/**
 * Public
 */

Lumina::HAL::FUnixMutex::~FUnixMutex () noexcept
{
    if ( pthread_mutex_destroy( &Mutex ) != 0 ) [[unlikely]]
    {
        return;
    }
}

void Lumina::HAL::FUnixMutex::Lock () noexcept
{
    if ( pthread_mutex_lock( &Mutex ) != 0 ) [[unlikely]]
    {
        return;
    }
}

void Lumina::HAL::FUnixMutex::Unlock () noexcept
{
    if ( pthread_mutex_unlock( &Mutex ) != 0 ) [[unlikely]]
    {
        return;
    }
}

Lumina::Bool Lumina::HAL::FUnixMutex::TryLock () noexcept
{
    return pthread_mutex_trylock( &Mutex ) == 0;
}

#endif
