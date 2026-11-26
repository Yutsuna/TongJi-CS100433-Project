/**
 * @file RHIPrivate.cpp
 * @brief Private RHI implementation details.
 */

#include "RHIPrivate.hpp"

/**
 * Logger
 */

const Lumina::FLogCategory Lumina::RHI::Internal::LogRHI( FStringView( "RHI" ) );

/**
 * Public
 */

Lumina::RHI::FRHIError Lumina::RHI::Internal::MakeError ( const ERHIErrorCode InCode, FString InMessage )
{
    return { .Code = InCode, .Message = std::move( InMessage ) };
}

GLenum Lumina::RHI::Internal::ToGLInternalFormat ( const ERHITextureFormat InFormat ) noexcept
{
    switch ( InFormat )
    {
    case ERHITextureFormat::RGBA8:
        return GL_RGBA8;
    case ERHITextureFormat::RGBA16F:
        return GL_RGBA16F;
    case ERHITextureFormat::RGBA32F:
        return GL_RGBA32F;
    case ERHITextureFormat::R32F:
        return GL_R32F;
    case ERHITextureFormat::R32UI:
        return GL_R32UI;
    case ERHITextureFormat::Depth24Stencil8:
        return GL_DEPTH24_STENCIL8;
    }
    return GL_NONE;
}

GLenum Lumina::RHI::Internal::ToGLFormat ( const ERHITextureFormat InFormat ) noexcept
{
    switch ( InFormat )
    {
    case ERHITextureFormat::RGBA8:
    case ERHITextureFormat::RGBA16F:
    case ERHITextureFormat::RGBA32F:
        return GL_RGBA;
    case ERHITextureFormat::R32F:
        return GL_RED;
    case ERHITextureFormat::R32UI:
        return GL_RED_INTEGER;
    case ERHITextureFormat::Depth24Stencil8:
        return GL_DEPTH_STENCIL;
    }
    return GL_NONE;
}

GLenum Lumina::RHI::Internal::ToGLType ( const ERHITextureFormat InFormat ) noexcept
{
    switch ( InFormat )
    {
    case ERHITextureFormat::RGBA8:
        return GL_UNSIGNED_BYTE;
    case ERHITextureFormat::RGBA16F:
        return GL_HALF_FLOAT;
    case ERHITextureFormat::RGBA32F:
    case ERHITextureFormat::R32F:
        return GL_FLOAT;
    case ERHITextureFormat::R32UI:
        return GL_UNSIGNED_INT;
    case ERHITextureFormat::Depth24Stencil8:
        return GL_UNSIGNED_INT_24_8;
    }
    return GL_NONE;
}

GLenum Lumina::RHI::Internal::ToGLFilter ( const ERHITextureFilter InFilter, const Bool bMipmapped ) noexcept
{
    if ( InFilter == ERHITextureFilter::Nearest )
    {
        return bMipmapped ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
    }
    return bMipmapped ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
}

GLenum Lumina::RHI::Internal::ToGLAddressMode ( const ERHIAddressMode InMode ) noexcept
{
    return InMode == ERHIAddressMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
}

GLenum Lumina::RHI::Internal::ToGLShaderStage ( const ERHIShaderStage InStage ) noexcept
{
    switch ( InStage )
    {
    case ERHIShaderStage::Vertex:
        return GL_VERTEX_SHADER;
    case ERHIShaderStage::Fragment:
        return GL_FRAGMENT_SHADER;
    case ERHIShaderStage::Compute:
        return GL_COMPUTE_SHADER;
    }
    return GL_NONE;
}

GLenum Lumina::RHI::Internal::ToGLImageAccess ( const ERHIImageAccess InAccess ) noexcept
{
    switch ( InAccess )
    {
    case ERHIImageAccess::ReadOnly:
        return GL_READ_ONLY;
    case ERHIImageAccess::WriteOnly:
        return GL_WRITE_ONLY;
    case ERHIImageAccess::ReadWrite:
        return GL_READ_WRITE;
    }
    return GL_READ_WRITE;
}

GLbitfield Lumina::RHI::Internal::ToGLMemoryBarrier ( const ERHIMemoryBarrier InBarrier ) noexcept
{
    if ( InBarrier == ERHIMemoryBarrier::All )
    {
        return GL_ALL_BARRIER_BITS;
    }

    GLbitfield Result = 0U;
    if ( HasAnyFlags( InBarrier, ERHIMemoryBarrier::ShaderStorage ) )
    {
        Result |= GL_SHADER_STORAGE_BARRIER_BIT;
    }
    if ( HasAnyFlags( InBarrier, ERHIMemoryBarrier::ImageAccess ) )
    {
        Result |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
    }
    if ( HasAnyFlags( InBarrier, ERHIMemoryBarrier::TextureFetch ) )
    {
        Result |= GL_TEXTURE_FETCH_BARRIER_BIT;
    }
    if ( HasAnyFlags( InBarrier, ERHIMemoryBarrier::Framebuffer ) )
    {
        Result |= GL_FRAMEBUFFER_BARRIER_BIT;
    }
    return Result;
}

void Lumina::RHI::Internal::LabelObject ( const GLenum InIdentifier, const UInt32 InObject, const FStringView InName ) noexcept
{
    if ( not InName.empty() )
    {
        ObjectLabel( InIdentifier, InObject, static_cast<GLsizei>( InName.size() ), InName.data() );
    }
}
