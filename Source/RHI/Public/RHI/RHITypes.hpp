/**
 * @file RHITypes.hpp
 * @brief Public values and descriptors for the OpenGL RHI.
 */

#pragma once

#include <Core/CoreTypes.hpp>
#include <Core/EnumFlags.hpp>
#include <Templates/Expected.hpp>

#include <Maths/Vec.hpp>

#include <Memory/Span.hpp>
#include <String/String.hpp>
#include <String/StringView.hpp>

namespace Lumina
{

namespace RHI
{

    /**
     * @enum ERHIErrorCode
     * @brief Error codes for the RHI.
     */
    enum class ERHIErrorCode : UInt8
    {
        InvalidArgument,
        InitializationFailed,
        Unsupported,
        ShaderCompilationFailed,
        ProgramLinkFailed,
        IoError,
        Timeout,
        BackendError,
        DeviceLost,
    };

    /**
     * @struct FRHIError
     * @brief Represents an error that occurred during RHI operations.
     */
    struct FRHIError
    {
        ERHIErrorCode Code = ERHIErrorCode::InitializationFailed;
        FString Message;
    };

    using FRHIExpected = TExpected<void, FRHIError>;

    /**
     * @enum ERHIBufferUsage
     * @brief Usage flags for RHI buffers.
     */
    enum class ERHIBufferUsage : UInt32
    {
        None     = 0,
        Vertex   = 1U << 0U,
        Index    = 1U << 1U,
        Uniform  = 1U << 2U,
        Storage  = 1U << 3U,
        Dynamic  = 1U << 4U,
        Readback = 1U << 5U,
    };
    LUMINA_ENUM_FLAGS( ERHIBufferUsage )

    /**
     * @enum ERHIMapAccess
     * @brief Access flags for RHI buffer mapping.
     */
    enum class ERHIMapAccess : UInt8
    {
        Read,
        Write,
        ReadWrite,
    };

    /**
     * @struct FRHIBufferDesc
     * @brief Describes a buffer for RHI operations.
     */
    struct FRHIBufferDesc
    {
        USize Size            = 0;
        ERHIBufferUsage Usage = ERHIBufferUsage::None;
        Bool bPersistent      = false;
        Bool bCoherent        = false;
        FStringView DebugName;
    };

    /**
     * @enum ERHITextureFormat
     * @brief Texture formats for RHI textures.
     */
    enum class ERHITextureFormat : UInt8
    {
        RGBA8,
        RGBA16F,
        RGBA32F,
        R32F,
        R32UI,
        Depth24Stencil8,
    };

    /**
     * @enum ERHITextureUsage
     * @brief Usage flags for RHI textures.
     */
    enum class ERHITextureUsage : UInt32
    {
        None            = 0,
        Sampled         = 1U << 0U,
        Storage         = 1U << 1U,
        ColorAttachment = 1U << 2U,
        DepthAttachment = 1U << 3U,
        Transfer        = 1U << 4U,
    };
    LUMINA_ENUM_FLAGS( ERHITextureUsage )

    /**
     * @struct FRHITextureDesc
     * @brief Describes a texture for RHI operations.
     */
    struct FRHITextureDesc
    {
        Maths::FVec2u Size       = Maths::FVec2u( 1U, 1U );
        ERHITextureFormat Format = ERHITextureFormat::RGBA8;
        ERHITextureUsage Usage   = ERHITextureUsage::Sampled;
        UInt32 MipLevels         = 1U;
        FStringView DebugName;
    };

    /**
     * @enum ERHITextureFilter
     * @brief Texture filtering modes for RHI textures.
     */
    enum class ERHITextureFilter : UInt8
    {
        Nearest,
        Linear,
    };

    /**
     * @enum ERHIAddressMode
     * @brief Address modes for RHI texture sampling.
     */
    enum class ERHIAddressMode : UInt8
    {
        Repeat,
        ClampToEdge,
    };

