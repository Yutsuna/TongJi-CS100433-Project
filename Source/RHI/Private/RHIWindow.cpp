/**
 * @file RHIWindow.cpp
 * @brief Implementation of the FRHIWindow class.
 */

#include <RHI/RHIWindow.hpp>

#include <Maths/Vec.hpp>
#include <RHIPrivate.hpp>
#include <String/String.hpp>

extern "C"
{
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
}

#include <limits>
#include <utility>

/**
 * Private
 */

namespace
{

Lumina::UInt32 WindowCount     = 0U;
Lumina::Bool bErrorCallbackSet = false;

inline void OnGLFWError ( const Lumina::Int32 InCode, const Lumina::AnsiChar *InDescription ) noexcept
{
    LUMINA_LOG_ERROR( Lumina::RHI::Internal::LogRHI, "GLFW error {}: {}", InCode, InDescription );
}

inline void EnsureErrorCallback () noexcept
{
    if ( not bErrorCallbackSet )
    {
        glfwSetErrorCallback( OnGLFWError );
        bErrorCallbackSet = true;
    }
}

[[nodiscard]] inline GLFWwindow *GetWindow ( void *InWindow ) noexcept
{
    return static_cast<GLFWwindow *>( InWindow );
}

[[nodiscard]] inline Lumina::TExpected<void, Lumina::RHI::FRHIError>
ValidateDesc ( const Lumina::RHI::FRHIWindowDesc &InDesc ) noexcept
{
    if ( InDesc.Size.X == 0U or InDesc.Size.Y == 0U ) [[unlikely]]
    {
        return Lumina::MakeUnexpected( Lumina::RHI::Internal::MakeError( Lumina::RHI::ERHIErrorCode::InvalidArgument,
                                                                         "Window dimensions must be non-zero" ) );
    }

    constexpr Lumina::UInt32 MaxDimension = static_cast<Lumina::UInt32>( std::numeric_limits<Lumina::Int32>::max() );
    if ( InDesc.Size.X > MaxDimension or InDesc.Size.Y > MaxDimension ) [[unlikely]]
    {
        return Lumina::MakeUnexpected( Lumina::RHI::Internal::MakeError( Lumina::RHI::ERHIErrorCode::InvalidArgument,
                                                                         "Window dimensions exceed GLFW limits" ) );
    }

    return {};
}

[[nodiscard]] inline Lumina::TExpected<void, Lumina::RHI::FRHIError> EnsureGLFWInitialized () noexcept
{
    EnsureErrorCallback();

    if ( WindowCount == 0U and glfwInit() == GLFW_FALSE ) [[unlikely]]
    {
        return Lumina::MakeUnexpected(
            Lumina::RHI::Internal::MakeError( Lumina::RHI::ERHIErrorCode::InitializationFailed, "glfwInit failed" ) );
    }
    return {};
}

inline void SetGLFWWindowHints ( const Lumina::RHI::FRHIWindowDesc &InDesc ) noexcept
{
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
    glfwWindowHint( GLFW_RESIZABLE, InDesc.bResizable ? GLFW_TRUE : GLFW_FALSE );
    glfwWindowHint( GLFW_VISIBLE, InDesc.bVisible ? GLFW_TRUE : GLFW_FALSE );
}

[[nodiscard]] inline GLFWwindow *CreateGLFWWindow ( const Lumina::RHI::FRHIWindowDesc &InDesc ) noexcept
{
    const Lumina::FString Title{ InDesc.Title };
    const Lumina::Maths::FVec2i Size = static_cast<Lumina::Maths::FVec2i>( InDesc.Size );

    return glfwCreateWindow( Size.X, Size.Y, Title.c_str(), nullptr, nullptr );
}

} // namespace

/**
 * Public
 */

Lumina::RHI::FRHIWindow::FRHIWindow ( void *InWindow ) noexcept : NativeWindow( InWindow )
{
    /* */
}

