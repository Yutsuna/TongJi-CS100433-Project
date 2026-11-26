/**
 * @file RHIGL.cpp
 * @brief OpenGL RHI interface
 */

#include "RHIGL.hpp"
#include <RHIPrivate.hpp>

#include <HAL/LockGuard.hpp>
#include <HAL/PlatformMutex.hpp>
#include <Templates/Unexpected.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

/**
 * Declares the OpenGL functions
 */

#define RHI_GL_DEFINE( Type, NativeName, Name ) Type Lumina::RHI::Internal::Name = nullptr;
RHI_GL_FUNCTION_LIST( RHI_GL_DEFINE )
#undef RHI_GL_DEFINE

namespace
{

/** NOTE: Serializes the one-time population of the OpenGL function table. */
Lumina::HAL::FPlatformSpinLockMutex GLLoadMutex;

/** NOTE: Published with release semantics once every function pointer is loaded. */
Lumina::TAtomic<Lumina::Bool> bGLFunctionsLoaded{ false };

} // namespace

/**
 * Public
 */

Lumina::RHI::FRHIExpected Lumina::RHI::Internal::LoadGLFunctions () noexcept
{
    if ( bGLFunctionsLoaded.load( std::memory_order::acquire ) )
    {
        return {};
    }

    const Lumina::HAL::TLockGuard<Lumina::HAL::FPlatformSpinLockMutex> Lock( GLLoadMutex );

    if ( bGLFunctionsLoaded.load( std::memory_order::acquire ) )
    {
        return {};
    }

#define RHI_GL_LOAD( Type, NativeName, Name )                                                                                    \
    Name = reinterpret_cast<Type>( glfwGetProcAddress( NativeName ) );                                                           \
    if ( Name == nullptr ) [[unlikely]]                                                                                          \
    {                                                                                                                            \
        return MakeUnexpected(                                                                                                   \
            MakeError( ERHIErrorCode::InitializationFailed, "One or more required OpenGL 4.6 functions are unavailable" ) );     \
    }
    RHI_GL_FUNCTION_LIST( RHI_GL_LOAD )
#undef RHI_GL_LOAD

    bGLFunctionsLoaded.store( true, std::memory_order::release );
    return {};
}
