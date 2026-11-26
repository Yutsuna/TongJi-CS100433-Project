/**
 * @file RHICommandList.cpp
 * @brief Implementation of the RHI command list.
 */

#include <RHI/RHICommandList.hpp>

#include "RHIPrivate.hpp"

#include <Algo/Find.hpp>
#include <RHI/RHIDevice.hpp>

/**
 * Private
 */

namespace
{

namespace LRHI                         = Lumina::RHI;
constexpr Lumina::UInt32 UnknownHandle = 0xFFFFFFFFU;

/** @brief Convert an RHI primitive topology to a GL topology. */
GLenum ToGLTopology ( const LRHI::ERHIPrimitiveTopology InTopology ) noexcept
{
    switch ( InTopology )
    {
    case LRHI::ERHIPrimitiveTopology::Points:
        return GL_POINTS;
    case LRHI::ERHIPrimitiveTopology::Lines:
        return GL_LINES;
    case LRHI::ERHIPrimitiveTopology::Triangles:
        return GL_TRIANGLES;
    }
    return GL_TRIANGLES;
}

/** @brief Enable or disable a GL capability. */
void SetCapability ( const GLenum InCapability, const Lumina::Bool bEnabled ) noexcept
{
    if ( bEnabled )
    {
        LRHI::Internal::Enable( InCapability );
    }
    else
    {
        LRHI::Internal::Disable( InCapability );
    }
}

} // namespace

/**
 * Public
 */

LRHI::FRHICommandList::FRHICommandList ( const FRHIDevice &InDevice ) noexcept : Device( &InDevice )
{
    BeginFrame();
}

LRHI::FRHICommandList::FRHICommandList ( FRHICommandList &&InOther ) noexcept
{
    *this = std::move( InOther );
}

LRHI::FRHICommandList &LRHI::FRHICommandList::operator=( FRHICommandList &&InOther ) noexcept
{
    if ( this != &InOther )
    {
        Device                  = std::exchange( InOther.Device, nullptr );
        CurrentProgram          = InOther.CurrentProgram;
        CurrentVertexArray      = InOther.CurrentVertexArray;
        CurrentFramebuffer      = InOther.CurrentFramebuffer;
        CurrentGraphicsPipeline = std::exchange( InOther.CurrentGraphicsPipeline, nullptr );
        CurrentViewport         = InOther.CurrentViewport;
        bHasViewport            = InOther.bHasViewport;
        bHasGraphicsState       = InOther.bHasGraphicsState;
        CurrentGraphicsState    = InOther.CurrentGraphicsState;
        TextureSlots            = InOther.TextureSlots;
        ImageSlots              = InOther.ImageSlots;
        ImageMipLevels          = InOther.ImageMipLevels;
        ImageAccesses           = InOther.ImageAccesses;
        SamplerSlots            = InOther.SamplerSlots;
        UniformBufferSlots      = InOther.UniformBufferSlots;
        StorageBufferSlots      = InOther.StorageBufferSlots;
    }
    return *this;
}

void LRHI::FRHICommandList::BeginFrame () noexcept
{
    LUMINA_ASSERT( Device != nullptr );
    Device->CheckThread();
    CurrentProgram          = UnknownHandle;
    CurrentVertexArray      = UnknownHandle;
    CurrentFramebuffer      = UnknownHandle;
    CurrentGraphicsPipeline = nullptr;
    bHasViewport            = false;
    bHasGraphicsState       = false;
    TextureSlots.fill( UnknownHandle );
    ImageSlots.fill( UnknownHandle );
    ImageMipLevels.fill( UnknownHandle );
    ImageAccesses.fill( ERHIImageAccess::ReadWrite );
    SamplerSlots.fill( UnknownHandle );
    UniformBufferSlots.fill( UnknownHandle );
    StorageBufferSlots.fill( UnknownHandle );
}

