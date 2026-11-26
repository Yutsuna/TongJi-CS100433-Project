/**
 * @file CoreDefinitions.hpp
 * @brief Macros and platform definitions
 */

#pragma once

#if defined( __unix__ )
    #define LUMINA_PLATFORM_UNIX
#endif

/* */

#if defined( _WIN32 ) or defined( _WIN64 )
    #define LUMINA_PLATFORM_WINDOWS
#elif defined( __linux__ )
    #define LUMINA_PLATFORM_LINUX
#elif defined( __APPLE__ ) and defined( __MACH__ )
    #define LUMINA_PLATFORM_MACOS
#endif

/* */

#if defined( __x86_64__ ) or defined( _M_X64 )
    #define LUMINA_ARCH_X64

#elif defined( __arm64__ ) or defined( _M_ARM64 )
    #define LUMINA_ARCH_ARM64

#endif

/* */

#if defined( LUMINA_PLATFORM_WINDOWS )
    #define LUMINA_PATH_SEPARATOR "\\"
    #define LUMINA_NEWLINE "\r\n"
    #define LUMINA_SHARED_LIB_EXTENSION ".dll"

#elif defined( LUMINA_PLATFORM_LINUX ) or defined( LUMINA_PLATFORM_MACOS )
    #define LUMINA_PATH_SEPARATOR "/"
    #define LUMINA_NEWLINE "\n"
    #define LUMINA_SHARED_LIB_EXTENSION ".so"
    #define LUMINA_UNUSED __attribute__( ( unused ) )

#else

#endif

/* */

#if defined( __clang__ )
    #define LUMINA_COMPILER_CLANG

#elif defined( __GNUC__ )
    #define LUMINA_COMPILER_GCC

#elif defined( _MSC_VER )
    #define LUMINA_COMPILER_MSVC

#endif

/* */

#if defined( __clang__ ) or defined( __GNUC__ )
    #define LUMINA_DISABLE_UBSAN __attribute__( ( no_sanitize( "undefined" ) ) )
#else
    #define LUMINA_DISABLE_UBSAN
#endif

/* */

#if defined( LUMINA_COMPILER_CLANG ) or defined( LUMINA_COMPILER_GCC )
    #define LUMINA_FORCE_INLINE inline __attribute__( ( always_inline ) )
    #define LUMINA_NO_INLINE __attribute__( ( noinline ) )
#elif defined( LUMINA_COMPILER_MSVC )
    #define LUMINA_FORCE_INLINE __forceinline
    #define LUMINA_NO_INLINE __declspec( noinline )
#else
    #define LUMINA_FORCE_INLINE inline
    #define LUMINA_NO_INLINE
#endif

/* */

#if defined( NDEBUG )
    #define LUMINA_ASSERT( Condition )                                                                                            \
        do                                                                                                                       \
        {                                                                                                                        \
        } while ( false )
#elif defined( LUMINA_COMPILER_CLANG ) or defined( LUMINA_COMPILER_GCC )
    #define LUMINA_ASSERT( Condition )                                                                                            \
        do                                                                                                                       \
        {                                                                                                                        \
            if ( not( Condition ) ) [[unlikely]]                                                                                 \
            {                                                                                                                    \
                __builtin_trap();                                                                                                \
            }                                                                                                                    \
        } while ( false )
#elif defined( LUMINA_COMPILER_MSVC )
    #define LUMINA_ASSERT( Condition )                                                                                            \
        do                                                                                                                       \
        {                                                                                                                        \
            if ( not( Condition ) ) [[unlikely]]                                                                                 \
            {                                                                                                                    \
                __debugbreak();                                                                                                  \
            }                                                                                                                    \
        } while ( false )
#endif

/* */

#define LUMINA_EXIT_SUCCESS 0
#define LUMINA_EXIT_FAILURE 84
