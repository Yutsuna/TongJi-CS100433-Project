/**
 * @file RHIDevice.cpp
 * @brief Direct OpenGL device and resource factory implementation.
 */

#include <Core/CoreDefinitions.hpp>

#include "RHIPrivate.hpp"
#include <RHI/RHIDevice.hpp>

#include <Algo/Any.hpp>
#include <Algo/Find.hpp>
#include <Algo/ForEach.hpp>
#include <Filesystem/File.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <bit>

/**
 * Private
 */

namespace
{

namespace LRHI = Lumina::RHI;

#ifndef NDEBUG
void APIENTRY OnGLDebugMessage ( GLenum,
                                 GLenum,
                                 Lumina::UInt32,
                                 const GLenum InSeverity,
                                 GLsizei,
                                 const GLchar *InMessage,
                                 const void LUMINA_UNUSED *Ptr ) noexcept
{
    if ( InSeverity == GL_DEBUG_SEVERITY_HIGH )
    {
        LUMINA_LOG_ERROR( LRHI::Internal::LogRHI, "OpenGL: {}", InMessage );
    }
    else if ( InSeverity != GL_DEBUG_SEVERITY_NOTIFICATION )
    {
        LUMINA_LOG_WARNING( LRHI::Internal::LogRHI, "OpenGL: {}", InMessage );
    }
}
#endif

Lumina::FString GetShaderLog ( const Lumina::UInt32 InShader ) noexcept
{
    GLint LogLength = 0;
    LRHI::Internal::GetShaderiv( InShader, GL_INFO_LOG_LENGTH, &LogLength );

    if ( LogLength <= 1 ) [[unlikely]]
    {
        return "Unknown GLSL compiler error";
    }

    Lumina::FString Log( static_cast<Lumina::USize>( LogLength - 1 ), '\0' );
    LRHI::Internal::GetShaderInfoLog( InShader, LogLength, nullptr, Log.data() );
    return Log;
}

Lumina::FString GetProgramLog ( const Lumina::UInt32 InProgram ) noexcept
{
    GLint LogLength = 0;
    LRHI::Internal::GetProgramiv( InProgram, GL_INFO_LOG_LENGTH, &LogLength );

    if ( LogLength <= 1 ) [[unlikely]]
    {
        return "Unknown GLSL linker error";
    }

    Lumina::FString Log( static_cast<Lumina::USize>( LogLength - 1 ), '\0' );
    LRHI::Internal::GetProgramInfoLog( InProgram, LogLength, nullptr, Log.data() );
    return Log;
}

Lumina::TExpected<Lumina::UInt32, LRHI::FRHIError> LinkProgram ( const Lumina::TSpan<const Lumina::UInt32> InShaders,
                                                                 const Lumina::FStringView InDebugName ) noexcept
{
    const Lumina::UInt32 Program = LRHI::Internal::CreateProgram();
    if ( Program == 0U ) [[unlikely]]
    {
        return Lumina::MakeUnexpected( LRHI::Internal::MakeError( LRHI::ERHIErrorCode::BackendError, "glCreateProgram failed" ) );
    }

    Lumina::Algo::ForEach( InShaders, [Program] ( const Lumina::UInt32 InShader ) noexcept
    {
        LRHI::Internal::AttachShader( Program, InShader );
    } );
    LRHI::Internal::LinkProgram( Program );

    GLint Linked = GL_FALSE;
    LRHI::Internal::GetProgramiv( Program, GL_LINK_STATUS, &Linked );
    Lumina::Algo::ForEach( InShaders, [Program] ( const Lumina::UInt32 InShader ) noexcept
    {
        LRHI::Internal::DetachShader( Program, InShader );
    } );

    if ( Linked != GL_TRUE )
    {
        const Lumina::FString Log = GetProgramLog( Program );
        LRHI::Internal::DeleteProgram( Program );
        return Lumina::MakeUnexpected( LRHI::Internal::MakeError( LRHI::ERHIErrorCode::ProgramLinkFailed, Log ) );
    }

    LRHI::Internal::LabelObject( GL_PROGRAM, Program, InDebugName );
    return Program;
}

Lumina::UInt32 VertexComponentCount ( const LRHI::ERHIVertexFormat InFormat ) noexcept
{
    using EVertexFmt = LRHI::ERHIVertexFormat;

    switch ( InFormat )
    {
    case EVertexFmt::Float:
    case EVertexFmt::UInt:
        return 1U;
    case EVertexFmt::Float2:
    case EVertexFmt::UInt2:
        return 2U;
    case EVertexFmt::Float3:
    case EVertexFmt::UInt3:
        return 3U;
    case EVertexFmt::Float4:
    case EVertexFmt::UInt4:
        return 4U;
    }
    return 0U;
}

Lumina::Bool IsIntegerVertexFormat ( const LRHI::ERHIVertexFormat InFormat ) noexcept
{
    using EVertexFmt = LRHI::ERHIVertexFormat;

    return InFormat == EVertexFmt::UInt or InFormat == EVertexFmt::UInt2 or InFormat == EVertexFmt::UInt3 or
           InFormat == EVertexFmt::UInt4;
}

} // namespace