    /**
     * @struct FRHISamplerDesc
     * @brief Describes a sampler for RHI operations.
     */
    struct FRHISamplerDesc
    {
        ERHITextureFilter MinFilter = ERHITextureFilter::Linear;
        ERHITextureFilter MagFilter = ERHITextureFilter::Linear;
        ERHIAddressMode AddressU    = ERHIAddressMode::ClampToEdge;
        ERHIAddressMode AddressV    = ERHIAddressMode::ClampToEdge;
        Bool bMipmapped             = false;
        FStringView DebugName;
    };

    /**
     * @enum ERHIShaderStage
     * @brief Shader stages for RHI operations.
     */
    enum class ERHIShaderStage : UInt8
    {
        Vertex,
        Fragment,
        Compute,
    };

    /**
     * @enum ERHIVertexFormat
     * @brief Vertex formats for RHI operations.
     */
    enum class ERHIVertexFormat : UInt8
    {
        Float,
        Float2,
        Float3,
        Float4,
        UInt,
        UInt2,
        UInt3,
        UInt4,
    };

    /**
     * @struct FRHIVertexBinding
     * @brief Describes a vertex binding for RHI operations.
     */
    struct FRHIVertexBinding
    {
        UInt32 Binding = 0U;
        UInt32 Stride  = 0U;
    };

    /**
     * @struct FRHIVertexAttribute
     * @brief Describes a vertex attribute for RHI operations.
     */
    struct FRHIVertexAttribute
    {
        UInt32 Location         = 0U;
        UInt32 Binding          = 0U;
        UInt32 Offset           = 0U;
        ERHIVertexFormat Format = ERHIVertexFormat::Float3;
    };

    /**
     * @enum ERHIPrimitiveTopology
     * @brief Primitive topology for RHI operations.
     */
    enum class ERHIPrimitiveTopology : UInt8
    {
        Points,
        Lines,
        Triangles,
    };

    /**
     * @enum ERHICullMode
     * @brief Cull mode for RHI operations.
     */
    enum class ERHICullMode : UInt8
    {
        None,
        Back,
        Front,
    };

    /**
     * @enum ERHIIndexType
     * @brief Index types for RHI operations.
     */
    enum class ERHIIndexType : UInt8
    {
        UInt16,
        UInt32,
    };

    /**
     * @enum ERHIImageAccess
     * @brief Image access modes for RHI operations.
     */
    enum class ERHIImageAccess : UInt8
    {
        ReadOnly,
        WriteOnly,
        ReadWrite,
    };

    /**
     * @enum ERHIMemoryBarrier
     * @brief Memory barrier types for RHI operations.
     */
    enum class ERHIMemoryBarrier : UInt32
    {
        None          = 0,
        ShaderStorage = 1U << 0U,
        ImageAccess   = 1U << 1U,
        TextureFetch  = 1U << 2U,
        Framebuffer   = 1U << 3U,
        All           = 0xFFFFFFFFU,
    };
    LUMINA_ENUM_FLAGS( ERHIMemoryBarrier )

    /**
     * @enum ERHIFenceStatus
     * @brief Fence status for RHI operations.
     */
    enum class ERHIFenceStatus : UInt8
    {
        Signaled,
        Timeout,
    };

    /**
     * @struct FRHICapabilities
     * @brief RHI capabilities for the current device.
     */
    struct FRHICapabilities
    {
        FString Vendor;
        FString Renderer;
        FString Version;
        UInt32 MaxTextureSize                  = 0U;
        UInt32 MaxCombinedTextureUnits         = 0U;
        UInt32 MaxImageUnits                   = 0U;
        UInt32 MaxStorageBufferBindings        = 0U;
        Maths::FVec3u MaxComputeWorkGroupCount = Maths::FVec3u( 0U );
        Maths::FVec3u MaxComputeWorkGroupSize  = Maths::FVec3u( 0U );
    };

    /**
     * @fn RHIFormatBytesPerPixel
     * @brief Returns the number of bytes per pixel for a given RHI texture format.
     */
    [[nodiscard]] constexpr USize RHIFormatBytesPerPixel ( const ERHITextureFormat InFormat ) noexcept;

} // namespace RHI

} // namespace Lumina

#include "Inline/RHITypes.inl"
