/**
 * @file LaunchProcessGuard.cpp
 * @brief Implementation of Launch Process Guard
 */

#include <Engine/LaunchEngineLoop.hpp>
#include <Engine/LaunchProcessGuard.hpp>

#include <Core/CoreDefinitions.hpp>

#include <HAL/PlatformSignal.hpp>

#include <Logging/Logger.hpp>

namespace
{

/** @brief Handles Signals received */
template <typename TMessage> void OnSignal ( const Lumina::HAL::ESystemSignal::Type InSignal, const TMessage &InMessage ) noexcept
{
    if ( Lumina::Launch::FLaunchEngineLoop::RequestExit() )
    {
        Lumina::FLogger::Flush( InMessage );
        Lumina::HAL::FPlatformSignal::Unbind( InSignal );
        Lumina::HAL::FPlatformSignal::Raise( InSignal );
    }
}

/** @brief Handles Interrupt Signal */
void OnInterruptSignal ( const Lumina::HAL::ESystemSignal::Type InSignal ) noexcept
{
    OnSignal( InSignal, "\rInterrupt signal trapped: exiting gracefully..." LUMINA_NEWLINE );
}

/** @brief Handles Terminate Signal */
void OnTerminateSignal ( const Lumina::HAL::ESystemSignal::Type InSignal ) noexcept
{
    OnSignal( InSignal, "\rTerminate signal trapped: exiting gracefully..." LUMINA_NEWLINE );
}

/** @brief Handles Out Of Memory */
void OnOutOfMemory () noexcept
{
    Lumina::FLogger::Flush( "Fatal: Out of Memory" LUMINA_NEWLINE );
    std::_Exit( static_cast<Lumina::Int32>( Lumina::Launch::ELaunchErrorCode::OutOfMemory ) );
}

} // namespace

Lumina::Launch::FLaunchProcessGuard::FLaunchProcessGuard () noexcept
    : PreviousNewHandler( std::set_new_handler( &OnOutOfMemory ) )
{
    HAL::FPlatformSignal::Bind( HAL::ESystemSignal::Interrupt, &OnInterruptSignal );
    HAL::FPlatformSignal::Bind( HAL::ESystemSignal::Terminate, &OnTerminateSignal );

    FLogger::GetInstance().Initialize();
}

Lumina::Launch::FLaunchProcessGuard::~FLaunchProcessGuard () noexcept
{
    FLogger::GetInstance().Shutdown();

    HAL::FPlatformSignal::Unbind( HAL::ESystemSignal::Terminate );
    HAL::FPlatformSignal::Unbind( HAL::ESystemSignal::Interrupt );

    std::set_new_handler( PreviousNewHandler );
}
