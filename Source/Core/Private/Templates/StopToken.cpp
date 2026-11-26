/**
 * @file StopToken.cpp
 * @brief Implementation of cooperative cancellation primitives.
 */

#include <Templates/StopToken.hpp>

#include <HAL/PlatformMutex.hpp>

#include <new>
#include <utility>

/**
 * Private
 */

/**
 * @class FStopState
 * @brief Shared stop state: a stop flag plus an intrusive list of callback nodes.
 */
class Lumina::Internal::FStopState final
{
public:

    void AddReference () noexcept
    {
        ReferenceCount.fetch_add( 1U, std::memory_order::relaxed );
    }

    void ReleaseReference () noexcept
    {
        if ( ReferenceCount.fetch_sub( 1U, std::memory_order::acq_rel ) == 1U )
        {
            delete this;
        }
    }

    void RegisterCallback ( FCallbackNode &InCallback ) noexcept
    {
        Mutex.Lock();
        if ( not bStopRequested.load( std::memory_order::acquire ) )
        {
            InCallback.AddReference();
            InCallback.Next = Callbacks;
            if ( Callbacks != nullptr )
            {
                Callbacks->Previous = &InCallback;
            }
            Callbacks = &InCallback;
            InCallback.Status.store( ECallbackStatus::Registered, std::memory_order::release );
            Mutex.Unlock();
            return;
        }

        /**
         * INFO: Stop was already requested: run the callback inline, but through the same status
         *       machinery as the deferred path. This publishes ExecutingThread and lets a concurrent
         *       handle destruction wait for completion instead of freeing the node mid-execution.
         *       The extra reference plays the role of the list reference for this execution.
         */
        InCallback.AddReference();
        InCallback.Status.store( ECallbackStatus::Pending, std::memory_order::release );
        Mutex.Unlock();

        /** INFO: The callback may destroy the last token/source handle, so keep the state alive. */
        AddReference();
        ExecuteCallback( InCallback );
        InCallback.ReleaseReference();
        ReleaseReference();
    }

    void UnregisterCallback ( FCallbackNode &InCallback ) noexcept
    {
        Mutex.Lock();
        ECallbackStatus Observed = InCallback.Status.load( std::memory_order::acquire );
        if ( Observed == ECallbackStatus::Registered )
        {
            if ( InCallback.Previous != nullptr )
            {
                InCallback.Previous->Next = InCallback.Next;
            }
            else
            {
                Callbacks = InCallback.Next;
            }
            if ( InCallback.Next != nullptr )
            {
                InCallback.Next->Previous = InCallback.Previous;
            }
            InCallback.Status.store( ECallbackStatus::Finished, std::memory_order::release );
            Mutex.Unlock();

            /**
             * INFO: Unlinked before any stop request could detach the node: no executor can ever
             *       touch it again, so there is nothing to wait for.
             */
            InCallback.ReleaseReference();
            return;
        }
        Mutex.Unlock();

        if ( Observed == ECallbackStatus::Pending )
        {
            /**
             * INFO: Detached by RequestStop but not executing yet: try to cancel it. On success the
             *       executor's CAS fails and the executor releases the list reference itself.
             *       On failure Observed is updated to the executor's state (Executing or Finished).
             */
            if ( InCallback.Status.compare_exchange_strong( Observed, ECallbackStatus::Finished, std::memory_order::acq_rel,
                                                            std::memory_order::acquire ) )
            {
                return;
            }
        }

        while ( Observed == ECallbackStatus::Executing )
        {
            /** INFO: Self-destruction from inside the running callback must not wait on itself. */
            if ( InCallback.ExecutingThread == HAL::FPlatformThread::GetCurrentId() )
            {
                return;
            }
            InCallback.Status.wait( Observed, std::memory_order::acquire );
            Observed = InCallback.Status.load( std::memory_order::acquire );
        }
    }

    [[nodiscard]] Bool RequestStop () noexcept
    {
        if ( bStopRequested.exchange( true, std::memory_order::acq_rel ) )
        {
            return false;
        }

        /**
         * INFO: Callbacks may destroy the last source/token handle, including the very source this
         *       method was called on. Hold a local reference so the state outlives the walk.
         */
        AddReference();

        Mutex.Lock();
        FCallbackNode *Callback = Callbacks;
        Callbacks               = nullptr;
        for ( FCallbackNode *Current = Callback; Current != nullptr; Current = Current->Next )
        {
            Current->Previous = nullptr;
            Current->Status.store( ECallbackStatus::Pending, std::memory_order::release );
        }
        Mutex.Unlock();

        while ( Callback != nullptr )
        {
            FCallbackNode *Next = Callback->Next;
            ExecuteCallback( *Callback );
            Callback->ReleaseReference();
            Callback = Next;
        }

        ReleaseReference();
        return true;
    }

private:

    static void ExecuteCallback ( FCallbackNode &InCallback ) noexcept
    {
        /**
         * INFO: ExecutingThread is written before the CAS so the release side of the CAS publishes
         *       it: a waiter that acquire-loads Executing is guaranteed to see the thread id.
         *       There is exactly one executor per node, so a failed CAS cannot race with a reader.
         */
        InCallback.ExecutingThread = HAL::FPlatformThread::GetCurrentId();

        ECallbackStatus Expected = ECallbackStatus::Pending;
        if ( not InCallback.Status.compare_exchange_strong( Expected, ECallbackStatus::Executing, std::memory_order::acq_rel,
                                                            std::memory_order::acquire ) )
        {
            /** INFO: UnregisterCallback cancelled the node first; it must not execute. */
            return;
        }

        InCallback.Execute();
        InCallback.Status.store( ECallbackStatus::Finished, std::memory_order::release );
        InCallback.Status.notify_all();
    }

private:

