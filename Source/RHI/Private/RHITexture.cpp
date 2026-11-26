/**
 * @file RHITexture.cpp
 * @brief Implementation of the RHITexture class.
 */

#include <RHI/RHITexture.hpp>

#include "RHIPrivate.hpp"

/**
 * Public
 */

Lumina::RHI::FRHITexture::FRHITexture ( const UInt32 InId,
                                        const Maths::FVec2u InSize,
                                        const ERHITextureFormat InFormat,
                                        const UInt32 InMipLevels ) noexcept
    : Id( InId ), Size( InSize ), Format( InFormat ), MipLevels( InMipLevels )
{
    /* */
}

Lumina::RHI::FRHITexture::FRHITexture ( FRHITexture &&InOther ) noexcept
    : Id( std::exchange( InOther.Id, 0U ) ), Size( InOther.Size ), Format( InOther.Format ),
      MipLevels( std::exchange( InOther.MipLevels, 0U ) )
{
    /* */
}

Lumina::RHI::FRHITexture &Lumina::RHI::FRHITexture::operator=( FRHITexture &&InOther ) noexcept
{
    if ( this != &InOther )
    {
        Reset();
        Id        = std::exchange( InOther.Id, 0U );
        Size      = InOther.Size;
        Format    = InOther.Format;
        MipLevels = std::exchange( InOther.MipLevels, 0U );
    }
    return *this;
}

Lumina::RHI::FRHITexture::~FRHITexture () noexcept
{
    Reset();
}

Lumina::TExpected<void, Lumina::RHI::FRHIError> Lumina::RHI::FRHITexture::Write ( const TSpan<const Byte> InData,
                                                                                  const UInt32 InMipLevel ) const noexcept
{
    if ( not IsValid() or InMipLevel >= MipLevels ) [[unlikely]]
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::InvalidArgument, "Invalid texture or mip level" ) );
    }

    const USize RequiredSize = GetMipByteSize( InMipLevel );
    if ( InData.size() < RequiredSize ) [[unlikely]]
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::InvalidArgument, "Texture upload buffer is too small" ) );
    }

    const Maths::FVec2u MipSize = GetMipSize( InMipLevel );
    Internal::TextureSubImage2D( Id, static_cast<GLint>( InMipLevel ), 0, 0, static_cast<GLsizei>( MipSize.X ),
                                 static_cast<GLsizei>( MipSize.Y ), Internal::ToGLFormat( Format ), Internal::ToGLType( Format ),
                                 InData.data() );
    return {};
}

Lumina::RHI::FRHIExpected Lumina::RHI::FRHITexture::Read ( const TSpan<Byte> OutData, const UInt32 InMipLevel ) const noexcept
{
    if ( not IsValid() or InMipLevel >= MipLevels ) [[unlikely]]
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::InvalidArgument, "Invalid texture or mip level" ) );
    }

    const USize RequiredSize = GetMipByteSize( InMipLevel );
    if ( OutData.size() < RequiredSize ) [[unlikely]]
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::InvalidArgument, "Texture readback buffer is too small" ) );
    }

    const Maths::FVec2u MipSize = GetMipSize( InMipLevel );
    Internal::GetTextureSubImage( Id, static_cast<GLint>( InMipLevel ), 0, 0, 0, static_cast<GLsizei>( MipSize.X ),
                                  static_cast<GLsizei>( MipSize.Y ), 1, Internal::ToGLFormat( Format ),
                                  Internal::ToGLType( Format ), static_cast<GLsizei>( OutData.size() ), OutData.data() );
    return {};
}

void Lumina::RHI::FRHITexture::GenerateMipmaps () noexcept
{
    LUMINA_ASSERT( IsValid() );
    Internal::GenerateTextureMipmap( Id );
}

Lumina::Bool Lumina::RHI::FRHITexture::IsValid () const noexcept
{
    return Id != 0U;
}

Lumina::Maths::FVec2u Lumina::RHI::FRHITexture::GetSize () const noexcept
{
    return Size;
}

Lumina::Maths::FVec2u Lumina::RHI::FRHITexture::GetMipSize ( const UInt32 InMipLevel ) const noexcept
{
    if ( InMipLevel >= 32U )
    {
        return { 1U };
    }
    return { ( Size.X >> InMipLevel ) == 0U ? 1U : Size.X >> InMipLevel,
             ( Size.Y >> InMipLevel ) == 0U ? 1U : Size.Y >> InMipLevel };
}

Lumina::USize Lumina::RHI::FRHITexture::GetMipByteSize ( const UInt32 InMipLevel ) const noexcept
{
    if ( InMipLevel >= MipLevels )
    {
        return 0U;
    }
    const Maths::FVec2u MipSize = GetMipSize( InMipLevel );
    return static_cast<USize>( MipSize.X ) * static_cast<USize>( MipSize.Y ) * RHIFormatBytesPerPixel( Format );
}

Lumina::RHI::ERHITextureFormat Lumina::RHI::FRHITexture::GetFormat () const noexcept
{
    return Format;
}

Lumina::UInt32 Lumina::RHI::FRHITexture::GetMipLevels () const noexcept
{
    return MipLevels;
}

void Lumina::RHI::FRHITexture::Reset () noexcept
{
    if ( Id != 0U )
    {
        Internal::DeleteTextures( 1, &Id );
    }
    Id        = 0U;
    Size      = { 0U };
    MipLevels = 0U;
}

/**
 * FRHISampler
 */

Lumina::RHI::FRHISampler::FRHISampler ( const UInt32 InId ) noexcept : Id( InId )
{
    /* */
}

Lumina::RHI::FRHISampler::FRHISampler ( FRHISampler &&InOther ) noexcept : Id( std::exchange( InOther.Id, 0U ) )
{
    /* */
}

Lumina::RHI::FRHISampler &Lumina::RHI::FRHISampler::operator=( FRHISampler &&InOther ) noexcept
{
    if ( this != &InOther )
    {
        Reset();
        Id = std::exchange( InOther.Id, 0U );
    }
    return *this;
}

Lumina::RHI::FRHISampler::~FRHISampler () noexcept
{
    Reset();
}

Lumina::Bool Lumina::RHI::FRHISampler::IsValid () const noexcept
{
    return Id != 0U;
}

void Lumina::RHI::FRHISampler::Reset () noexcept
{
    if ( Id != 0U )
    {
        Internal::DeleteSamplers( 1, &Id );
    }
    Id = 0U;
}
