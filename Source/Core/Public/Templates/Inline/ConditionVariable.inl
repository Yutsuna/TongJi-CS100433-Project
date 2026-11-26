/**
 * @file ConditionVariable.inl
 * @brief Inline implementation of FConditionVariable.
 */

#pragma once

#include <Templates/ConditionVariable.hpp>

#include <HAL/PlatformFutex.hpp>
#include <HAL/PlatformTime.hpp>

/**
 * Notify
 */

LUMINA_FORCE_INLINE void Lumina::FConditionVariable::NotifyOne () noexcept
{
    /**
     * INFO: Relaxed is enough: the caller's mutex orders the state change, and the futex word only
     *       has to differ from what a waiter sampled
     */
    Sequence.fetch_add( 1U, std::memory_order::relaxed );
    HAL::FPlatformFutex::WakeOne( Sequence );
}

LUMINA_FORCE_INLINE void Lumina::FConditionVariable::NotifyAll () noexcept
{
    Sequence.fetch_add( 1U, std::memory_order::relaxed );
    HAL::FPlatformFutex::WakeAll( Sequence );
}

/**
 * Wait
 */

template <Lumina::Concepts::CIsLockable TLockType> void Lumina::FConditionVariable::Wait ( TLockType &InLock ) noexcept
{
    /**
     * INFO: The sequence is sampled while InLock is still held. A notification issued after the unlock changes the
     *       counter, so the futex wait returns immediately instead of sleeping: no lost wake-up.
     */
    const UInt32 Observed = Sequence.load( std::memory_order::relaxed );

    InLock.Unlock();
    HAL::FPlatformFutex::Wait( Sequence, Observed );
    InLock.Lock();
}

template <Lumina::Concepts::CIsLockable TLockType, class TPredicate>
void Lumina::FConditionVariable::Wait ( TLockType &InLock, TPredicate &&InPredicate ) noexcept
{
    while ( not static_cast<Bool>( InPredicate() ) )
    {
        Wait( InLock );
    }
}

/**
 * WaitFor
 */

template <Lumina::Concepts::CIsLockable TLockType>
Lumina::Bool Lumina::FConditionVariable::WaitFor ( TLockType &InLock, Float64 InTimeoutSeconds ) noexcept
{
    const UInt32 Observed = Sequence.load( std::memory_order::relaxed );

    InLock.Unlock();
    const Bool bNotTimedOut = HAL::FPlatformFutex::WaitFor( Sequence, Observed, InTimeoutSeconds );
    InLock.Lock();

    return bNotTimedOut;
}

template <Lumina::Concepts::CIsLockable TLockType, class TPredicate>
Lumina::Bool Lumina::FConditionVariable::WaitFor ( TLockType &InLock, Float64 InTimeoutSeconds, TPredicate &&InPredicate ) noexcept
{
    return WaitUntil( InLock, HAL::FPlatformTime::Seconds() + InTimeoutSeconds, std::forward<TPredicate>( InPredicate ) );
}

/**
 * WaitUntil
 */

template <Lumina::Concepts::CIsLockable TLockType>
Lumina::Bool Lumina::FConditionVariable::WaitUntil ( TLockType &InLock, Float64 InDeadlineSeconds ) noexcept
{
    /** INFO: Clamp expired deadlines to zero so the futex layer never receives a negative timeout. */
    const Float64 Remaining = InDeadlineSeconds - HAL::FPlatformTime::Seconds();
    return WaitFor( InLock, Remaining > 0.0 ? Remaining : 0.0 );
}

template <Lumina::Concepts::CIsLockable TLockType, class TPredicate>
Lumina::Bool
Lumina::FConditionVariable::WaitUntil ( TLockType &InLock, Float64 InDeadlineSeconds, TPredicate &&InPredicate ) noexcept
{
    while ( not static_cast<Bool>( InPredicate() ) )
    {
        if ( not WaitUntil( InLock, InDeadlineSeconds ) )
        {
            /** INFO: One last check: the condition may have become true right as the deadline expired */
            return static_cast<Bool>( InPredicate() );
        }
    }

    return true;
}
