/**
 * @file RHIFramebuffer.cpp
 * @brief Framebuffer implementation for OpenGL RHI
 */

#include "RHIGL.hpp"
#include <RHI/RHIFramebuffer.hpp>

/**
 * Public
 */

Lumina::RHI::FRHIFramebuffer::FRHIFramebuffer ( const UInt32 InId, const Maths::FVec2u InSize ) noexcept
    : Id( InId ), Size( InSize )
{
    /* */
}

Lumina::RHI::FRHIFramebuffer::FRHIFramebuffer ( FRHIFramebuffer &&InOther ) noexcept
    : Id( std::exchange( InOther.Id, 0U ) ), Size( InOther.Size )
{
    /* */
}

Lumina::RHI::FRHIFramebuffer &Lumina::RHI::FRHIFramebuffer::operator=( FRHIFramebuffer &&InOther ) noexcept
{
    if ( this != &InOther )
    {
        Reset();
        Id   = std::exchange( InOther.Id, 0U );
        Size = InOther.Size;
    }
    return *this;
}

Lumina::RHI::FRHIFramebuffer::~FRHIFramebuffer () noexcept
{
    Reset();
}

Lumina::Bool Lumina::RHI::FRHIFramebuffer::IsValid () const noexcept
{
    return Id != 0U;
}

Lumina::Maths::FVec2u Lumina::RHI::FRHIFramebuffer::GetSize () const noexcept
{
    return Size;
}

void Lumina::RHI::FRHIFramebuffer::Reset () noexcept
{
    if ( Id != 0U )
    {
        Internal::DeleteFramebuffers( 1, &Id );
    }
    Id   = 0U;
    Size = Maths::FVec2u( 0U );
}
