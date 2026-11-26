/**
 * @file WindowsThread.cpp
 * @brief Implementation of FWindowsThread.
 */

#include <Core/CoreDefinitions.hpp>

#if defined( LUMEN_PLATFORM_WINDOWS )

    #include <HAL/Windows/WindowsThread.hpp>

    #include <utility>

Lumina::HAL::FWindowsThread::FId::FId ( DWORD InId ) noexcept : Value( InId )
{
    /* */
}

Lumina::Bool Lumina::HAL::FWindowsThread::FId::IsValid () const noexcept
{
    return Value != 0;
}

Lumina::Bool Lumina::HAL::FWindowsThread::FId::operator==( const FId &InOther ) const noexcept
{
    return Value == InOther.Value;
}

Lumina::HAL::FWindowsThread::FWindowsThread ( FWindowsThread &&InOther ) noexcept
    : Thread( InOther.Thread ), ThreadId( InOther.ThreadId )
{
    InOther.Thread   = nullptr;
    InOther.ThreadId = 0;
}

Lumina::HAL::FWindowsThread &Lumina::HAL::FWindowsThread::operator=( FWindowsThread &&InOther ) noexcept
{
    if ( this != &InOther )
    {
        Reset();
        Thread           = InOther.Thread;
        ThreadId         = InOther.ThreadId;
        InOther.Thread   = nullptr;
        InOther.ThreadId = 0;
    }
    return *this;
}

Lumina::HAL::FWindowsThread::~FWindowsThread () noexcept
{
    Reset();
}

Lumina::Bool Lumina::HAL::FWindowsThread::Start ( FEntryPoint InEntryPoint, void *InData ) noexcept
{
    if ( IsJoinable() )
    {
        return false;
    }

    DWORD NewThreadId = 0;
    HANDLE NewThread  = CreateThread( nullptr, 0, InEntryPoint, InData, 0, &NewThreadId );
    if ( NewThread == nullptr )
    {
        return false;
    }

    Thread   = NewThread;
    ThreadId = NewThreadId;
    return true;
}

void Lumina::HAL::FWindowsThread::Join () noexcept
{
    if ( not IsJoinable() )
    {
        return;
    }
    if ( GetCurrentThreadId() == ThreadId )
    {
        Detach();
        return;
    }
    if ( WaitForSingleObject( Thread, INFINITE ) == WAIT_OBJECT_0 )
    {
        Reset();
    }
}

void Lumina::HAL::FWindowsThread::Detach () noexcept
{
    Reset();
}

void Lumina::HAL::FWindowsThread::Swap ( FWindowsThread &InOther ) noexcept
{
    std::swap( Thread, InOther.Thread );
    std::swap( ThreadId, InOther.ThreadId );
}

Lumina::Bool Lumina::HAL::FWindowsThread::IsJoinable () const noexcept
{
    return Thread != nullptr;
}

Lumina::HAL::FWindowsThread::FId Lumina::HAL::FWindowsThread::GetId () const noexcept
{
    return FId( ThreadId );
}

Lumina::HAL::FWindowsThread::FNativeHandle Lumina::HAL::FWindowsThread::GetNativeHandle () const noexcept
{
    return Thread;
}

Lumina::HAL::FWindowsThread::FId Lumina::HAL::FWindowsThread::GetCurrentId () noexcept
{
    return FId( ::GetCurrentThreadId() );
}

Lumina::UInt32 Lumina::HAL::FWindowsThread::HardwareConcurrency () noexcept
{
    SYSTEM_INFO SystemInfo{};
    GetSystemInfo( &SystemInfo );
    return static_cast<UInt32>( SystemInfo.dwNumberOfProcessors );
}

void Lumina::HAL::FWindowsThread::Yield () noexcept
{
    static_cast<void>( SwitchToThread() );
}

void Lumina::HAL::FWindowsThread::Reset () noexcept
{
    if ( Thread != nullptr )
    {
        static_cast<void>( CloseHandle( Thread ) );
    }
    Thread   = nullptr;
    ThreadId = 0;
}

#endif
