/**
 * @file Main.cpp
 * @brief App module entry-point.
 */

#include <Engine/LaunchEngine.hpp>

Lumina::Int32 main ( const Lumina::Int32 Argc, const Lumina::AnsiChar *Argv[] )
{
    return Lumina::Launch::GuardedMain( Argc, Argv );
}
