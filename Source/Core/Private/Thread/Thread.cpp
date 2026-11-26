/**
 * @file Thread.cpp
 * @brief Implementation of FThread.
 */

#include <Thread/Thread.hpp>

#include <utility>

Lumina::FThread::FThread ( FThread &&InOther ) noexcept
    : StopSource( std::move( InOther.StopSource ) ), Thread( std::move( InOther.Thread ) )
{
}

Lumina::FThread::~FThread () noexcept
{
    static_cast<void>( RequestStop() );
    Join();
}

Lumina::FThread &Lumina::FThread::operator=( FThread &&InOther ) noexcept
{
    if ( this != &InOther )
    {
        static_cast<void>( RequestStop() );
        Join();
        StopSource = std::move( InOther.StopSource );
        Thread     = std::move( InOther.Thread );
    }
    return *this;
}

void Lumina::FThread::Join () noexcept
{
    Thread.Join();

    if ( not Thread.IsJoinable() )
    {
        StopSource = FStopSource( nullptr );
    }
}

void Lumina::FThread::Detach () noexcept
{
    Thread.Detach();

    if ( not Thread.IsJoinable() )
    {
        StopSource = FStopSource( nullptr );
    }
}

void Lumina::FThread::Swap ( FThread &InOther ) noexcept
{
    StopSource.Swap( InOther.StopSource );
    Thread.Swap( InOther.Thread );
}

Lumina::Bool Lumina::FThread::IsJoinable () const noexcept
{
    return Thread.IsJoinable();
}

Lumina::Bool Lumina::FThread::RequestStop () noexcept
{
    return StopSource.RequestStop();
}

Lumina::FStopSource Lumina::FThread::GetStopSource () const noexcept
{
    return StopSource;
}

Lumina::FStopToken Lumina::FThread::GetStopToken () const noexcept
{
    return StopSource.GetToken();
}

Lumina::FThread::FId Lumina::FThread::GetId () const noexcept
{
    return Thread.GetId();
}

Lumina::FThread::FNativeHandle Lumina::FThread::GetNativeHandle () const noexcept
{
    return Thread.GetNativeHandle();
}

Lumina::FThread::FId Lumina::FThread::GetCurrentId () noexcept
{
    return HAL::FPlatformThread::GetCurrentId();
}

Lumina::UInt32 Lumina::FThread::HardwareConcurrency () noexcept
{
    return HAL::FPlatformThread::HardwareConcurrency();
}

void Lumina::FThread::Yield () noexcept
{
    HAL::FPlatformThread::Yield();
}
