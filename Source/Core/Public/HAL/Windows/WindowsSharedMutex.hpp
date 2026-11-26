/**
 * @file WindowsSharedMutex.hpp
 * @brief Readers-writer mutex backed by a Win32 SRWLOCK.
 */

#pragma once

#include <Core/CoreDefinitions.hpp>

#if not defined( LUMINA_PLATFORM_WINDOWS )
    #if not defined( LUMINA_CLANG_TIDY )
        #error "WindowsSharedMutex.hpp is only supported on Windows"
    #endif
#else

    #include <Core/CoreTypes.hpp>
    #include <Core/NonCopyable.hpp>
    #include <Core/NonMovable.hpp>

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

namespace Lumina
{

namespace HAL
{

    /**
     * @class FWindowsSharedMutex
     * @brief Non-recursive readers-writer mutex.
     * @note  Not recursive: never take the shared lock twice on one thread.
     */
    class CORE_EXPORT FWindowsSharedMutex final : public FNonCopyable, public FNonMovable
    {
    public:

        FWindowsSharedMutex () noexcept  = default;
        ~FWindowsSharedMutex () noexcept = default;

    public:

        LUMINA_FORCE_INLINE void Lock () noexcept;
        LUMINA_FORCE_INLINE void Unlock () noexcept;
        LUMINA_FORCE_INLINE void LockShared () noexcept;
        LUMINA_FORCE_INLINE void UnlockShared () noexcept;

        [[nodiscard]] LUMINA_FORCE_INLINE Bool TryLock () noexcept;
        [[nodiscard]] LUMINA_FORCE_INLINE Bool TryLockShared () noexcept;

    private:

        SRWLOCK Mutex = SRWLOCK_INIT;
    };

} // namespace HAL

} // namespace Lumina

    #include "Inline/WindowsSharedMutex.inl"

#endif