/**
 * Public
 */

LRHI::FRHIDevice::FRHIDevice ( FRHIWindow &InWindow ) noexcept : Window( &InWindow ), OwnerThread( FThread::GetCurrentId() )
{
    /* */
}

LRHI::FRHIDevice::FRHIDevice ( FRHIDevice &&InOther ) noexcept
    : Window( std::exchange( InOther.Window, nullptr ) ), Capabilities( std::move( InOther.Capabilities ) ),
      OwnerThread( InOther.OwnerThread )
{
    /* */
}

LRHI::FRHIDevice &LRHI::FRHIDevice::operator=( FRHIDevice &&InOther ) noexcept
{
    if ( this != &InOther )
    {
        Window       = std::exchange( InOther.Window, nullptr );
        Capabilities = std::move( InOther.Capabilities );
        OwnerThread  = InOther.OwnerThread;
    }
    return *this;
}

Lumina::TExpected<LRHI::FRHIDevice, LRHI::FRHIError> LRHI::FRHIDevice::Create ( FRHIWindow &InWindow )
{
    if ( not InWindow.IsValid() ) [[unlikely]]
    {
        return MakeUnexpected(
            Internal::MakeError( ERHIErrorCode::InvalidArgument, "Cannot create a device from an empty window" ) );
    }

    InWindow.MakeContextCurrent();
    LUMINA_TRY_EXPECT_ERROR( Internal::LoadGLFunctions() );

    FRHIDevice Result( InWindow );
    const auto ReadString = [] ( const GLenum InName ) noexcept -> FString
    {
        const GLubyte *Value = Internal::GetString( InName );
        return Value == nullptr ? FString{} : FString( reinterpret_cast<const AnsiChar *>( Value ) );
    };
    Result.Capabilities.Vendor   = ReadString( GL_VENDOR );
    Result.Capabilities.Renderer = ReadString( GL_RENDERER );
    Result.Capabilities.Version  = ReadString( GL_VERSION );

    const auto ReadUInt = [] ( const GLenum InName ) noexcept -> UInt32
    {
        GLint Value = 0;
        Internal::GetIntegerv( InName, &Value );
        return static_cast<UInt32>( Value );
    };
    Result.Capabilities.MaxTextureSize           = ReadUInt( GL_MAX_TEXTURE_SIZE );
    Result.Capabilities.MaxCombinedTextureUnits  = ReadUInt( GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS );
    Result.Capabilities.MaxImageUnits            = ReadUInt( GL_MAX_IMAGE_UNITS );
    Result.Capabilities.MaxStorageBufferBindings = ReadUInt( GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS );

    for ( UInt32 Axis = 0U; Axis < 3U; ++Axis )
    {
        GLint Value = 0;
        Internal::GetIntegeriV( GL_MAX_COMPUTE_WORK_GROUP_COUNT, Axis, &Value );
        Result.Capabilities.MaxComputeWorkGroupCount.Data[Axis] = static_cast<UInt32>( Value );
        Internal::GetIntegeriV( GL_MAX_COMPUTE_WORK_GROUP_SIZE, Axis, &Value );
        Result.Capabilities.MaxComputeWorkGroupSize.Data[Axis] = static_cast<UInt32>( Value );
    }

#ifndef NDEBUG
    Internal::Enable( GL_DEBUG_OUTPUT );
    Internal::Enable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
    Internal::DebugMessageCallback( OnGLDebugMessage, nullptr );
#endif

    LUMINA_LOG_INFO( Internal::LogRHI, "OpenGL device: {} — {}", Result.Capabilities.Renderer, Result.Capabilities.Version );
    return Result;
}

