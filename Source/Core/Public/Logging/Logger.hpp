/**
 * @file Logger.hpp
 * @brief Definition of the Logger class
 */

#pragma once

#include <Logging/LoggingCategory.hpp>

#include <HAL/PlatformMutex.hpp>

#include <Thread/Thread.hpp>

#include <Templates/ConditionVariable.hpp>
#include <Templates/Queue.hpp>

namespace Lumina
{

/**
 * @class FLogger
 * @brief Asynchronous Logging
 */
class CORE_EXPORT FLogger
{
public:

    /** @brief Logs a message to the console */
    template <typename... Args>
    void TLog ( const FLogCategory &Category, ELogVerbosity::Type Verbosity, FStringView Format, Args &&...InArgs ) noexcept;

    /**
     * @brief Initializes the logger and starts the worker thread.
     * @note Until this is called (or if the thread cannot be created), messages are written synchronously.
     */
    void Initialize () noexcept;

    /**
     * @brief Shuts down the logger and flushes all pending log messages.
     * @note Thread-safe. After this call, messages are written synchronously.
     */
    void Shutdown () noexcept;

public:

    /** @brief Gets the singleton instance of the logger. */
    static FLogger &GetInstance () noexcept;

    /** @brief Flushes a message to the console immediately without enqueuing it. */
    static void Flush ( const AnsiChar *const String ) noexcept;
    static void Flush ( const FString &String ) noexcept;

public:

    struct FLogMessage
    {
        FLogCategory Category;
        ELogVerbosity::Type Verbosity;
        FString Message;
        Float64 Timestamp;
    };

private:

    void EnqueueLogMessage ( const FLogCategory &Category, const ELogVerbosity::Type Verbosity, FString &&Message );
    void FlushLogMessages ( const FStopToken &InStopToken );
    void CoutMessage ( const FLogMessage &LogMessage ) const noexcept;

private:

    FLogger () noexcept = default;
    ~FLogger ();

private:

    HAL::FPlatformMutex QueueMutex;      ///< Guards LogMessageQueue, WorkerThread and the writes to bIsAsync.
    HAL::FPlatformMutex ShutdownMutex;   ///< Serializes concurrent Shutdown() calls.
    FConditionVariable Condition;        ///< Manages synchronization for log message processing.
    TQueue<FLogMessage> LogMessageQueue; ///< Holds log messages waiting to be processed.
    FThread WorkerThread;                ///< Only access under QueueMutex.

    TAtomic<Bool> bIsAsync{ false };
};

} // namespace Lumina

#include "Inline/Logger.inl"

#define LUMINA_LOG( Category, Verbosity, Format, ... )                                                                            \
    Lumina::FLogger::GetInstance().TLog( Category, Verbosity, Format __VA_OPT__(, ) __VA_ARGS__ )

/**
 * @note The name is converted to FStringView explicitly: FLogCategory has both an FStringView and an FString
 *       constructor, so passing a string literal directly is ambiguous.
 */
#define LUMINA_LOG_DEFINE_CATEGORY( CategoryVar, CategoryName )                                                                   \
    static const ::Lumina::FLogCategory CategoryVar( static_cast<::Lumina::FStringView>( CategoryName ) );

#define LUMINA_LOG_FATAL( Category, Format, ... )                                                                                 \
    LUMINA_LOG( Category, Lumina::ELogVerbosity::Fatal, Format __VA_OPT__(, ) __VA_ARGS__ )
#define LUMINA_LOG_ERROR( Category, Format, ... )                                                                                 \
    LUMINA_LOG( Category, Lumina::ELogVerbosity::Error, Format __VA_OPT__(, ) __VA_ARGS__ )
#define LUMINA_LOG_WARNING( Category, Format, ... )                                                                               \
    LUMINA_LOG( Category, Lumina::ELogVerbosity::Warning, Format __VA_OPT__(, ) __VA_ARGS__ )
#define LUMINA_LOG_DISPLAY( Category, Format, ... )                                                                               \
    LUMINA_LOG( Category, Lumina::ELogVerbosity::Display, Format __VA_OPT__(, ) __VA_ARGS__ )
#define LUMINA_LOG_INFO( Category, Format, ... )                                                                                  \
    LUMINA_LOG( Category, Lumina::ELogVerbosity::Info, Format __VA_OPT__(, ) __VA_ARGS__ )
#define LUMINA_LOG_VERBOSE( Category, Format, ... )                                                                               \
    LUMINA_LOG( Category, Lumina::ELogVerbosity::Verbose, Format __VA_OPT__(, ) __VA_ARGS__ )
#define LUMINA_LOG_VERY_VERBOSE( Category, Format, ... )                                                                          \
    LUMINA_LOG( Category, Lumina::ELogVerbosity::VeryVerbose, Format __VA_OPT__(, ) __VA_ARGS__ )

#ifdef LUMINA_SHOW_LOG_SHADER_TRACE
    #define LUMINA_LOG_SHADER_TRACE( Category, Format, ... ) LUMINA_LOG_VERBOSE( Category, Format __VA_OPT__(, ) __VA_ARGS__ )
#else
    #define LUMINA_LOG_SHADER_TRACE( Category, Format, ... ) ( ( void )0 )
#endif
