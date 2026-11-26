/**
 * @file UnixRecursiveSharedMutex.hpp
 * @brief Unix platforms recursive mutex definitions
 */

#pragma once

#include <Core/CoreDefinitions.hpp>

#if not defined( LUMINA_PLATFORM_UNIX )
    #if not defined( LUMINA_CLANG_TIDY )
        #error "UnixRecursiveSharedMutex.hpp is only available on Unix platforms."
    #endif
#else

    #include <Core/CoreTypes.hpp>
    #include <Core/NonCopyable.hpp>

    #include <pthread.h>

namespace Lumina
{

namespace HAL
{

    /**
     * @class FUnixRecursiveSharedMutex
     * @brief Recursive OS-parking mutex.
     */
    class CORE_EXPORT FUnixRecursiveSharedMutex final : public FNonCopyable
    {
    public:

        LUMINA_FORCE_INLINE FUnixRecursiveSharedMutex () noexcept;
        LUMINA_FORCE_INLINE ~FUnixRecursiveSharedMutex() noexcept;

    public:

        /**
         * @brief Tries to acquire the mutex.
         * @return True if the mutex was acquired, false otherwise.
         */
        LUMINA_FORCE_INLINE Bool TryLock () noexcept;

        /**
         * @brief Acquires the mutex.
         */
        LUMINA_FORCE_INLINE void Lock () noexcept;

        /**
         * @brief Releases the mutex.
         */
        LUMINA_FORCE_INLINE void Unlock () noexcept;

    private:

        /** @brief Spin rounds before blocking. Round N issues 2^n relax hints ( 5 rounds ~ 62 hints). */
        static constexpr UInt32 GSpinRounds = 5U;

        pthread_mutex_t Mutex;
    };

} // namespace HAL

} // namespace Lumina

    #include "Inline/UnixRecursiveSharedMutex.inl"

#endif
