/**
 * @file RHIDevice.hpp
 * @brief Direct OpenGL device and resource factory.
 */

#pragma once

#include <Core/NonCopyable.hpp>

#include <Filesystem/Path.hpp>
#include <Thread/Thread.hpp>

#include <RHI/RHIBuffer.hpp>
#include <RHI/RHICommandList.hpp>
#include <RHI/RHIFence.hpp>
#include <RHI/RHIFramebuffer.hpp>
#include <RHI/RHIPipeline.hpp>
#include <RHI/RHITexture.hpp>
#include <RHI/RHIWindow.hpp>
#include <RHIExport.hpp>

namespace Lumina
{

namespace RHI
{

    /**
     * @class FRHIDevice
     * @brief Represents a Direct OpenGL device and resource factory.
     */
    class RHI_EXPORT FRHIDevice final : public FNonCopyable
    {
    public:

        FRHIDevice () noexcept = default;
        FRHIDevice ( FRHIDevice &&InOther ) noexcept;
        FRHIDevice &operator=( FRHIDevice &&InOther ) noexcept;
        ~FRHIDevice () noexcept = default;

    public:

        /**
         * @brief Creates a new RHI device.
         *
         * NOTE: The caller must keep InWindow alive for the entire lifetime of the returned device. The device stores a
         * non-owning pointer to it and dereferences it in IsValid() and CheckThread().
         */
        [[nodiscard]] static TExpected<FRHIDevice, FRHIError> Create ( FRHIWindow &InWindow );

    public:

        /** @brief Creates a new buffer. */
        [[nodiscard]] TExpected<FRHIBuffer, FRHIError> CreateBuffer ( const FRHIBufferDesc &InDesc,
                                                                      TSpan<const Byte> InInitialData = {} ) const noexcept;

        /** @brief Creates a new texture. */
        [[nodiscard]] TExpected<FRHITexture, FRHIError> CreateTexture ( const FRHITextureDesc &InDesc ) const noexcept;

        /** @brief Creates a new sampler. */
        [[nodiscard]] TExpected<FRHISampler, FRHIError> CreateSampler ( const FRHISamplerDesc &InDesc ) const noexcept;

        /** @brief Creates a new framebuffer. */
        [[nodiscard]] TExpected<FRHIFramebuffer, FRHIError>
        CreateFramebuffer ( const FRHIFramebufferDesc &InDesc ) const noexcept;

        /** @brief Creates a new shader. */
        [[nodiscard]] TExpected<FRHIShader, FRHIError>
        CreateShader ( ERHIShaderStage InStage, FStringView InSource, FStringView InDebugName = {} ) const noexcept;

        /** @brief Creates a new shader from a file. */
        [[nodiscard]] TExpected<FRHIShader, FRHIError> CreateShaderFromFile ( ERHIShaderStage InStage,
                                                                              const Filesystem::FPath &InPath ) const noexcept;
        /** @brief Creates a new graphics pipeline. */
        [[nodiscard]] TExpected<FRHIGraphicsPipeline, FRHIError>
        CreateGraphicsPipeline ( const FRHIGraphicsPipelineDesc &InDesc ) const noexcept;

        /** @brief Creates a new compute pipeline. */
        [[nodiscard]] TExpected<FRHIComputePipeline, FRHIError>
        CreateComputePipeline ( const FRHIComputePipelineDesc &InDesc ) const noexcept;

        /** @brief Creates a new fence. */
        [[nodiscard]] TExpected<FRHIFence, FRHIError> CreateFence () const noexcept;

        /** @brief Creates a new command list. */
        [[nodiscard]] FRHICommandList CreateCommandList () const noexcept;

    public:

        /** @return the device capabilities. */
        [[nodiscard]] const FRHICapabilities &GetCapabilities () const noexcept;

        /** @return whether the device is valid. */
        [[nodiscard]] Bool IsValid () const noexcept;

        void CheckThread () const noexcept;

    private:

        explicit FRHIDevice ( FRHIWindow &InWindow ) noexcept;

        /** NOTE: Non-owning; the caller must keep the window alive for the device's lifetime (see Create). */
        FRHIWindow *Window = nullptr;
        FRHICapabilities Capabilities;
        FThread::FId OwnerThread;
    };

} // namespace RHI

} // namespace Lumina
