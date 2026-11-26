/**
 * @file RHIFramebuffer.hpp
 * @brief OpenGL framebuffer with texture attachments.
 */

#pragma once

#include <Core/NonCopyable.hpp>
#include <RHI/RHITexture.hpp>
#include <RHIExport.hpp>

namespace Lumina
{

namespace RHI
{

    /**
     * @struct FRHIFramebufferDesc
     * @brief Descriptor for a framebuffer, specifying color and depth attachments.
     */
    struct FRHIFramebufferDesc
    {
        TSpan<const FRHITexture *const> ColorAttachments;
        const FRHITexture *DepthAttachment = nullptr;
        FStringView DebugName;
    };

    /**
     * @class FRHIFramebuffer
     * @brief OpenGL framebuffer with texture attachments.
     */
    class RHI_EXPORT FRHIFramebuffer final : public FNonCopyable
    {
    public:

        FRHIFramebuffer () noexcept = default;
        FRHIFramebuffer ( FRHIFramebuffer &&InOther ) noexcept;
        FRHIFramebuffer &operator=( FRHIFramebuffer &&InOther ) noexcept;
        ~FRHIFramebuffer () noexcept;

    public:

        [[nodiscard]] Bool IsValid () const noexcept;
        [[nodiscard]] Maths::FVec2u GetSize () const noexcept;
        void Reset () noexcept;

    private:

        FRHIFramebuffer ( UInt32 InId, Maths::FVec2u InSize ) noexcept;

        UInt32 Id          = 0U;
        Maths::FVec2u Size = Maths::FVec2u( 0U );

        friend class FRHIDevice;
        friend class FRHICommandList;
    };

} // namespace RHI

} // namespace Lumina
