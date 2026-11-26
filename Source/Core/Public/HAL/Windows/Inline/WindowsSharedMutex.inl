/**
 * @file WindowsSharedMutex.inl
 * @brief Readers-writer mutex implementation.
 */

#pragma once

#include <HAL/Windows/WindowsSharedMutex.hpp>

#if not defined( LUMEN_PLATFORM_WINDOWS )
    #if not defined( LUMEN_CLANG_TIDY )
        #error "WindowsSharedMutex.inl is only supported on Windows"
    #endif
#else

/**
 * Public
 */

void Lumina::HAL::FWindowsSharedMutex::Lock () noexcept
{
    AcquireSRWLockExclusive( &Mutex );
}

void Lumina::HAL::FWindowsSharedMutex::Unlock () noexcept
{
    ReleaseSRWLockExclusive( &Mutex );
}

void Lumina::HAL::FWindowsSharedMutex::LockShared () noexcept
{
    AcquireSRWLockShared( &Mutex );
}

void Lumina::HAL::FWindowsSharedMutex::UnlockShared () noexcept
{
    ReleaseSRWLockShared( &Mutex );
}

Lumina::Bool Lumina::HAL::FWindowsSharedMutex::TryLock () noexcept
{
    return TryAcquireSRWLockExclusive( &Mutex ) != 0;
}

Lumina::Bool Lumina::HAL::FWindowsSharedMutex::TryLockShared () noexcept
{
    return TryAcquireSRWLockShared( &Mutex ) != 0;
}

#endif