Lumina::TExpected<LRHI::FRHIBuffer, LRHI::FRHIError>
LRHI::FRHIDevice::CreateBuffer ( const FRHIBufferDesc &InDesc, const TSpan<const Byte> InInitialData ) const noexcept
{
    CheckThread();
    if ( InDesc.Size == 0U or InDesc.Usage == ERHIBufferUsage::None or InInitialData.size() > InDesc.Size or
         InDesc.Size > static_cast<USize>( std::numeric_limits<GLsizeiptr>::max() ) )
    {
        return MakeUnexpected(
            Internal::MakeError( ERHIErrorCode::InvalidArgument, "Invalid buffer descriptor or initial data" ) );
    }
    if ( InDesc.bCoherent and not InDesc.bPersistent )
    {
        return MakeUnexpected(
            Internal::MakeError( ERHIErrorCode::InvalidArgument, "Coherent mapping requires persistent mapping" ) );
    }
    if ( InDesc.bPersistent and not HasAnyFlags( InDesc.Usage, ERHIBufferUsage::Dynamic | ERHIBufferUsage::Readback ) )
    {
        return MakeUnexpected(
            Internal::MakeError( ERHIErrorCode::InvalidArgument, "Persistent buffers require Dynamic or Readback usage" ) );
    }

    GLbitfield Flags = 0U;
    if ( HasAnyFlags( InDesc.Usage, ERHIBufferUsage::Dynamic ) or not InInitialData.empty() )
    {
        Flags |= GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT;
    }
    if ( HasAnyFlags( InDesc.Usage, ERHIBufferUsage::Readback ) )
    {
        Flags |= GL_MAP_READ_BIT;
    }
    if ( InDesc.bPersistent )
    {
        Flags |= GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    }

    UInt32 Id = 0U;
    Internal::CreateBuffers( 1, &Id );
    if ( Id == 0U )
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::BackendError, "glCreateBuffers failed" ) );
    }
    Internal::NamedBufferStorage( Id, static_cast<GLsizeiptr>( InDesc.Size ), nullptr, Flags );
    if ( not InInitialData.empty() )
    {
        Internal::NamedBufferSubData( Id, 0, static_cast<GLsizeiptr>( InInitialData.size() ), InInitialData.data() );
    }
    Internal::LabelObject( GL_BUFFER, Id, InDesc.DebugName );
    return FRHIBuffer( Id, InDesc.Size, InDesc.Usage, InDesc.bPersistent, InDesc.bCoherent );
}

