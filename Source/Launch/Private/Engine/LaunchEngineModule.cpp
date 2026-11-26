/**
 * @file LaunchEngineModule.cpp
 * @brief Module registration table.
 */

#include <Core/CoreDefinitions.hpp>
#include <Engine/LaunchEngineModule.hpp>

#include <Memory/Span.hpp>
#include <Templates/Array.hpp>

/**
 * Public
 */

Lumina::Launch::EModuleTraits Lumina::Launch::IEngineModule::GetTraits () const noexcept
{
    return EModuleTraits::None;
}

void Lumina::Launch::IEngineModule::FixedTick ( Float64 LUMINA_UNUSED InFixedDeltaSeconds ) noexcept
{
    /** Empty */
}

void Lumina::Launch::IEngineModule::Tick ( Float64 LUMINA_UNUSED InDeltaSeconds, Float64 LUMINA_UNUSED InAlpha ) noexcept
{
    /** Empty */
}

Lumina::TSpan<const Lumina::Launch::FModuleDescriptor> Lumina::Launch::GetEngineModules () noexcept
{
    return {};
    //static constexpr TArray<FModuleDescriptor, 2> Modules = {
    //    FModuleDescriptor{ .Name = "Application", .Create = &MakeApplicationModule },
    //    FModuleDescriptor{ .Name = "Window", .Create = &MakeWindowModule },
    //};
//
    //return Modules;
}
