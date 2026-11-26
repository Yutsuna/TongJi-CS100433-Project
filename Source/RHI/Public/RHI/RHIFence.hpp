/**
 * @file RHIFence.hpp
 * @brief Move-only OpenGL synchronization fence.
 */

#pragma once

#include <Core/NonCopyable.hpp>
#include <Templates/Expected.hpp>

#include <RHI/RHITypes.hpp>
#include <RHIExport.hpp>

namespace Lumina
{

namespace RHI
{

    /**
     * @class FRHIFence
     * @brief Move-only OpenGL synchronization fence.
     */
    class RHI_EXPORT FRHIFence final : public FNonCopyable
    {
    public:

        FRHIFence () noexcept = default;
        FRHIFence ( FRHIFence &&InOther ) noexcept;
        FRHIFence &operator=( FRHIFence &&InOther ) noexcept;
        ~FRHIFence () noexcept;

    public:

        [[nodiscard]] TExpected<ERHIFenceStatus, FRHIError> Wait ( UInt64 InTimeoutNanoseconds = 0U ) const noexcept;
        [[nodiscard]] Bool IsValid () const noexcept;

        void Reset () noexcept;

    private:

        explicit FRHIFence ( void *InHandle ) noexcept;

        void *Handle = nullptr;

        friend class FRHIDevice;
    };

} // namespace RHI

} // namespace Lumina