Lumina::TExpected<LRHI::FRHITexture, LRHI::FRHIError>
LRHI::FRHIDevice::CreateTexture ( const FRHITextureDesc &InDesc ) const noexcept
{
    CheckThread();

    const UInt32 MaxDimension = InDesc.Size.X > InDesc.Size.Y ? InDesc.Size.X : InDesc.Size.Y;
    const UInt32 MaxMipLevels = MaxDimension == 0U ? 0U : static_cast<UInt32>( std::bit_width( MaxDimension ) );

    if ( InDesc.Size.X == 0U or InDesc.Size.Y == 0U or MaxDimension > Capabilities.MaxTextureSize or InDesc.MipLevels == 0U or
         InDesc.MipLevels > MaxMipLevels or InDesc.Usage == ERHITextureUsage::None )
    {
        return MakeUnexpected(
            Internal::MakeError( ERHIErrorCode::InvalidArgument, "Invalid or unsupported texture descriptor" ) );
    }

    UInt32 Id = 0U;
    Internal::CreateTextures( GL_TEXTURE_2D, 1, &Id );
    if ( Id == 0U )
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::BackendError, "glCreateTextures failed" ) );
    }
    Internal::TextureStorage2D( Id, static_cast<GLsizei>( InDesc.MipLevels ), Internal::ToGLInternalFormat( InDesc.Format ),
                                static_cast<GLsizei>( InDesc.Size.X ), static_cast<GLsizei>( InDesc.Size.Y ) );

    const Bool bInteger = InDesc.Format == ERHITextureFormat::R32UI;
    Internal::TextureParameteri( Id, GL_TEXTURE_MIN_FILTER,
                                 static_cast<GLint>( bInteger                ? GL_NEAREST
                                                     : InDesc.MipLevels > 1U ? GL_LINEAR_MIPMAP_LINEAR
                                                                             : GL_LINEAR ) );
    Internal::TextureParameteri( Id, GL_TEXTURE_MAG_FILTER, static_cast<GLint>( bInteger ? GL_NEAREST : GL_LINEAR ) );
    Internal::TextureParameteri( Id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    Internal::TextureParameteri( Id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    Internal::LabelObject( GL_TEXTURE, Id, InDesc.DebugName );

    return FRHITexture( Id, InDesc.Size, InDesc.Format, InDesc.MipLevels );
}

Lumina::TExpected<LRHI::FRHISampler, LRHI::FRHIError>
LRHI::FRHIDevice::CreateSampler ( const FRHISamplerDesc &InDesc ) const noexcept
{
    CheckThread();
    UInt32 Id = 0U;
    Internal::CreateSamplers( 1, &Id );

    if ( Id == 0U )
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::BackendError, "glCreateSamplers failed" ) );
    }

    Internal::SamplerParameteri( Id, GL_TEXTURE_MIN_FILTER,
                                 static_cast<GLint>( Internal::ToGLFilter( InDesc.MinFilter, InDesc.bMipmapped ) ) );
    Internal::SamplerParameteri( Id, GL_TEXTURE_MAG_FILTER,
                                 static_cast<GLint>( Internal::ToGLFilter( InDesc.MagFilter, false ) ) );
    Internal::SamplerParameteri( Id, GL_TEXTURE_WRAP_S, static_cast<GLint>( Internal::ToGLAddressMode( InDesc.AddressU ) ) );
    Internal::SamplerParameteri( Id, GL_TEXTURE_WRAP_T, static_cast<GLint>( Internal::ToGLAddressMode( InDesc.AddressV ) ) );
    Internal::LabelObject( GL_SAMPLER, Id, InDesc.DebugName );
    return FRHISampler( Id );
}

Lumina::TExpected<LRHI::FRHIFramebuffer, LRHI::FRHIError>
LRHI::FRHIDevice::CreateFramebuffer ( const FRHIFramebufferDesc &InDesc ) const noexcept
{
    CheckThread();
    if ( InDesc.ColorAttachments.size() > 32U or ( InDesc.ColorAttachments.empty() and InDesc.DepthAttachment == nullptr ) )
    {
        return MakeUnexpected(
            Internal::MakeError( ERHIErrorCode::InvalidArgument, "Framebuffer requires at least one attachment" ) );
    }

    Maths::FVec2u Size( 0U );
    if ( not InDesc.ColorAttachments.empty() and InDesc.ColorAttachments.front() != nullptr )
    {
        Size = InDesc.ColorAttachments.front()->GetSize();
    }
    else if ( InDesc.DepthAttachment != nullptr )
    {
        Size = InDesc.DepthAttachment->GetSize();
    }
    if ( Size.X == 0U or Size.Y == 0U )
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::InvalidArgument, "Invalid framebuffer attachment" ) );
    }

    UInt32 Id = 0U;
    Internal::CreateFramebuffers( 1, &Id );
    if ( Id == 0U )
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::BackendError, "glCreateFramebuffers failed" ) );
    }

    TArray<GLenum, 32U> DrawBuffers{};
    UInt32 ColorCount = 0U;
    for ( const FRHITexture *Texture : InDesc.ColorAttachments )
    {
        if ( Texture == nullptr or not Texture->IsValid() or Texture->GetSize() != Size )
        {
            Internal::DeleteFramebuffers( 1, &Id );
            return MakeUnexpected( Internal::MakeError( ERHIErrorCode::InvalidArgument,
                                                        "Framebuffer color attachments must be valid and equally sized" ) );
        }
        Internal::NamedFramebufferTexture( Id, GL_COLOR_ATTACHMENT0 + ColorCount, Texture->Id, 0 );
        DrawBuffers[ColorCount] = GL_COLOR_ATTACHMENT0 + ColorCount;
        ++ColorCount;
    }
    if ( InDesc.DepthAttachment != nullptr )
    {
        if ( not InDesc.DepthAttachment->IsValid() or InDesc.DepthAttachment->GetSize() != Size or
             InDesc.DepthAttachment->GetFormat() != ERHITextureFormat::Depth24Stencil8 )
        {
            Internal::DeleteFramebuffers( 1, &Id );
            return MakeUnexpected(
                Internal::MakeError( ERHIErrorCode::InvalidArgument, "Invalid framebuffer depth attachment" ) );
        }
        Internal::NamedFramebufferTexture( Id, GL_DEPTH_STENCIL_ATTACHMENT, InDesc.DepthAttachment->Id, 0 );
    }

    if ( ColorCount == 0U )
    {
        DrawBuffers[0] = GL_NONE;
        Internal::NamedFramebufferDrawBuffers( Id, 1, DrawBuffers.data() );
    }
    else
    {
        Internal::NamedFramebufferDrawBuffers( Id, static_cast<GLsizei>( ColorCount ), DrawBuffers.data() );
    }

    const GLenum Status = Internal::CheckNamedFramebufferStatus( Id, GL_FRAMEBUFFER );
    if ( Status != GL_FRAMEBUFFER_COMPLETE )
    {
        Internal::DeleteFramebuffers( 1, &Id );
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::BackendError, "Framebuffer is incomplete" ) );
    }
    Internal::LabelObject( GL_FRAMEBUFFER, Id, InDesc.DebugName );
    return FRHIFramebuffer( Id, Size );
}