void LRHI::FRHICommandList::BindPipeline ( const FRHIGraphicsPipeline &InPipeline ) noexcept
{
    Device->CheckThread();
    LUMINA_ASSERT( InPipeline.IsValid() );
    if ( CurrentProgram != InPipeline.Program )
    {
        Internal::UseProgram( InPipeline.Program );
        CurrentProgram = InPipeline.Program;
    }
    if ( CurrentVertexArray != InPipeline.VertexArray )
    {
        Internal::BindVertexArray( InPipeline.VertexArray );
        CurrentVertexArray = InPipeline.VertexArray;
    }
    CurrentGraphicsPipeline = &InPipeline;
    ApplyGraphicsState( InPipeline.State );
}

void LRHI::FRHICommandList::BindPipeline ( const FRHIComputePipeline &InPipeline ) noexcept
{
    Device->CheckThread();
    LUMINA_ASSERT( InPipeline.IsValid() );
    if ( CurrentProgram != InPipeline.Program )
    {
        Internal::UseProgram( InPipeline.Program );
        CurrentProgram = InPipeline.Program;
    }
    CurrentGraphicsPipeline = nullptr;
}

void LRHI::FRHICommandList::BindVertexBuffer ( const FRHIBuffer &InBuffer, const UInt32 InBinding, const USize InOffset ) noexcept
{
    LUMINA_ASSERT( CurrentGraphicsPipeline != nullptr );
    LUMINA_ASSERT( HasAnyFlags( InBuffer.Usage, ERHIBufferUsage::Vertex ) );

    const auto *const Binding =
        Algo::FindIf( CurrentGraphicsPipeline->VertexBindings, [InBinding] ( const FRHIVertexBinding &InValue ) noexcept
    {
        return InValue.Binding == InBinding;
    } );

    LUMINA_ASSERT( Binding != nullptr );
    if ( Binding != nullptr )
    {
        Internal::VertexArrayVertexBuffer( CurrentVertexArray, InBinding, InBuffer.Id, static_cast<GLintptr>( InOffset ),
                                           static_cast<GLsizei>( Binding->Stride ) );
    }
}

void LRHI::FRHICommandList::BindIndexBuffer ( const FRHIBuffer &InBuffer ) const noexcept
{
    LUMINA_ASSERT( CurrentVertexArray != UnknownHandle and CurrentVertexArray != 0U );
    LUMINA_ASSERT( HasAnyFlags( InBuffer.Usage, ERHIBufferUsage::Index ) );

    Internal::VertexArrayElementBuffer( CurrentVertexArray, InBuffer.Id );
}

void LRHI::FRHICommandList::BindUniformBuffer ( const FRHIBuffer &InBuffer, const UInt32 InBinding ) noexcept
{
    LUMINA_ASSERT( InBinding < BufferSlotCount );
    LUMINA_ASSERT( HasAnyFlags( InBuffer.Usage, ERHIBufferUsage::Uniform ) );

    if ( UniformBufferSlots[InBinding] != InBuffer.Id )
    {
        Internal::BindBufferBase( GL_UNIFORM_BUFFER, InBinding, InBuffer.Id );
        UniformBufferSlots[InBinding] = InBuffer.Id;
    }
}

void LRHI::FRHICommandList::BindStorageBuffer ( const FRHIBuffer &InBuffer, const UInt32 InBinding ) noexcept
{
    Device->CheckThread();

    LUMINA_ASSERT( InBinding < BufferSlotCount );
    LUMINA_ASSERT( InBinding < Device->GetCapabilities().MaxStorageBufferBindings );
    LUMINA_ASSERT( HasAnyFlags( InBuffer.Usage, ERHIBufferUsage::Storage ) );

    if ( StorageBufferSlots[InBinding] != InBuffer.Id )
    {
        Internal::BindBufferBase( GL_SHADER_STORAGE_BUFFER, InBinding, InBuffer.Id );
        StorageBufferSlots[InBinding] = InBuffer.Id;
    }
}

