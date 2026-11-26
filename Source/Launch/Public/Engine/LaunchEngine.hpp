/**
 * @file LaunchEngine.hpp
 * Launches the Lumina Engine
 */

#pragma once

#include <Core/CoreTypes.hpp>

namespace Lumina
{

namespace Launch
{

    Int32 GuardedMain ( Int32 InArgc, const AnsiChar *const *InArgv ) noexcept;

}

} // namespace Lumina