Lumina::TExpected<LRHI::FRHIShader, LRHI::FRHIError> LRHI::FRHIDevice::CreateShader (
    const ERHIShaderStage InStage, const FStringView InSource, const FStringView InDebugName ) const noexcept
{
    CheckThread();
    if ( InSource.empty() or InSource.size() > static_cast<USize>( std::numeric_limits<GLint>::max() ) )
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::InvalidArgument, "Shader source is empty or too large" ) );
    }

    const UInt32 Id = Internal::CreateShader( Internal::ToGLShaderStage( InStage ) );
    if ( Id == 0U )
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::BackendError, "glCreateShader failed" ) );
    }
    const AnsiChar *Source = InSource.data();
    const GLint Length     = static_cast<GLint>( InSource.size() );
    Internal::ShaderSource( Id, 1, &Source, &Length );
    Internal::CompileShader( Id );

    GLint Compiled = GL_FALSE;
    Internal::GetShaderiv( Id, GL_COMPILE_STATUS, &Compiled );
    if ( Compiled != GL_TRUE )
    {
        const FString Log = GetShaderLog( Id );
        Internal::DeleteShader( Id );
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::ShaderCompilationFailed, Log ) );
    }
    Internal::LabelObject( GL_SHADER, Id, InDebugName );
    return FRHIShader( Id, InStage );
}

Lumina::TExpected<LRHI::FRHIShader, LRHI::FRHIError>
LRHI::FRHIDevice::CreateShaderFromFile ( const ERHIShaderStage InStage, const Filesystem::FPath &InPath ) const noexcept
{
    auto Source = Filesystem::FFile::ReadAllText( InPath );
    if ( not Source )
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::IoError, "Unable to read shader: " + InPath.ToString() ) );
    }
    return CreateShader( InStage, *Source, InPath.GetFilename() );
}