    TAtomic<UInt32> ReferenceCount{ 1U };
    TAtomic<Bool> bStopRequested{ false };
    HAL::FPlatformMutex Mutex;
    FCallbackNode *Callbacks = nullptr;

    friend class Lumina::FStopSource;
    friend class Lumina::FStopToken;
};

Lumina::Internal::FCallbackNode::FCallbackNode () noexcept  = default;
Lumina::Internal::FCallbackNode::~FCallbackNode () noexcept = default;

void Lumina::Internal::FCallbackNode::AddReference () noexcept
{
    ReferenceCount.fetch_add( 1U, std::memory_order::relaxed );
}

void Lumina::Internal::FCallbackNode::ReleaseReference () noexcept
{
    if ( ReferenceCount.fetch_sub( 1U, std::memory_order::acq_rel ) == 1U )
    {
        delete this;
    }
}

Lumina::FStopToken::FStopToken ( const FStopToken &InOther ) noexcept : State( InOther.State )
{
    if ( State != nullptr )
    {
        State->AddReference();
    }
}

Lumina::FStopToken::FStopToken ( FStopToken &&InOther ) noexcept : State( std::exchange( InOther.State, nullptr ) )
{
    /* */
}

Lumina::FStopToken::~FStopToken () noexcept
{
    if ( State != nullptr )
    {
        State->ReleaseReference();
    }
}

Lumina::FStopToken &Lumina::FStopToken::operator=( const FStopToken &InOther ) noexcept
{
    if ( this != &InOther )
    {
        if ( InOther.State != nullptr )
        {
            InOther.State->AddReference();
        }
        if ( State != nullptr )
        {
            State->ReleaseReference();
        }
        State = InOther.State;
    }
    return *this;
}

Lumina::FStopToken &Lumina::FStopToken::operator=( FStopToken &&InOther ) noexcept
{
    if ( this != &InOther )
    {
        if ( State != nullptr )
        {
            State->ReleaseReference();
        }
        State = std::exchange( InOther.State, nullptr );
    }
    return *this;
}

Lumina::Bool Lumina::FStopToken::StopPossible () const noexcept
{
    return State != nullptr;
}

Lumina::Bool Lumina::FStopToken::StopRequested () const noexcept
{
    return State != nullptr and State->bStopRequested.load( std::memory_order::acquire );
}

void Lumina::FStopToken::Swap ( FStopToken &InOther ) noexcept
{
    std::swap( State, InOther.State );
}

Lumina::Bool Lumina::FStopToken::operator==( const FStopToken &InOther ) const noexcept
{
    return State == InOther.State;
}

Lumina::FStopToken::FStopToken ( Internal::FStopState *InState ) noexcept : State( InState )
{
    if ( State != nullptr )
    {
        State->AddReference();
    }
}

Lumina::FStopSource::FStopSource () noexcept : State( new ( std::nothrow ) Internal::FStopState() )
{
}

Lumina::FStopSource::FStopSource ( NullPtrType ) noexcept
{
}

Lumina::FStopSource::FStopSource ( const FStopSource &InOther ) noexcept : State( InOther.State )
{
    if ( State != nullptr )
    {
        State->AddReference();
    }
}

Lumina::FStopSource::FStopSource ( FStopSource &&InOther ) noexcept : State( std::exchange( InOther.State, nullptr ) )
{
}

Lumina::FStopSource::~FStopSource () noexcept
{
    if ( State != nullptr )
    {
        State->ReleaseReference();
    }
}

Lumina::FStopSource &Lumina::FStopSource::operator=( const FStopSource &InOther ) noexcept
{
    if ( this != &InOther )
    {
        if ( InOther.State != nullptr )
        {
            InOther.State->AddReference();
        }
        if ( State != nullptr )
        {
            State->ReleaseReference();
        }
        State = InOther.State;
    }
    return *this;
}

Lumina::FStopSource &Lumina::FStopSource::operator=( FStopSource &&InOther ) noexcept
{
    if ( this != &InOther )
    {
        if ( State != nullptr )
        {
            State->ReleaseReference();
        }
        State = std::exchange( InOther.State, nullptr );
    }
    return *this;
}

Lumina::FStopToken Lumina::FStopSource::GetToken () const noexcept
{
    return FStopToken( State );
}

Lumina::Bool Lumina::FStopSource::RequestStop () noexcept
{
    return State != nullptr and State->RequestStop();
}

Lumina::Bool Lumina::FStopSource::StopPossible () const noexcept
{
    return State != nullptr;
}

Lumina::Bool Lumina::FStopSource::StopRequested () const noexcept
{
    return State != nullptr and State->bStopRequested.load( std::memory_order::acquire );
}

void Lumina::FStopSource::Swap ( FStopSource &InOther ) noexcept
{
    std::swap( State, InOther.State );
}

Lumina::Bool Lumina::FStopSource::operator==( const FStopSource &InOther ) const noexcept
{
    return State == InOther.State;
}

void Lumina::FStopCallback::Register ( const FStopToken &InStopToken ) noexcept
{
    if ( Callback != nullptr )
    {
        State = InStopToken.State;
        State->AddReference();
        State->RegisterCallback( *Callback );
    }
}

Lumina::FStopCallback::~FStopCallback () noexcept
{
    Reset();
}

void Lumina::FStopCallback::Reset () noexcept
{
    Internal::FCallbackNode *CallbackToRemove = std::exchange( Callback, nullptr );
    Internal::FStopState *CallbackState       = std::exchange( State, nullptr );

    if ( CallbackState != nullptr )
    {
        CallbackState->UnregisterCallback( *CallbackToRemove );
        CallbackToRemove->ReleaseReference();
        CallbackState->ReleaseReference();
    }
}
