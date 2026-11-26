/**
 * @file RHICommandList.hpp
 * @brief Immediate OpenGL commands with redundant-state filtering.
 */

#pragma once

#include <Core/NonCopyable.hpp>

#include <RHI/RHIBuffer.hpp>
#include <RHI/RHIFramebuffer.hpp>
#include <RHI/RHIPipeline.hpp>
#include <RHIExport.hpp>

#include <Templates/Array.hpp>
#include <Templates/Expected.hpp>

namespace Lumina
{

namespace RHI
{

    class FRHIDevice;

    /**
     * @class FRHICommandList
     * @brief Represents a command list for the RHI.
     */
    class RHI_EXPORT FRHICommandList final : public FNonCopyable
    {
    public:

        FRHICommandList () noexcept = default;
        FRHICommandList ( FRHICommandList &&InOther ) noexcept;
        FRHICommandList &operator=( FRHICommandList &&InOther ) noexcept;
        ~FRHICommandList () noexcept = default;

    public:

        void BeginFrame () noexcept;
        void BindPipeline ( const FRHIGraphicsPipeline &InPipeline ) noexcept;
        void BindPipeline ( const FRHIComputePipeline &InPipeline ) noexcept;
        void BindVertexBuffer ( const FRHIBuffer &InBuffer, UInt32 InBinding = 0U, USize InOffset = 0U ) noexcept;
        void BindIndexBuffer ( const FRHIBuffer &InBuffer ) const noexcept;
        void BindUniformBuffer ( const FRHIBuffer &InBuffer, UInt32 InBinding ) noexcept;
        void BindStorageBuffer ( const FRHIBuffer &InBuffer, UInt32 InBinding ) noexcept;
        void BindTexture ( const FRHITexture &InTexture, UInt32 InSlot, const FRHISampler *InSampler = nullptr ) noexcept;
        void BindImage ( const FRHITexture &InTexture,
                         UInt32 InSlot,
                         ERHIImageAccess InAccess = ERHIImageAccess::ReadWrite,
                         UInt32 InMipLevel        = 0U ) noexcept;
        void BindFramebuffer ( const FRHIFramebuffer *InFramebuffer ) noexcept;

    public:

        void SetViewport ( Maths::FVec2u InSize ) noexcept;
        void ClearColor ( const Maths::FVec4f &InColor ) const noexcept;

        void Draw ( UInt32 InVertexCount, UInt32 InFirstVertex = 0U ) const noexcept;
        void DrawIndexed ( UInt32 InIndexCount, ERHIIndexType InIndexType, USize InByteOffset = 0U ) const noexcept;

        void MemoryBarrier ( ERHIMemoryBarrier InBarrier = ERHIMemoryBarrier::All ) const noexcept;

    public:

        [[nodiscard]] FRHIExpected Dispatch ( Maths::FVec3u InGroupCount ) const noexcept;
        [[nodiscard]] FRHIExpected
        Dispatch2D ( const FRHIComputePipeline &InPipeline, Maths::FVec2u InThreadCount, Maths::FVec2u InLocalSize ) noexcept;

    private:

        explicit FRHICommandList ( const FRHIDevice &InDevice ) noexcept;
        void ApplyGraphicsState ( const FRHIGraphicsState &InState ) noexcept;

    private:

        static constexpr USize TextureSlotCount = 32U;
        static constexpr USize ImageSlotCount   = 16U;
        static constexpr USize BufferSlotCount  = 16U;

    private:

        const FRHIDevice *Device                            = nullptr;
        UInt32 CurrentProgram                               = 0U;
        UInt32 CurrentVertexArray                           = 0U;
        UInt32 CurrentFramebuffer                           = 0U;
        const FRHIGraphicsPipeline *CurrentGraphicsPipeline = nullptr;
        Maths::FVec2u CurrentViewport                       = Maths::FVec2u( 0U );
        Bool bHasViewport                                   = false;
        Bool bHasGraphicsState                              = false;
        FRHIGraphicsState CurrentGraphicsState              = {};
        TArray<UInt32, TextureSlotCount> TextureSlots{};
        TArray<UInt32, ImageSlotCount> ImageSlots{};
        TArray<UInt32, ImageSlotCount> ImageMipLevels{};
        TArray<ERHIImageAccess, ImageSlotCount> ImageAccesses{};
        TArray<UInt32, TextureSlotCount> SamplerSlots{};
        TArray<UInt32, BufferSlotCount> UniformBufferSlots{};
        TArray<UInt32, BufferSlotCount> StorageBufferSlots{};

        friend class FRHIDevice;
    };

} // namespace RHI

} // namespace Lumina