Lumina::RHI::FRHIWindow::FRHIWindow ( FRHIWindow &&InOther ) noexcept
    : NativeWindow( std::exchange( InOther.NativeWindow, nullptr ) )
{
    /* */
}

Lumina::RHI::FRHIWindow &Lumina::RHI::FRHIWindow::operator=( FRHIWindow &&InOther ) noexcept
{
    if ( this != &InOther )
    {
        Reset();
        NativeWindow = std::exchange( InOther.NativeWindow, nullptr );
    }
    return *this;
}

Lumina::RHI::FRHIWindow::~FRHIWindow () noexcept
{
    Reset();
}

Lumina::TExpected<Lumina::RHI::FRHIWindow, Lumina::RHI::FRHIError>
Lumina::RHI::FRHIWindow::Create ( const FRHIWindowDesc &InDesc )
{
    LUMINA_TRY_EXPECT_ERROR( ValidateDesc( InDesc ) );
    LUMINA_TRY_EXPECT_ERROR( EnsureGLFWInitialized() );

    SetGLFWWindowHints( InDesc );

    GLFWwindow *Window = CreateGLFWWindow( InDesc );
    if ( Window == nullptr ) [[unlikely]]
    {
        if ( WindowCount == 0U )
        {
            glfwTerminate();
        }
        return MakeUnexpected(
            Internal::MakeError( ERHIErrorCode::InitializationFailed, "Failed to create an OpenGL 4.6 core context" ) );
    }

    ++WindowCount;

    FRHIWindow Result( Window );
    Result.MakeContextCurrent();
    Result.SetVSync( InDesc.bVSync );
    return Result;
}

void Lumina::RHI::FRHIWindow::PollEvents () const noexcept
{
    glfwPollEvents();
}

void Lumina::RHI::FRHIWindow::SwapBuffers () const noexcept
{
    LUMINA_ASSERT( IsValid() );
    glfwSwapBuffers( GetWindow( NativeWindow ) );
}

void Lumina::RHI::FRHIWindow::MakeContextCurrent () const noexcept
{
    LUMINA_ASSERT( IsValid() );
    glfwMakeContextCurrent( GetWindow( NativeWindow ) );
}

void Lumina::RHI::FRHIWindow::SetVSync ( const Bool bEnabled ) const noexcept
{
    LUMINA_ASSERT( IsValid() );
    glfwSwapInterval( bEnabled ? 1 : 0 );
}

void Lumina::RHI::FRHIWindow::RequestClose () const noexcept
{
    LUMINA_ASSERT( IsValid() );
    glfwSetWindowShouldClose( GetWindow( NativeWindow ), GLFW_TRUE );
}

Lumina::Bool Lumina::RHI::FRHIWindow::ShouldClose () const noexcept
{
    return IsValid() and glfwWindowShouldClose( GetWindow( NativeWindow ) ) == GLFW_TRUE;
}

Lumina::Bool Lumina::RHI::FRHIWindow::IsValid () const noexcept
{
    return NativeWindow != nullptr;
}

Lumina::Maths::FVec2u Lumina::RHI::FRHIWindow::GetFramebufferSize () const noexcept
{
    if ( not IsValid() )
    {
        return { 0U };
    }

    Int32 Width  = 0;
    Int32 Height = 0;
    glfwGetFramebufferSize( GetWindow( NativeWindow ), &Width, &Height );

    return { static_cast<UInt32>( Width ), static_cast<UInt32>( Height ) };
}

void Lumina::RHI::FRHIWindow::Reset () noexcept
{
    if ( NativeWindow == nullptr )
    {
        return;
    }

    glfwDestroyWindow( GetWindow( NativeWindow ) );
    NativeWindow = nullptr;

    LUMINA_ASSERT( WindowCount > 0U );
    --WindowCount;

    if ( WindowCount == 0U )
    {
        glfwTerminate();
    }
}
