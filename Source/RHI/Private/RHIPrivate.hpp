/**
 * @file RHIPrivate.hpp
 * @brief Private RHI implementation details.
 */

#pragma once

#include <Logging/Logger.hpp>

#include "RHIGL.hpp"
#include <RHI/RHITypes.hpp>

#include <Templates/Unexpected.hpp>

namespace Lumina
{

namespace RHI
{

    namespace Internal
    {

        extern const FLogCategory LogRHI;

        [[nodiscard]] FRHIError MakeError ( ERHIErrorCode InCode, FString InMessage );
        [[nodiscard]] GLenum ToGLInternalFormat ( ERHITextureFormat InFormat ) noexcept;
        [[nodiscard]] GLenum ToGLFormat ( ERHITextureFormat InFormat ) noexcept;
        [[nodiscard]] GLenum ToGLType ( ERHITextureFormat InFormat ) noexcept;
        [[nodiscard]] GLenum ToGLFilter ( ERHITextureFilter InFilter, Bool bMipmapped ) noexcept;
        [[nodiscard]] GLenum ToGLAddressMode ( ERHIAddressMode InMode ) noexcept;
        [[nodiscard]] GLenum ToGLShaderStage ( ERHIShaderStage InStage ) noexcept;
        [[nodiscard]] GLenum ToGLImageAccess ( ERHIImageAccess InAccess ) noexcept;
        [[nodiscard]] GLbitfield ToGLMemoryBarrier ( ERHIMemoryBarrier InBarrier ) noexcept;

        void LabelObject ( GLenum InIdentifier, UInt32 InObject, FStringView InName ) noexcept;

    } // namespace Internal

} // namespace RHI

} // namespace Lumina
