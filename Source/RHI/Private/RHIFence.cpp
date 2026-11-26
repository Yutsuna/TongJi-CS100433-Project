/**
 * @file RHIFence.cpp
 * @brief Implementation of the FRHIFence class.
 */

#include "RHIPrivate.hpp"
#include <RHI/RHIFence.hpp>

/**
 * Public
 */

Lumina::RHI::FRHIFence::FRHIFence ( void *InHandle ) noexcept : Handle( InHandle )
{
    /* */
}

Lumina::RHI::FRHIFence::FRHIFence ( FRHIFence &&InOther ) noexcept : Handle( std::exchange( InOther.Handle, nullptr ) )
{
    /* */
}

Lumina::RHI::FRHIFence &Lumina::RHI::FRHIFence::operator=( FRHIFence &&InOther ) noexcept
{
    if ( this != &InOther )
    {
        Reset();
        Handle = std::exchange( InOther.Handle, nullptr );
    }
    return *this;
}

Lumina::RHI::FRHIFence::~FRHIFence () noexcept
{
    Reset();
}

Lumina::TExpected<Lumina::RHI::ERHIFenceStatus, Lumina::RHI::FRHIError>
Lumina::RHI::FRHIFence::Wait ( const UInt64 InTimeoutNanoseconds ) const noexcept
{
    if ( not IsValid() )
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::InvalidArgument, "Cannot wait on an empty fence" ) );
    }

    const GLenum Result = Internal::ClientWaitSync( static_cast<GLsync>( Handle ), GL_SYNC_FLUSH_COMMANDS_BIT,
                                                    static_cast<GLuint64>( InTimeoutNanoseconds ) );
    if ( Result == GL_ALREADY_SIGNALED or Result == GL_CONDITION_SATISFIED )
    {
        return ERHIFenceStatus::Signaled;
    }
    if ( Result == GL_TIMEOUT_EXPIRED )
    {
        return ERHIFenceStatus::Timeout;
    }
    return MakeUnexpected( Internal::MakeError( ERHIErrorCode::BackendError, "glClientWaitSync failed" ) );
}

Lumina::Bool Lumina::RHI::FRHIFence::IsValid () const noexcept
{
    return Handle != nullptr;
}

void Lumina::RHI::FRHIFence::Reset () noexcept
{
    if ( Handle != nullptr )
    {
        Internal::DeleteSync( static_cast<GLsync>( Handle ) );
        Handle = nullptr;
    }
}
