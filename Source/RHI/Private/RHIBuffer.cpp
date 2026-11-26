/**
 * @file RHIBuffer.cpp
 * @brief RHI Buffer Implementation.
 */

#include <RHI/RHIBuffer.hpp>

#include "RHIPrivate.hpp"

/**
 * Private
 */

Lumina::RHI::FRHIBuffer::FRHIBuffer ( const UInt32 InId,
                                      const USize InSize,
                                      const ERHIBufferUsage InUsage,
                                      const Bool bInPersistent,
                                      const Bool bInCoherent ) noexcept
    : Id( InId ), Size( InSize ), Usage( InUsage ), bPersistent( bInPersistent ), bCoherent( bInCoherent )
{
    /* */
}

/**
 * Public
 */

Lumina::RHI::FRHIBuffer::FRHIBuffer ( FRHIBuffer &&InOther ) noexcept
    : Id( std::exchange( InOther.Id, 0U ) ), Size( std::exchange( InOther.Size, 0U ) ),
      Usage( std::exchange( InOther.Usage, ERHIBufferUsage::None ) ),
      MappedMemory( std::exchange( InOther.MappedMemory, nullptr ) ), bPersistent( std::exchange( InOther.bPersistent, false ) ),
      bCoherent( std::exchange( InOther.bCoherent, false ) )
{
    /* */
}

Lumina::RHI::FRHIBuffer &Lumina::RHI::FRHIBuffer::operator=( FRHIBuffer &&InOther ) noexcept
{
    if ( this != &InOther )
    {
        Reset();
        Id           = std::exchange( InOther.Id, 0U );
        Size         = std::exchange( InOther.Size, 0U );
        Usage        = std::exchange( InOther.Usage, ERHIBufferUsage::None );
        MappedMemory = std::exchange( InOther.MappedMemory, nullptr );
        bPersistent  = std::exchange( InOther.bPersistent, false );
        bCoherent    = std::exchange( InOther.bCoherent, false );
    }
    return *this;
}

Lumina::RHI::FRHIBuffer::~FRHIBuffer () noexcept
{
    Reset();
}

Lumina::RHI::FRHIExpected Lumina::RHI::FRHIBuffer::Write ( const USize InOffset, const TSpan<const Byte> InData ) const noexcept
{
    if ( not IsValid() or InOffset > Size or InData.size() > Size - InOffset )
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::InvalidArgument, "Buffer write is out of bounds" ) );
    }
    if ( not HasAnyFlags( Usage, ERHIBufferUsage::Dynamic ) )
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::InvalidArgument, "Buffer was not created as Dynamic" ) );
    }
    if ( MappedMemory != nullptr )
    {
        return MakeUnexpected(
            Internal::MakeError( ERHIErrorCode::InvalidArgument, "Cannot call Write while the buffer is mapped" ) );
    }
    if ( not InData.empty() )
    {
        Internal::NamedBufferSubData( Id, static_cast<GLintptr>( InOffset ), static_cast<GLsizeiptr>( InData.size() ),
                                      InData.data() );
    }
    return {};
}

Lumina::RHI::FRHIExpected Lumina::RHI::FRHIBuffer::Read ( const USize InOffset, const TSpan<Byte> OutData ) const noexcept
{
    if ( not IsValid() or InOffset > Size or OutData.size() > Size - InOffset )
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::InvalidArgument, "Buffer read is out of bounds" ) );
    }
    if ( not OutData.empty() )
    {
        Internal::GetNamedBufferSubData( Id, static_cast<GLintptr>( InOffset ), static_cast<GLsizeiptr>( OutData.size() ),
                                         OutData.data() );
    }
    return {};
}

