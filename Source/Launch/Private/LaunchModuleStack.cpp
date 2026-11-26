/**
 * @file LaunchModuleStack.cpp
 * @brief FLaunchModuleStack implementation.
 */

#include <Engine/LaunchEngineModule.hpp>
#include <Launch/LaunchModuleStack.hpp>

#include <Algo/ForEach.hpp>
#include <Logging/Logger.hpp>

/**
 * Private
 */

namespace
{

LUMINA_LOG_DEFINE_CATEGORY( LogModuleStack, "ModuleStack" )

}

/**
 * Public
 */

Lumina::Launch::FLaunchModuleStack::FLaunchModuleStack ( FLaunchModuleStack &&Other ) noexcept
    : Modules( std::move( Other.Modules ) ), FixedTickList( std::move( Other.FixedTickList ) ),
      TickList( std::move( Other.TickList ) )
{
    Other.Modules.clear();
    Other.FixedTickList.clear();
    Other.TickList.clear();
}

Lumina::Launch::FLaunchModuleStack::~FLaunchModuleStack () noexcept
{
    ShutdownAll();
}

Lumina::Launch::TLaunchResult<void>
Lumina::Launch::FLaunchModuleStack::StartAll ( const TSpan<const FModuleDescriptor> InDescriptors,
                                               const FLaunchEngineConfig &InConfig ) noexcept
{
    /** Reserve once: the push_back calls below can no longer allocate. */
    Modules.reserve( Modules.size() + InDescriptors.size() );
    FixedTickList.reserve( FixedTickList.size() + InDescriptors.size() );
    TickList.reserve( TickList.size() + InDescriptors.size() );

    for ( const FModuleDescriptor &Descriptor : InDescriptors )
    {
        LUMINA_LOG_INFO( LogModuleStack, "Starting module '{}'", Descriptor.Name );

        if ( Descriptor.Create == nullptr ) [[unlikely]]
        {
            LUMINA_LOG_ERROR( LogModuleStack, "Module '{}' has no factory", Descriptor.Name );
            return LaunchFailure( ELaunchStage::Init, ELaunchErrorCode::ModuleCreationFailed,
                                  "A module descriptor has no factory" );
        }

        auto Created = Descriptor.Create( InConfig );
        if ( not Created ) [[unlikely]]
        {
            /** The error message is a static literal and cannot carry the module name: log it here. */
            LUMINA_LOG_ERROR( LogModuleStack, "Module '{}' failed to start", Descriptor.Name );
            return MakeUnexpected( Created.Error() );
        }

        TUniquePtr<IEngineModule> Module = std::move( *Created );
        if ( not Module ) [[unlikely]]
        {
            LUMINA_LOG_ERROR( LogModuleStack, "Module '{}': factory returned a null module", Descriptor.Name );
            return LaunchFailure( ELaunchStage::Init, ELaunchErrorCode::ModuleCreationFailed,
                                  "A module factory returned a null module" );
        }

        const EModuleTraits Traits = Module->GetTraits();
        if ( HasAnyFlags( Traits, EModuleTraits::FixedTick ) )
        {
            FixedTickList.push_back( Module.Get() );
        }
        if ( HasAnyFlags( Traits, EModuleTraits::Tick ) )
        {
            TickList.push_back( Module.Get() );
        }

        Modules.push_back( std::move( Module ) );
    }

    return {};
}

void Lumina::Launch::FLaunchModuleStack::ShutdownAll () noexcept
{
    FixedTickList.clear();
    TickList.clear();

    while ( not Modules.empty() )
    {
        Modules.pop_back();
    }
}

void Lumina::Launch::FLaunchModuleStack::FixedTick ( const Float64 InFixedDeltaSeconds ) noexcept
{
    Algo::ForEach( FixedTickList, [&] ( Lumina::Launch::IEngineModule *const Module ) noexcept -> void
    {
        Module->FixedTick( InFixedDeltaSeconds );
    } );
}

void Lumina::Launch::FLaunchModuleStack::Tick ( const Float64 InDeltaSeconds, const Float64 InAlpha ) noexcept
{
    Algo::ForEach( TickList, [&] ( Lumina::Launch::IEngineModule *const Module ) noexcept -> void
    {
        Module->Tick( InDeltaSeconds, InAlpha );
    } );
}
