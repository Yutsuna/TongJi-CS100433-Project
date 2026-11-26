/**
 * @file RHIGL.cpp
 * @brief OpenGL RHI interface
 */

#include "RHIGL.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

/**
 * Declares the OpenGL functions
 */

namespace Lumina
{

namespace RHI
{

    namespace Internal
    {

#define RHI_GL_DEFINE( Type, NativeName, Name ) Type Name = nullptr;
        RHI_GL_FUNCTION_LIST( RHI_GL_DEFINE )
#undef RHI_GL_DEFINE

    } // namespace Internal

} // namespace RHI

} // namespace Lumina

/**
 * Public
 */

Lumina::Bool Lumina::RHI::Internal::LoadGLFunctions () noexcept
{
#define RHI_GL_LOAD( Type, NativeName, Name )                                                                                    \
    Name = reinterpret_cast<Type>( glfwGetProcAddress( NativeName ) );                                                           \
    if ( Name == nullptr ) [[unlikely]]                                                                                          \
    {                                                                                                                            \
        return false;                                                                                                            \
    }
    RHI_GL_FUNCTION_LIST( RHI_GL_LOAD )
#undef RHI_GL_LOAD
    return true;
}
