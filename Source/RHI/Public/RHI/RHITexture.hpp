/**
 * @file RHITexture.hpp
 * @brief Move-only OpenGL 2D texture and sampler resources.
 */

#pragma once

#include <Core/NonCopyable.hpp>

#include <Maths/Vec.hpp>

#include <RHI/RHITypes.hpp>
#include <RHIExport.hpp>

#include <Templates/Expected.hpp>

namespace Lumina
{

namespace RHI
{

    /**
     * @class FRHITexture
     * @brief Move-only OpenGL 2D texture and sampler resources.
     */
    class RHI_EXPORT FRHITexture final : public FNonCopyable
    {
    public:

        FRHITexture () noexcept = default;
        FRHITexture ( FRHITexture &&InOther ) noexcept;
        FRHITexture &operator=( FRHITexture &&InOther ) noexcept;
        ~FRHITexture () noexcept;

    public:

        [[nodiscard]] FRHIExpected Write ( TSpan<const Byte> InData, UInt32 InMipLevel = 0U ) const noexcept;
        [[nodiscard]] FRHIExpected Read ( TSpan<Byte> OutData, UInt32 InMipLevel = 0U ) const noexcept;

    public:

        void GenerateMipmaps () noexcept;
        void Reset () noexcept;

    public:

        [[nodiscard]] Bool IsValid () const noexcept;
        [[nodiscard]] Maths::FVec2u GetSize () const noexcept;
        [[nodiscard]] Maths::FVec2u GetMipSize ( UInt32 InMipLevel ) const noexcept;
        [[nodiscard]] USize GetMipByteSize ( UInt32 InMipLevel = 0U ) const noexcept;
        [[nodiscard]] ERHITextureFormat GetFormat () const noexcept;
        [[nodiscard]] UInt32 GetMipLevels () const noexcept;

    private:

        FRHITexture ( UInt32 InId, Maths::FVec2u InSize, ERHITextureFormat InFormat, UInt32 InMipLevels ) noexcept;

        UInt32 Id                = 0U;
        Maths::FVec2u Size       = Maths::FVec2u( 0U );
        ERHITextureFormat Format = ERHITextureFormat::RGBA8;
        UInt32 MipLevels         = 0U;

        friend class FRHIDevice;
        friend class FRHICommandList;
        friend class FRHIFramebuffer;
    };

    /**
     * @class FRHISampler
     * @brief Move-only OpenGL sampler resources.
     */
    class RHI_EXPORT FRHISampler final : public FNonCopyable
    {
    public:

        FRHISampler () noexcept = default;
        FRHISampler ( FRHISampler &&InOther ) noexcept;
        FRHISampler &operator=( FRHISampler &&InOther ) noexcept;
        ~FRHISampler () noexcept;

    public:

        [[nodiscard]] Bool IsValid () const noexcept;
        void Reset () noexcept;

    private:

        explicit FRHISampler ( UInt32 InId ) noexcept;

        UInt32 Id = 0U;

        friend class FRHIDevice;
        friend class FRHICommandList;
    };

} // namespace RHI

} // namespace Lumina