void LRHI::FRHICommandList::BindTexture ( const FRHITexture &InTexture,
                                          const UInt32 InSlot,
                                          const FRHISampler *InSampler ) noexcept
{
    Device->CheckThread();

    LUMINA_ASSERT( InSlot < TextureSlotCount );
    LUMINA_ASSERT( InSlot < Device->GetCapabilities().MaxCombinedTextureUnits );
    LUMINA_ASSERT( InTexture.IsValid() );

    if ( TextureSlots[InSlot] != InTexture.Id )
    {
        Internal::BindTextureUnit( InSlot, InTexture.Id );
        TextureSlots[InSlot] = InTexture.Id;
    }

    const UInt32 SamplerId = InSampler == nullptr ? 0U : InSampler->Id;
    if ( SamplerSlots[InSlot] != SamplerId )
    {
        Internal::BindSampler( InSlot, SamplerId );
        SamplerSlots[InSlot] = SamplerId;
    }
}

void LRHI::FRHICommandList::BindImage ( const FRHITexture &InTexture,
                                        const UInt32 InSlot,
                                        const ERHIImageAccess InAccess,
                                        const UInt32 InMipLevel ) noexcept
{
    Device->CheckThread();

    LUMINA_ASSERT( InSlot < ImageSlotCount );
    LUMINA_ASSERT( InSlot < Device->GetCapabilities().MaxImageUnits );
    LUMINA_ASSERT( InTexture.IsValid() and InMipLevel < InTexture.MipLevels );

    if ( ImageSlots[InSlot] != InTexture.Id or ImageMipLevels[InSlot] != InMipLevel or ImageAccesses[InSlot] != InAccess )
    {
        Internal::BindImageTexture( InSlot, InTexture.Id, static_cast<GLint>( InMipLevel ), GL_FALSE, 0,
                                    Internal::ToGLImageAccess( InAccess ), Internal::ToGLInternalFormat( InTexture.Format ) );
        ImageSlots[InSlot]     = InTexture.Id;
        ImageMipLevels[InSlot] = InMipLevel;
        ImageAccesses[InSlot]  = InAccess;
    }
}

void LRHI::FRHICommandList::BindFramebuffer ( const FRHIFramebuffer *InFramebuffer ) noexcept
{
    Device->CheckThread();
    const UInt32 Id = InFramebuffer == nullptr ? 0U : InFramebuffer->Id;

    if ( CurrentFramebuffer != Id )
    {
        Internal::BindFramebuffer( GL_FRAMEBUFFER, Id );
        CurrentFramebuffer = Id;
    }
}

void LRHI::FRHICommandList::SetViewport ( const Maths::FVec2u InSize ) noexcept
{
    if ( not bHasViewport or CurrentViewport != InSize )
    {
        Internal::Viewport( 0, 0, static_cast<GLsizei>( InSize.X ), static_cast<GLsizei>( InSize.Y ) );
        CurrentViewport = InSize;
        bHasViewport    = true;
    }
}

void LRHI::FRHICommandList::ClearColor ( const Maths::FVec4f &InColor ) const noexcept
{
    Internal::ClearColor( InColor.X, InColor.Y, InColor.Z, InColor.W );
    Internal::Clear( GL_COLOR_BUFFER_BIT );
}

void LRHI::FRHICommandList::Draw ( const UInt32 InVertexCount, const UInt32 InFirstVertex ) const noexcept
{
    LUMINA_ASSERT( CurrentGraphicsPipeline != nullptr );
    Internal::DrawArrays( ToGLTopology( CurrentGraphicsPipeline->State.Topology ), static_cast<GLint>( InFirstVertex ),
                          static_cast<GLsizei>( InVertexCount ) );
}

void LRHI::FRHICommandList::DrawIndexed ( const UInt32 InIndexCount,
                                          const ERHIIndexType InIndexType,
                                          const USize InByteOffset ) const noexcept
{
    LUMINA_ASSERT( CurrentGraphicsPipeline != nullptr );

    const GLenum Type = InIndexType == ERHIIndexType::UInt16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
    Internal::DrawElements( ToGLTopology( CurrentGraphicsPipeline->State.Topology ), static_cast<GLsizei>( InIndexCount ), Type,
                            reinterpret_cast<const void *>( static_cast<UPtr>( InByteOffset ) ) );
}

