/**
 * @file LockGuard.inl
 * @brief LockGuard inline implementation.
 */

#pragma once

#include <HAL/LockGuard.hpp>

#include <utility>

/**
 * TLockGuard
 */

template <class TMutexType> Lumina::HAL::TLockGuard<TMutexType>::TLockGuard( TMutexType &InMutex ) noexcept : Mutex( InMutex )
{
    Mutex.Lock();
}

template <class TMutexType> Lumina::HAL::TLockGuard<TMutexType>::TLockGuard::~TLockGuard() noexcept
{
    Mutex.Unlock();
}

/**
 * TSharedLock
 */

template <class TMutexType> Lumina::HAL::TSharedLock<TMutexType>::TSharedLock( TMutexType &InMutex ) noexcept : Mutex( InMutex )
{
    Mutex.LockShared();
}

template <class TMutexType> Lumina::HAL::TSharedLock<TMutexType>::TSharedLock::~TSharedLock() noexcept
{
    Mutex.UnlockShared();
}

/**
 * TUniqueLock
 */

template <class TMutexType> Lumina::HAL::TUniqueLock<TMutexType>::TUniqueLock( TMutexType &InMutex ) noexcept : Mutex( &InMutex )
{
    Lock();
}

template <class TMutexType>
Lumina::HAL::TUniqueLock<TMutexType>::TUniqueLock( TMutexType &InMutex, FDeferLock LUMEN_UNUSED InTag ) noexcept
    : Mutex( &InMutex )
{
    /** Empty */
}

template <class TMutexType>
Lumina::HAL::TUniqueLock<TMutexType>::TUniqueLock( TMutexType &InMutex, FTryToLock LUMEN_UNUSED InTag ) noexcept
    : Mutex( &InMutex )
{
    static_cast<void>( TryLock() );
}

template <class TMutexType>
Lumina::HAL::TUniqueLock<TMutexType>::TUniqueLock( TMutexType &InMutex, FAdoptLock LUMEN_UNUSED InTag ) noexcept
    : Mutex( &InMutex ), bIsLocked( true )
{
    /** Empty */
}

template <class TMutexType> Lumina::HAL::TUniqueLock<TMutexType>::TUniqueLock::~TUniqueLock() noexcept
{
    Unlock();
}

template <class TMutexType>
Lumina::HAL::TUniqueLock<TMutexType>::TUniqueLock( TUniqueLock<TMutexType> &&InOther ) noexcept
    : Mutex( std::exchange( InOther.Mutex, nullptr ) ), bIsLocked( std::exchange( InOther.bIsLocked, false ) )
{
    /** Empty */
}

template <class TMutexType>
Lumina::HAL::TUniqueLock<TMutexType> &Lumina::HAL::TUniqueLock<TMutexType>::operator=( TUniqueLock<TMutexType> &&InOther ) noexcept
{
    if ( this != &InOther )
    {
        Unlock();

        Mutex     = std::exchange( InOther.Mutex, nullptr );
        bIsLocked = std::exchange( InOther.bIsLocked, false );
    }

    return *this;
}

template <class TMutexType> void Lumina::HAL::TUniqueLock<TMutexType>::Lock () noexcept
{
    if ( Mutex and not bIsLocked )
    {
        Mutex->Lock();
        bIsLocked = true;
    }
}

template <class TMutexType> void Lumina::HAL::TUniqueLock<TMutexType>::Unlock () noexcept
{
    if ( Mutex and bIsLocked )
    {
        Mutex->Unlock();
        bIsLocked = false;
    }
}

template <class TMutexType> TMutexType *Lumina::HAL::TUniqueLock<TMutexType>::Release () noexcept
{
    bIsLocked = false;
    return std::exchange( Mutex, nullptr );
}

template <class TMutexType> Lumina::Bool Lumina::HAL::TUniqueLock<TMutexType>::TryLock () noexcept
{
    /** INFO: If already owned, do not touch the mutex or the state: a failing TryLock() on the mutex would clear bIsLocked and
     * leak the lock */
    if ( not( Mutex and not bIsLocked ) )
    {
        return false;
    }

    bIsLocked = Mutex->TryLock();

    return bIsLocked;
}

template <class TMutexType> Lumina::Bool Lumina::HAL::TUniqueLock<TMutexType>::IsLocked () const noexcept
{
    return bIsLocked;
}

template <class TMutexType> Lumina::HAL::TUniqueLock<TMutexType>::operator Lumina::Bool () const noexcept
{
    return IsLocked();
}
