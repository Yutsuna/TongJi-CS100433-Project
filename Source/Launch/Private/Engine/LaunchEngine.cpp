/**
 * @file LaunchEngine.cpp
 * @brief Launches the Lumina Engine
 */

#include <Core/CoreDefinitions.hpp>

#include <Engine/LaunchEngine.hpp>
#include <Engine/LaunchEngineLoop.hpp>
#include <Engine/LaunchProcessGuard.hpp>

#include <Logging/Logger.hpp>

/**
 * Private
 */

namespace
{

LUMINA_LOG_DEFINE_CATEGORY( LogLaunch, "Launch" )

Lumina::Launch::TLaunchResult<Lumina::Int32> ReportFatalError ( const Lumina::Launch::FLaunchError &InError ) noexcept
{
    if ( InError.Code == Lumina::Launch::ELaunchErrorCode::None )
    {
        return LUMINA_EXIT_SUCCESS;
    }
    LUMINA_LOG_ERROR( LogLaunch, "Fatal error during {}: {:.{}} (code {})", ToCString( InError.Stage ), InError.Message.data(),
                      InError.Message.size(), InError.ToExitCode() );

    return InError.ToExitCode();
}

constexpr Lumina::Int32 ToExitCode ( const Lumina::Launch::FRunSummary LUMINA_UNUSED &RunSummary ) noexcept
{
    return LUMINA_EXIT_SUCCESS;
}

} // namespace

/**
 * Public
 */

Lumina::Int32 Lumina::Launch::GuardedMain ( const Int32 InArgc, const AnsiChar *const *InArgv ) noexcept
{
    const FLaunchProcessGuard ProcessGuard;
    const Int32 ExitCode = FLaunchEngineLoop::PreInit( FCommandLine( InArgc, InArgv ) )
                               .AndThen( &FLaunchEngineLoop::Init )
                               .AndThen( &FLaunchEngineLoop::Run )
                               .Transform( &ToExitCode )
                               .OrElse( &ReportFatalError )
                               .ValueOr( LUMINA_EXIT_FAILURE );

    return ExitCode;
}
