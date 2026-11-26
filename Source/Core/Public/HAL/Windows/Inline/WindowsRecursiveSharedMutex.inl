/**
 * @file WindowsRecursiveSharedMutex.inl
 * @brief Recursive mutex implementation.
 */

#pragma once

#include <HAL/Windows/WindowsRecursiveSharedMutex.hpp>

#if not defined( LUMEN_PLATFORM_WINDOWS )
    #if not defined( LUMEN_CLANG_TIDY )
        #error "WindowsRecursiveSharedMutex.inl is only supported on Windows"
    #endif
#else

/**
 * Public
 */

Lumina::HAL::FWindowsRecursiveSharedMutex::FWindowsRecursiveSharedMutex () noexcept
{
    static_cast<void>( InitializeCriticalSectionAndSpinCount( &Mutex, GSpinCount ) );
}

Lumina::HAL::FWindowsRecursiveSharedMutex::~FWindowsRecursiveSharedMutex () noexcept
{
    DeleteCriticalSection( &Mutex );
}

void Lumina::HAL::FWindowsRecursiveSharedMutex::Lock () noexcept
{
    EnterCriticalSection( &Mutex );
}

void Lumina::HAL::FWindowsRecursiveSharedMutex::Unlock () noexcept
{
    LeaveCriticalSection( &Mutex );
}

Lumina::Bool Lumina::HAL::FWindowsRecursiveSharedMutex::TryLock () noexcept
{
    return TryEnterCriticalSection( &Mutex ) != 0;
}

#endif