LRHI::FRHIExpected LRHI::FRHICommandList::Dispatch ( const Maths::FVec3u InGroupCount ) const noexcept
{
    Device->CheckThread();
    if ( CurrentProgram == 0U or CurrentProgram == UnknownHandle or CurrentGraphicsPipeline != nullptr )
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::InvalidArgument, "No compute pipeline is bound" ) );
    }

    for ( USize Axis = 0U; Axis < 3U; ++Axis )
    {
        if ( InGroupCount.Data[Axis] == 0U or
             InGroupCount.Data[Axis] > Device->GetCapabilities().MaxComputeWorkGroupCount.Data[Axis] )
        {
            return MakeUnexpected(
                Internal::MakeError( ERHIErrorCode::InvalidArgument, "Compute group count is zero or exceeds device limits" ) );
        }
    }
    Internal::DispatchCompute( InGroupCount.X, InGroupCount.Y, InGroupCount.Z );
    return {};
}

LRHI::FRHIExpected LRHI::FRHICommandList::Dispatch2D ( const FRHIComputePipeline &InPipeline,
                                                       const Maths::FVec2u InThreadCount,
                                                       const Maths::FVec2u InLocalSize ) noexcept
{
    if ( InLocalSize.X == 0U or InLocalSize.Y == 0U )
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::InvalidArgument, "Compute local size must be non-zero" ) );
    }
    const Maths::FVec2u GroupCount( ( InThreadCount.X / InLocalSize.X ) + ( InThreadCount.X % InLocalSize.X != 0U ? 1U : 0U ),
                                    ( InThreadCount.Y / InLocalSize.Y ) + ( InThreadCount.Y % InLocalSize.Y != 0U ? 1U : 0U ) );
    BindPipeline( InPipeline );
    return Dispatch( Maths::FVec3u( GroupCount.X, GroupCount.Y, 1U ) );
}

void LRHI::FRHICommandList::MemoryBarrier ( const ERHIMemoryBarrier InBarrier ) const noexcept
{
    if ( InBarrier != ERHIMemoryBarrier::None )
    {
        Internal::MemoryBarrier( Internal::ToGLMemoryBarrier( InBarrier ) );
    }
}

void LRHI::FRHICommandList::ApplyGraphicsState ( const FRHIGraphicsState &InState ) noexcept
{
    if ( bHasGraphicsState and CurrentGraphicsState.CullMode == InState.CullMode and
         CurrentGraphicsState.bDepthTest == InState.bDepthTest and CurrentGraphicsState.bDepthWrite == InState.bDepthWrite and
         CurrentGraphicsState.bBlend == InState.bBlend )
    {
        return;
    }

    if ( not bHasGraphicsState or CurrentGraphicsState.CullMode != InState.CullMode )
    {
        SetCapability( GL_CULL_FACE, InState.CullMode != ERHICullMode::None );
        if ( InState.CullMode != ERHICullMode::None )
        {
            Internal::CullFace( InState.CullMode == ERHICullMode::Front ? GL_FRONT : GL_BACK );
        }
    }
    if ( not bHasGraphicsState or CurrentGraphicsState.bDepthTest != InState.bDepthTest )
    {
        SetCapability( GL_DEPTH_TEST, InState.bDepthTest );
        if ( InState.bDepthTest )
        {
            Internal::DepthFunc( GL_LEQUAL );
        }
    }
    if ( not bHasGraphicsState or CurrentGraphicsState.bDepthWrite != InState.bDepthWrite )
    {
        Internal::DepthMask( InState.bDepthWrite ? GL_TRUE : GL_FALSE );
    }
    if ( not bHasGraphicsState or CurrentGraphicsState.bBlend != InState.bBlend )
    {
        SetCapability( GL_BLEND, InState.bBlend );
        if ( InState.bBlend )
        {
            Internal::BlendFuncSeparate( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
        }
    }

    CurrentGraphicsState = InState;
    bHasGraphicsState    = true;
}
