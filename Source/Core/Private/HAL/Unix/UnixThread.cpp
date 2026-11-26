/**
 * @file UnixThread.cpp
 * @brief Implementation of FUnixThread.
 */

#include <Core/CoreDefinitions.hpp>

#if defined( LUMEN_PLATFORM_UNIX )

    #include <HAL/Unix/UnixThread.hpp>

    #include <limits>
    #include <sched.h>
    #include <unistd.h>
    #include <utility>

Lumina::HAL::FUnixThread::FId::FId ( pthread_t InId ) noexcept : Value( InId ), bValid( true )
{
}

Lumina::Bool Lumina::HAL::FUnixThread::FId::IsValid () const noexcept
{
    return bValid;
}

Lumina::Bool Lumina::HAL::FUnixThread::FId::operator==( const FId &InOther ) const noexcept
{
    return bValid == InOther.bValid && ( not bValid || pthread_equal( Value, InOther.Value ) != 0 );
}

Lumina::HAL::FUnixThread::FUnixThread ( FUnixThread &&InOther ) noexcept : Thread( InOther.Thread ), bJoinable( InOther.bJoinable )
{
    InOther.Thread    = pthread_t{};
    InOther.bJoinable = false;
}

Lumina::HAL::FUnixThread &Lumina::HAL::FUnixThread::operator=( FUnixThread &&InOther ) noexcept
{
    if ( this != &InOther )
    {
        Reset();
        Thread            = InOther.Thread;
        bJoinable         = InOther.bJoinable;
        InOther.Thread    = pthread_t{};
        InOther.bJoinable = false;
    }
    return *this;
}

Lumina::HAL::FUnixThread::~FUnixThread () noexcept
{
    Reset();
}

Lumina::Bool Lumina::HAL::FUnixThread::Start ( FEntryPoint InEntryPoint, void *InData ) noexcept
{
    if ( IsJoinable() )
    {
        return false;
    }

    pthread_t NewThread{};
    if ( pthread_create( &NewThread, nullptr, InEntryPoint, InData ) != 0 )
    {
        return false;
    }

    Thread    = NewThread;
    bJoinable = true;
    return true;
}

void Lumina::HAL::FUnixThread::Join () noexcept
{
    if ( not IsJoinable() )
    {
        return;
    }
    if ( pthread_equal( pthread_self(), Thread ) != 0 )
    {
        Detach();
        return;
    }
    if ( pthread_join( Thread, nullptr ) == 0 )
    {
        Thread    = pthread_t{};
        bJoinable = false;
    }
}

void Lumina::HAL::FUnixThread::Detach () noexcept
{
    if ( IsJoinable() && pthread_detach( Thread ) == 0 )
    {
        Thread    = pthread_t{};
        bJoinable = false;
    }
}

void Lumina::HAL::FUnixThread::Swap ( FUnixThread &InOther ) noexcept
{
    std::swap( Thread, InOther.Thread );
    std::swap( bJoinable, InOther.bJoinable );
}

Lumina::Bool Lumina::HAL::FUnixThread::IsJoinable () const noexcept
{
    return bJoinable;
}

Lumina::HAL::FUnixThread::FId Lumina::HAL::FUnixThread::GetId () const noexcept
{
    return IsJoinable() ? FId( Thread ) : FId();
}

Lumina::HAL::FUnixThread::FNativeHandle Lumina::HAL::FUnixThread::GetNativeHandle () const noexcept
{
    return Thread;
}

Lumina::HAL::FUnixThread::FId Lumina::HAL::FUnixThread::GetCurrentId () noexcept
{
    return FId( pthread_self() );
}

Lumina::UInt32 Lumina::HAL::FUnixThread::HardwareConcurrency () noexcept
{
    const long Count = sysconf( _SC_NPROCESSORS_ONLN );
    if ( Count <= 0 )
    {
        return 0U;
    }

    constexpr UInt64 Maximum = std::numeric_limits<UInt32>::max();
    const UInt64 NativeCount = static_cast<UInt64>( Count );
    return NativeCount > Maximum ? std::numeric_limits<UInt32>::max() : static_cast<UInt32>( NativeCount );
}

void Lumina::HAL::FUnixThread::Yield () noexcept
{
    static_cast<void>( sched_yield() );
}

void Lumina::HAL::FUnixThread::Reset () noexcept
{
    if ( IsJoinable() )
    {
        static_cast<void>( pthread_detach( Thread ) );
    }
    Thread    = pthread_t{};
    bJoinable = false;
}

#endif
