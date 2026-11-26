/**
 * @file UnixSharedMutex.hpp
 * @brief Readers-writer mutex backed by a POSIX pthread rwlock.
 */

#pragma once

#include <Core/CoreDefinitions.hpp>

#if not defined( LUMINA_PLATFORM_UNIX )
    #if not defined( LUMINA_CLANG_TIDY )
        #error "UnixSharedMutex.hpp is only available on Unix platforms."
    #endif
#else

    #include <Core/CoreTypes.hpp>
    #include <Core/NonCopyable.hpp>
    #include <Core/NonMovable.hpp>

    #include <pthread.h>

namespace Lumina
{

namespace HAL
{

    class CORE_EXPORT FUnixSharedMutex final : public FNonCopyable, public FNonMovable
    {
    public:

        FUnixSharedMutex () noexcept = default;
        LUMINA_FORCE_INLINE ~FUnixSharedMutex() noexcept;

        LUMINA_FORCE_INLINE void Lock () noexcept;
        LUMINA_FORCE_INLINE void Unlock () noexcept;

        LUMINA_FORCE_INLINE void LockShared () noexcept;
        LUMINA_FORCE_INLINE void UnlockShared () noexcept;

        [[nodiscard]] LUMINA_FORCE_INLINE Bool TryLock () noexcept;
        [[nodiscard]] LUMINA_FORCE_INLINE Bool TryLockShared () noexcept;

    private:

    #if defined( PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP )
        pthread_rwlock_t RwLock = PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP;
    #else
        pthread_rwlock_t RwLock = PTHREAD_RWLOCK_INITIALIZER;
    #endif
    };

} // namespace HAL

} // namespace Lumina

    #include "Inline/UnixSharedMutex.inl"

#endif
