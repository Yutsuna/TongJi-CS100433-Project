/**
 * @file Signal.cpp
 * @brief Implementation of signal handling functions (POSIX and Windows).
 */

#include <Core/CoreDefinitions.hpp>

#include <Algo/ForEach.hpp>

#include <HAL/LockGuard.hpp>
#include <HAL/PlatformSignal.hpp>

#include <bit>
#include <csignal>

#if not defined( LUMEN_PLATFROM_WINDOWS )
    #include <signal.h> /** NOLINT(modernize-deprecated-headers, hicpp-deprecated-headers) */
#endif

static_assert( Lumina::TAtomic<Lumina::UInt64>::is_always_lock_free, "PendingSignals must be lock-free to be signal-safe." );

/**
 * Private
 */

namespace
{

/** @brief Highest supported signal number + 1 ( one bit per signal in a 64-bit mask ). */
constexpr Lumina::Int32 GMaxSignals = 64;

constexpr Lumina::UInt64 SignalBit ( const Lumina::Int32 InSignal ) noexcept
{
    return static_cast<Lumina::UInt64>( 1U ) << static_cast<Lumina::UInt8>( InSignal );
}

constexpr Lumina::Int32 PopLowestBit ( Lumina::UInt64 &InOutMask ) noexcept
{
    if ( InOutMask == 0U )
    {
        return -1;
    }
    const Lumina::Int32 Index = static_cast<Lumina::Int32>( std::countr_zero( InOutMask ) );

    InOutMask &= InOutMask - 1U;
    return Index;
}

using FOSHandler = void ( * )( Lumina::Int32 );

#if defined( LUMEN_PLATFORM_UNIX )
void InstallSignalAction ( const Lumina::Int32 InOSCode, void ( *InHandler )( Lumina::Int32 ), const Lumina::Int32 InFlags ) noexcept
{
    struct sigaction Action = {};
    Action.sa_handler       = InHandler;
    sigemptyset( &Action.sa_mask );
    Action.sa_flags = InFlags;
    static_cast<void>( sigaction( InOSCode, &Action, nullptr ) );
}
#endif

/** @brief Installs InHandler as the OS-level handler of InOSCode. */
void InstallOSHandler ( const Lumina::Int32 InOSCode, const FOSHandler InHandler ) noexcept
{
#if defined( LUMEN_PLATFROM_WINDOWS )
    static_cast<void>( std::signal( InOSCode, InHandler ) );

#elif defined( LUMEN_PLATFORM_UNIX )
    InstallSignalAction( InOSCode, InHandler, SA_RESTART );
#else
    #error "PlatformSignal.cpp: InstallOSHandler is only available on Windows or Unix platforms."
#endif
}

/** @brief Restores the default OS behaviour of InOSCode. */
void RestoreDefaultOSHandler ( const Lumina::Int32 InOSCode ) noexcept
{
#if defined( LUMEN_PLATFROM_WINDOWS )
    static_cast<void>( std::signal( InOSCode, SIG_DFL ) );

#elif defined( LUMEN_PLATFORM_UNIX )
    InstallSignalAction( InOSCode, SIG_DFL, 0 );
#else
    #error "PlatformSignal.cpp: RestoreDefaultOSHandler is only available on Windows or Unix platforms."
#endif
}

} // namespace

/**
 * Static members
 */

Lumina::HAL::FPlatformSignal::FSignalHandlerMap Lumina::HAL::FPlatformSignal::SignalListeners;
Lumina::HAL::FPlatformRecursiveSharedMutex Lumina::HAL::FPlatformSignal::ListenersMutex;
Lumina::TAtomic<Lumina::UInt64> Lumina::HAL::FPlatformSignal::PendingSignals{ 0U };

/**
 * Public
 */

