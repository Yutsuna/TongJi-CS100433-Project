///
/// @file CoreDefinitions.hpp
/// @brief Macros and platform definitions
///

#pragma once

#if defined( __unix__ )
    #define Lumina_PLATFORM_UNIX
#endif
#if defined( _WIN32 ) || defined( _WIN64 )
    #define Lumina_PLATFORM_WINDOWS

#elif defined( __linux__ )
    #define Lumina_PLATFORM_LINUX

#elif defined( __APPLE__ ) && defined( __MACH__ )
    #define Lumina_PLATFORM_MACOS

#else
    #define Lumina_PATH_SEPARATOR "/"
    #define Lumina_NEWLINE "\n"
    #define Lumina_SHARED_LIB_EXTENSION ""
#endif

#if defined( Lumina_PLATFORM_WINDOWS )
    #define Lumina_PATH_SEPARATOR "\\"
    #define Lumina_NEWLINE "\r\n"
    #define Lumina_SHARED_LIB_EXTENSION ".dll"

#elif defined( Lumina_PLATFORM_LINUX ) || defined( Lumina_PLATFORM_MACOS )
    #define Lumina_PATH_SEPARATOR "/"
    #define Lumina_NEWLINE "\n"
    #define Lumina_SHARED_LIB_EXTENSION ".so"
    #define Lumina_UNUSED __attribute__( ( unused ) )

#else

#endif

#if defined( __clang__ )
    #define Lumina_COMPILER_CLANG

#elif defined( __GNUC__ )
    #define Lumina_COMPILER_GCC

#endif

#if defined( __clang__ ) || defined( __GNUC__ )
    #define Lumina_DISABLE_UBSAN __attribute__( ( no_sanitize( "undefined" ) ) )
#else
    #define Lumina_DISABLE_UBSAN
#endif

#define Lumina_EXIT_SUCCESS 0
#define Lumina_EXIT_FAILURE 84
