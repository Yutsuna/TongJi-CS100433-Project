/**
 * @file RHITypes.inl
 * @brief Inline implementation of RHI types.
 */

#pragma once

#include <RHI/RHITypes.hpp>

/**
 * Public
 */

constexpr Lumina::USize Lumina::RHI::RHIFormatBytesPerPixel ( const ERHITextureFormat InFormat ) noexcept
{
    switch ( InFormat )
    {
    case ERHITextureFormat::RGBA8:
    case ERHITextureFormat::R32UI:
    case ERHITextureFormat::Depth24Stencil8:
        return 4U;
    case ERHITextureFormat::RGBA16F:
        return 8U;
    case ERHITextureFormat::RGBA32F:
        return 16U;
    case ERHITextureFormat::R32F:
        return 4U;
    }
    return 0U;
}