Lumina::TExpected<Lumina::TSpan<Lumina::Byte>, Lumina::RHI::FRHIError>
Lumina::RHI::FRHIBuffer::Map ( const USize InOffset, const USize InSize, const ERHIMapAccess InAccess ) noexcept
{
    if ( not IsValid() or InSize == 0U or InOffset > Size or InSize > Size - InOffset or MappedMemory != nullptr ) [[unlikely]]
    {
        return MakeUnexpected(
            Internal::MakeError( ERHIErrorCode::InvalidArgument, "Invalid buffer mapping range or buffer already mapped" ) );
    }

    GLbitfield Flags = 0U;
    if ( InAccess != ERHIMapAccess::Write )
    {
        if ( not HasAnyFlags( Usage, ERHIBufferUsage::Readback ) ) [[unlikely]]
        {
            return MakeUnexpected(
                Internal::MakeError( ERHIErrorCode::InvalidArgument, "Read mapping requires Readback usage" ) );
        }
        Flags |= GL_MAP_READ_BIT;
    }
    if ( InAccess != ERHIMapAccess::Read )
    {
        if ( not HasAnyFlags( Usage, ERHIBufferUsage::Dynamic ) ) [[unlikely]]
        {
            return MakeUnexpected(
                Internal::MakeError( ERHIErrorCode::InvalidArgument, "Write mapping requires Dynamic usage" ) );
        }
        Flags |= GL_MAP_WRITE_BIT;
    }
    if ( bPersistent )
    {
        Flags |= GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    }
    else if ( InAccess != ERHIMapAccess::Read )
    {
        Flags |= GL_MAP_INVALIDATE_RANGE_BIT;
    }

    MappedMemory =
        Internal::MapNamedBufferRange( Id, static_cast<GLintptr>( InOffset ), static_cast<GLsizeiptr>( InSize ), Flags );
    if ( MappedMemory == nullptr ) [[unlikely]]
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::BackendError, "glMapNamedBufferRange failed" ) );
    }
    return TSpan<Byte>( static_cast<Byte *>( MappedMemory ), InSize );
}

void Lumina::RHI::FRHIBuffer::Unmap () noexcept
{
    if ( MappedMemory != nullptr ) [[likely]]
    {
        Internal::UnmapNamedBuffer( Id );
        MappedMemory = nullptr;
    }
}

Lumina::RHI::FRHIExpected Lumina::RHI::FRHIBuffer::CopyTo ( FRHIBuffer &InDestination,
                                                            const USize InSourceOffset,
                                                            const USize InDestinationOffset,
                                                            const USize InSize ) const noexcept
{
    if ( not IsValid() or not InDestination.IsValid() or InSourceOffset > Size or InSize > Size - InSourceOffset or
         InDestinationOffset > InDestination.Size or InSize > InDestination.Size - InDestinationOffset )
    {
        return MakeUnexpected( Internal::MakeError( ERHIErrorCode::InvalidArgument, "Buffer copy is out of bounds" ) );
    }
    if ( this == &InDestination and InSourceOffset < InDestinationOffset + InSize and
         InDestinationOffset < InSourceOffset + InSize )
    {
        return MakeUnexpected(
            Internal::MakeError( ERHIErrorCode::InvalidArgument, "Overlapping copies within one buffer are unsupported" ) );
    }
    if ( InSize > 0U )
    {
        Internal::CopyNamedBufferSubData( Id, InDestination.Id, static_cast<GLintptr>( InSourceOffset ),
                                          static_cast<GLintptr>( InDestinationOffset ), static_cast<GLsizeiptr>( InSize ) );
    }
    return {};
}

Lumina::Bool Lumina::RHI::FRHIBuffer::IsValid () const noexcept
{
    return Id != 0U;
}

Lumina::USize Lumina::RHI::FRHIBuffer::GetSize () const noexcept
{
    return Size;
}

Lumina::RHI::ERHIBufferUsage Lumina::RHI::FRHIBuffer::GetUsage () const noexcept
{
    return Usage;
}

void Lumina::RHI::FRHIBuffer::Reset () noexcept
{
    if ( MappedMemory != nullptr )
    {
        Unmap();
    }
    if ( Id != 0U )
    {
        Internal::DeleteBuffers( 1, &Id );
    }
    Id          = 0U;
    Size        = 0U;
    Usage       = ERHIBufferUsage::None;
    bPersistent = false;
    bCoherent   = false;
}
