/**
 * @file RHIWindow.cpp
 * @brief Implementation of the FRHIWindow class.
 */

#include <RHI/RHIWindow.hpp>
#include <RHIPrivate.hpp>

#include <HAL/LockGuard.hpp>
#include <HAL/PlatformMutex.hpp>
#include <Maths/Vec.hpp>
#include <String/String.hpp>
#include <Templates/Optional.hpp>

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

namespace LRHI = Lumina::RHI;

Lumina::UInt32 WindowCount     = 0U;
Lumina::Bool bErrorCallbackSet = false;

/** NOTE: Serializes GLFW init/terminate and the window count they depend on. */
Lumina::HAL::FPlatformMutex GLFWStateMutex;

inline void OnGLFWError ( const Lumina::Int32 InCode, const Lumina::AnsiChar *InDescription ) noexcept
{
    LUMINA_LOG_ERROR( LRHI::Internal::LogRHI, "GLFW error {}: {}", InCode, InDescription );
}

/** NOTE: The caller must hold GLFWStateMutex. */
inline void EnsureErrorCallbackLocked () noexcept
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

[[nodiscard]] inline Lumina::TExpected<void, LRHI::FRHIError> ValidateDesc ( const LRHI::FRHIWindowDesc &InDesc ) noexcept
{
    if ( InDesc.Size.X == 0U or InDesc.Size.Y == 0U ) [[unlikely]]
    {
        return Lumina::MakeUnexpected(
            LRHI::Internal::MakeError( LRHI::ERHIErrorCode::InvalidArgument, "Window dimensions must be non-zero" ) );
    }

    constexpr Lumina::UInt32 MaxDimension = static_cast<Lumina::UInt32>( std::numeric_limits<Lumina::Int32>::max() );
    if ( InDesc.Size.X > MaxDimension or InDesc.Size.Y > MaxDimension ) [[unlikely]]
    {
        return Lumina::MakeUnexpected(
            LRHI::Internal::MakeError( LRHI::ERHIErrorCode::InvalidArgument, "Window dimensions exceed GLFW limits" ) );
    }

    return {};
}

/** NOTE: The caller must hold GLFWStateMutex and keep it until the window count is updated. */
[[nodiscard]] inline Lumina::TExpected<void, LRHI::FRHIError> EnsureGLFWInitializedLocked () noexcept
{
    EnsureErrorCallbackLocked();

    if ( WindowCount == 0U and glfwInit() == GLFW_FALSE ) [[unlikely]]
    {
        return Lumina::MakeUnexpected(
            LRHI::Internal::MakeError( LRHI::ERHIErrorCode::InitializationFailed, "glfwInit failed" ) );
    }
    return {};
}

inline void SetGLFWWindowHints ( const LRHI::FRHIWindowDesc &InDesc ) noexcept
{
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
    glfwWindowHint( GLFW_RESIZABLE, InDesc.bResizable ? GLFW_TRUE : GLFW_FALSE );
    glfwWindowHint( GLFW_VISIBLE, InDesc.bVisible ? GLFW_TRUE : GLFW_FALSE );
}

[[nodiscard]] inline GLFWwindow *CreateGLFWWindow ( const LRHI::FRHIWindowDesc &InDesc ) noexcept
{
    const Lumina::FString Title{ InDesc.Title };
    const Lumina::Maths::FVec2i Size = static_cast<Lumina::Maths::FVec2i>( InDesc.Size );

    return glfwCreateWindow( Size.X, Size.Y, Title.c_str(), nullptr, nullptr );
}

} // namespace

/**
 * Public
 */

LRHI::FRHIWindow::FRHIWindow ( void *InWindow ) noexcept : NativeWindow( InWindow )
{
    /* */
}

LRHI::FRHIWindow::FRHIWindow ( FRHIWindow &&InOther ) noexcept : NativeWindow( std::exchange( InOther.NativeWindow, nullptr ) )
{
    /* */
}