void Lumina::HAL::FPlatformSignal::Bind ( const ESystemSignal::Type InSignal, const FSignalHandlerSignature InHandler )
{
    const Int32 OSCode = InSignal;

    const TLockGuard<HAL::FPlatformRecursiveSharedMutex> Guard( ListenersMutex );
    FSignalHandlerVector &Listeners = SignalListeners[InSignal];

    if ( Listeners.empty() )
    {
        InstallOSHandler( OSCode, &FPlatformSignal::HandleOSSignal );
    }

    Listeners.push_back( InHandler );
}

void Lumina::HAL::FPlatformSignal::Raise ( const ESystemSignal::Type InSignal )
{
    const Int32 OSCode = InSignal;

    /**
     *  INFO: raise() runs the handler synchronously on the calling thread.
     *        The signal is recorded when it returns
     */
    std::raise( OSCode );

    Dispatch();
}

void Lumina::HAL::FPlatformSignal::Dispatch ()
{
    UInt64 Pending = PendingSignals.exchange( 0U, std::memory_order::acq_rel );

    for ( Int32 OSCode = PopLowestBit( Pending ); OSCode >= 0; OSCode = PopLowestBit( Pending ) )
    {
        const ESystemSignal::Type SignalType = static_cast<ESystemSignal::Type>( OSCode );

        Algo::ForEach( SnapshotHandlers( SignalType ), [SignalType] ( const FSignalHandlerSignature InHandler )
        {
            InHandler( SignalType );
        } );
    }
}

void Lumina::HAL::FPlatformSignal::Unbind ( const ESystemSignal::Type InSignal )
{
    const Int32 OSCode = InSignal;

    const TLockGuard<HAL::FPlatformRecursiveSharedMutex> Guard( ListenersMutex );

    if ( SignalListeners.contains( InSignal ) )
    {
        /**
         *  INFO: The async-signal-safe OS handler stays installed here on purpose:
         *        a later Raise() on this slot must be a safe no-op (pending bit set,
         *        then consumed by Dispatch() with no listeners), never a fallback to
         *        the process-killing default disposition. Reset() is what re-arms
         *        the OS defaults.
         */
        SignalListeners.erase( InSignal );
        PendingSignals.fetch_and( ~SignalBit( OSCode ), std::memory_order::acq_rel );
    }
}

void Lumina::HAL::FPlatformSignal::Reset ()
{
    const TLockGuard<HAL::FPlatformRecursiveSharedMutex> Guard( ListenersMutex );

    Algo::ForEach( SignalListeners, [] ( const FSignalHandlerMap::value_type &InEntry )
    {
        RestoreDefaultOSHandler( static_cast<Int32>( InEntry.first ) );
    } );

    SignalListeners.clear();
    SignalListeners.rehash( 0 );
    PendingSignals.store( 0U, std::memory_order::release );
}

/**
 * Private
 */

void Lumina::HAL::FPlatformSignal::HandleOSSignal ( const Int32 InSignal )
{
#if defined( LUMEN_PLATFROM_WINDOWS )
    /**
     * INFO: The CRT may reset the handler to SIG_DFL before calling it: re-arm so the next occurrence is caught too.
     */
    static_cast<void>( std::signal( InSignal, &FPlatformSignal::HandleOSSignal ) );
#endif

    /** INFO: async-signal-safe: a single lock-free atomic RMW. */
    if ( InSignal > 0 and InSignal < GMaxSignals )
    {
        PendingSignals.fetch_or( SignalBit( InSignal ), std::memory_order::release );
    }
}

Lumina::HAL::FPlatformSignal::FSignalHandlerVector
Lumina::HAL::FPlatformSignal::SnapshotHandlers ( const ESystemSignal::Type InSignal )
{
    const TLockGuard<FPlatformRecursiveSharedMutex> Guard( ListenersMutex );
    FSignalHandlerVector Handlers;

    if ( const auto It = SignalListeners.find( InSignal ); It != SignalListeners.end() )
    {
        Handlers = It->second;
    }

    return Handlers;
}
