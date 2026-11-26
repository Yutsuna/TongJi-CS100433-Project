/**
 * @file RHIBuffer.hpp
 * @brief Move-only OpenGL buffer resource.
 */

#pragma once

#include <Core/NonCopyable.hpp>
#include <Memory/Span.hpp>
#include <Templates/Expected.hpp>

#include <RHI/RHITypes.hpp>
#include <RHIExport.hpp>

namespace Lumina
{

namespace RHI
{

    /**
     * @class FRHIBuffer
     * @brief Move-only OpenGL buffer resource.
     */
    class RHI_EXPORT FRHIBuffer final : public FNonCopyable
    {
    public:

        FRHIBuffer () noexcept = default;
        FRHIBuffer ( FRHIBuffer &&InOther ) noexcept;
        FRHIBuffer &operator=( FRHIBuffer &&InOther ) noexcept;
        ~FRHIBuffer () noexcept;

    public:

        [[nodiscard]] FRHIExpected Write ( USize InOffset, TSpan<const Byte> InData ) const noexcept;
        [[nodiscard]] FRHIExpected Read ( USize InOffset, TSpan<Byte> OutData ) const noexcept;
        [[nodiscard]] TExpected<TSpan<Byte>, FRHIError> Map ( USize InOffset, USize InSize, ERHIMapAccess InAccess ) noexcept;

        [[nodiscard]] Bool IsValid () const noexcept;
        [[nodiscard]] USize GetSize () const noexcept;
        [[nodiscard]] ERHIBufferUsage GetUsage () const noexcept;
        [[nodiscard]] FRHIExpected
        CopyTo ( FRHIBuffer &InDestination, USize InSourceOffset, USize InDestinationOffset, USize InSize ) const noexcept;

    public:

        void Unmap () noexcept;
        void Reset () noexcept;

    private:

        FRHIBuffer ( UInt32 InId, USize InSize, ERHIBufferUsage InUsage, Bool bInPersistent, Bool bInCoherent ) noexcept;

    private:

        UInt32 Id             = 0U;
        USize Size            = 0U;
        ERHIBufferUsage Usage = ERHIBufferUsage::None;
        void *MappedMemory    = nullptr;
        Bool bPersistent      = false;
        Bool bCoherent        = false;

        friend class FRHIDevice;
        friend class FRHICommandList;
    };

} // namespace RHI

} // namespace Lumina
