/**
 * @file UnixSpinLockMutex.hpp
 * @brief Busy-wait spin lock for POSIX UNIX platforms.
 */

#pragma once

#include <Core/CoreDefinitions.hpp>

#if not defined( LUMINA_PLATFORM_UNIX )
    #if not defined( LUMINA_CLANG_TIDY )
        #error "FUnixSpinLockMutex.hpp is only available on Unix platforms."
    #endif
#else

    #include <Core/CoreTypes.hpp>
    #include <Core/NonCopyable.hpp>
    #include <Core/NonMovable.hpp>

namespace Lumina
{

namespace HAL
{

    class CORE_EXPORT FUnixSpinLockMutex final : public FNonCopyable, public FNonMovable
    {
    public:

        FUnixSpinLockMutex () noexcept  = default;
        ~FUnixSpinLockMutex () noexcept = default;

    public:

        LUMINA_FORCE_INLINE void Lock () noexcept;
        LUMINA_FORCE_INLINE void Unlock () noexcept;

        [[nodiscard]] LUMINA_FORCE_INLINE Bool TryLock () noexcept;
        [[nodiscard]] LUMINA_FORCE_INLINE Bool IsLocked () const noexcept;

    private:

        /** @brief Number of CpuRelax() iterations before the waiter starts yielding its time slice. */
        static constexpr UInt32 GMaxRelaxSpins = 64U;

        FAtomicFlag Flag;
    };

} // namespace HAL

} // namespace Lumina

    #include "Inline/UnixSpinLockMutex.inl"

#endif