Lumina::TExpected<LRHI::FRHIGraphicsPipeline, LRHI::FRHIError>
LRHI::FRHIDevice::CreateGraphicsPipeline ( const FRHIGraphicsPipelineDesc &InDesc ) const noexcept
{
    CheckThread();
    if ( InDesc.VertexShader == nullptr or InDesc.FragmentShader == nullptr or not InDesc.VertexShader->IsValid() or
         not InDesc.FragmentShader->IsValid() or InDesc.VertexShader->Stage != ERHIShaderStage::Vertex or
         InDesc.FragmentShader->Stage != ERHIShaderStage::Fragment )
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::InvalidArgument,
                                                    "Graphics pipeline requires valid vertex and fragment shaders" ) );
    }

    const TArray<UInt32, 2U> Shaders = { InDesc.VertexShader->Id, InDesc.FragmentShader->Id };
    LUMINA_EXPECT( Program, LinkProgram( Shaders, InDesc.DebugName ) );

    UInt32 VertexArray = 0U;
    Internal::CreateVertexArrays( 1, &VertexArray );
    if ( VertexArray == 0U )
    {
        Internal::DeleteProgram( Program );
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::BackendError, "glCreateVertexArrays failed" ) );
    }

    if ( Algo::AnyOf( InDesc.VertexBindings, [&] ( const FRHIVertexBinding &InBinding ) noexcept -> Bool
    {
        return InBinding.Stride == 0U;
    } ) )
    {
        Internal::DeleteVertexArrays( 1, &VertexArray );
        Internal::DeleteProgram( Program );
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::InvalidArgument, "Vertex binding stride must be non-zero" ) );
    }

    for ( const FRHIVertexAttribute &Attribute : InDesc.VertexAttributes )
    {
        const auto *const Binding =
            Algo::FindIf( InDesc.VertexBindings, [&Attribute] ( const FRHIVertexBinding &InBinding ) noexcept
        {
            return InBinding.Binding == Attribute.Binding;
        } );
        const UInt32 ComponentCount = VertexComponentCount( Attribute.Format );
        if ( Binding == nullptr or Attribute.Offset + ( ComponentCount * sizeof( Float32 ) ) > Binding->Stride )
        {
            Internal::DeleteVertexArrays( 1, &VertexArray );
            Internal::DeleteProgram( Program );
            return MakeUnexpected(
                Internal::MakeError( ERHIErrorCode::InvalidArgument, "Vertex attribute exceeds its binding" ) );
        }

        Internal::EnableVertexArrayAttrib( VertexArray, Attribute.Location );
        Internal::VertexArrayAttribBinding( VertexArray, Attribute.Location, Attribute.Binding );
        if ( IsIntegerVertexFormat( Attribute.Format ) )
        {
            Internal::VertexArrayAttribIFormat( VertexArray, Attribute.Location, static_cast<GLint>( ComponentCount ),
                                                GL_UNSIGNED_INT, Attribute.Offset );
        }
        else
        {
            Internal::VertexArrayAttribFormat( VertexArray, Attribute.Location, static_cast<GLint>( ComponentCount ), GL_FLOAT,
                                               GL_FALSE, Attribute.Offset );
        }
    }
    Internal::LabelObject( GL_VERTEX_ARRAY, VertexArray, InDesc.DebugName );
    return FRHIGraphicsPipeline( Program, VertexArray, InDesc.State, InDesc.VertexBindings );
}

Lumina::TExpected<LRHI::FRHIComputePipeline, LRHI::FRHIError>
LRHI::FRHIDevice::CreateComputePipeline ( const FRHIComputePipelineDesc &InDesc ) const noexcept
{
    CheckThread();
    if ( InDesc.ComputeShader == nullptr or not InDesc.ComputeShader->IsValid() or
         InDesc.ComputeShader->Stage != ERHIShaderStage::Compute )
    {
        return MakeUnexpected(
            Internal::MakeError( ERHIErrorCode::InvalidArgument, "Compute pipeline requires a valid compute shader" ) );
    }

    const TArray<UInt32, 1U> Shaders = { InDesc.ComputeShader->Id };
    LUMINA_EXPECT( Program, LinkProgram( Shaders, InDesc.DebugName ) );
    return FRHIComputePipeline( Program );
}

Lumina::TExpected<LRHI::FRHIFence, LRHI::FRHIError> LRHI::FRHIDevice::CreateFence () const noexcept
{
    CheckThread();
    GLsync Handle = Internal::FenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0U );
    if ( Handle == nullptr )
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::BackendError, "glFenceSync failed" ) );
    }
    return FRHIFence( static_cast<void *>( Handle ) );
}

LRHI::FRHICommandList LRHI::FRHIDevice::CreateCommandList () const noexcept
{
    CheckThread();
    return FRHICommandList( *this );
}

const LRHI::FRHICapabilities &LRHI::FRHIDevice::GetCapabilities () const noexcept
{
    return Capabilities;
}

Lumina::Bool LRHI::FRHIDevice::IsValid () const noexcept
{
    return Window != nullptr and Window->IsValid();
}

void LRHI::FRHIDevice::CheckThread () const noexcept
{
    LUMINA_ASSERT( IsValid() );
    LUMINA_ASSERT( OwnerThread == FThread::GetCurrentId() );
}