LRHI::FRHIWindow &LRHI::FRHIWindow::operator=( FRHIWindow &&InOther ) noexcept
{
    if ( this != &InOther )
    {
        Reset();
        NativeWindow = std::exchange( InOther.NativeWindow, nullptr );
    }
    return *this;
}

LRHI::FRHIWindow::~FRHIWindow () noexcept
{
    Reset();
}

Lumina::TExpected<LRHI::FRHIWindow, LRHI::FRHIError> LRHI::FRHIWindow::Create ( const FRHIWindowDesc &InDesc )
{
    LUMINA_TRY_EXPECT_ERROR( ValidateDesc( InDesc ) );

    const Lumina::HAL::TLockGuard<Lumina::HAL::FPlatformMutex> Lock( GLFWStateMutex );
    LUMINA_TRY_EXPECT_ERROR( EnsureGLFWInitializedLocked() );

    SetGLFWWindowHints( InDesc );

    GLFWwindow *Window = CreateGLFWWindow( InDesc );
    if ( Window == nullptr ) [[unlikely]]
    {
        if ( WindowCount == 0U )
        {
            glfwTerminate();
        }
        return MakeUnexpected(
            LRHI::Internal::MakeError( ERHIErrorCode::InitializationFailed, "Failed to create an OpenGL 4.6 core context" ) );
    }

    ++WindowCount;

    FRHIWindow Result( Window );
    Result.MakeContextCurrent();
    Result.SetVSync( InDesc.bVSync );
    return Result;
}

void LRHI::FRHIWindow::PollEvents () const noexcept
{
    glfwPollEvents();
}

void LRHI::FRHIWindow::SwapBuffers () const noexcept
{
    LUMINA_ASSERT( IsValid() );
    glfwSwapBuffers( GetWindow( NativeWindow ) );
}

void LRHI::FRHIWindow::MakeContextCurrent () const noexcept
{
    LUMINA_ASSERT( IsValid() );
    glfwMakeContextCurrent( GetWindow( NativeWindow ) );
}

void LRHI::FRHIWindow::SetVSync ( const Bool bEnabled ) const noexcept
{
    LUMINA_ASSERT( IsValid() );
    glfwSwapInterval( bEnabled ? 1 : 0 );
}

void LRHI::FRHIWindow::RequestClose () const noexcept
{
    LUMINA_ASSERT( IsValid() );
    glfwSetWindowShouldClose( GetWindow( NativeWindow ), GLFW_TRUE );
}

Lumina::Bool LRHI::FRHIWindow::ShouldClose () const noexcept
{
    return IsValid() and glfwWindowShouldClose( GetWindow( NativeWindow ) ) == GLFW_TRUE;
}

Lumina::Bool LRHI::FRHIWindow::IsValid () const noexcept
{
    return NativeWindow != nullptr;
}

Lumina::TOptional<Lumina::Maths::FVec2u> LRHI::FRHIWindow::GetFramebufferSize () const noexcept
{
    if ( not IsValid() )
    {
        return {};
    }

    Int32 Width  = 0;
    Int32 Height = 0;
    glfwGetFramebufferSize( GetWindow( NativeWindow ), &Width, &Height );

    return Lumina::TOptional<Lumina::Maths::FVec2u>(
        Lumina::Maths::FVec2u( static_cast<UInt32>( Width ), static_cast<UInt32>( Height ) ) );
}

void LRHI::FRHIWindow::Reset () noexcept
{
    if ( NativeWindow == nullptr )
    {
        return;
    }

    const Lumina::HAL::TLockGuard<Lumina::HAL::FPlatformMutex> Lock( GLFWStateMutex );
    glfwDestroyWindow( GetWindow( NativeWindow ) );
    NativeWindow = nullptr;

    LUMINA_ASSERT( WindowCount > 0U );
    --WindowCount;

    if ( WindowCount == 0U )
    {
        glfwTerminate();
    }
}
