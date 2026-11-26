/**
 * @file RHIPipeline.cpp
 * @brief Implement the FRHIPipeline class
 */

#include "RHIGL.hpp"
#include <RHI/RHIPipeline.hpp>

#include <Algo/Find.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

/**
 * Pipeline
 */

Lumina::RHI::FRHIPipeline::FRHIPipeline ( const UInt32 InProgram ) noexcept : Program( InProgram )
{
    /* */
}

Lumina::RHI::FRHIPipeline::FRHIPipeline ( FRHIPipeline &&InOther ) noexcept
    : Program( std::exchange( InOther.Program, 0U ) ), UniformLocations( std::move( InOther.UniformLocations ) )
{
    /* */
}

Lumina::RHI::FRHIPipeline &Lumina::RHI::FRHIPipeline::operator=( FRHIPipeline &&InOther ) noexcept
{
    if ( this != &InOther )
    {
        ResetProgram();
        Program          = std::exchange( InOther.Program, 0U );
        UniformLocations = std::move( InOther.UniformLocations );
    }
    return *this;
}

Lumina::RHI::FRHIPipeline::~FRHIPipeline () noexcept
{
    ResetProgram();
}

Lumina::Bool Lumina::RHI::FRHIPipeline::IsValid () const noexcept
{
    return Program != 0U;
}

Lumina::Int32 Lumina::RHI::FRHIPipeline::ResolveUniform ( const FStringView InName ) noexcept
{
    auto *const Found = Algo::FindIf( UniformLocations, [InName] ( const FRHIUniformLocation &InEntry ) noexcept
    {
        return InEntry.Name == InName;
    } );
    if ( Found != nullptr )
    {
        return Found->Location;
    }

    const FString Name( InName );
    const Int32 Location = Internal::GetUniformLocation( Program, Name.c_str() );

    UniformLocations.push_back( { .Name = Name, .Location = Location } );
    return Location;
}

void Lumina::RHI::FRHIPipeline::SetUniform ( const FStringView InName, const Int32 InValue ) noexcept
{
    const Int32 Location = ResolveUniform( InName );
    if ( Location >= 0 )
    {
        Internal::ProgramUniform1i( Program, Location, InValue );
    }
}

void Lumina::RHI::FRHIPipeline::SetUniform ( const FStringView InName, const UInt32 InValue ) noexcept
{
    const Int32 Location = ResolveUniform( InName );
    if ( Location >= 0 )
    {
        Internal::ProgramUniform1ui( Program, Location, InValue );
    }
}

void Lumina::RHI::FRHIPipeline::SetUniform ( const FStringView InName, const Float32 InValue ) noexcept
{
    const Int32 Location = ResolveUniform( InName );
    if ( Location >= 0 )
    {
        Internal::ProgramUniform1f( Program, Location, InValue );
    }
}

void Lumina::RHI::FRHIPipeline::SetUniform ( const FStringView InName, const Maths::FVec2f &InValue ) noexcept
{
    const Int32 Location = ResolveUniform( InName );
    if ( Location >= 0 )
    {
        Internal::ProgramUniform2f( Program, Location, InValue.X, InValue.Y );
    }
}

void Lumina::RHI::FRHIPipeline::SetUniform ( const FStringView InName, const Maths::FVec3f &InValue ) noexcept
{
    const Int32 Location = ResolveUniform( InName );
    if ( Location >= 0 )
    {
        Internal::ProgramUniform3f( Program, Location, InValue.X, InValue.Y, InValue.Z );
    }
}

void Lumina::RHI::FRHIPipeline::SetUniform ( const FStringView InName, const Maths::FVec4f &InValue ) noexcept
{
    const Int32 Location = ResolveUniform( InName );
    if ( Location >= 0 )
    {
        Internal::ProgramUniform4f( Program, Location, InValue.X, InValue.Y, InValue.Z, InValue.W );
    }
}

void Lumina::RHI::FRHIPipeline::SetUniform ( const FStringView InName, const Maths::FMat4x4f &InValue ) noexcept
{
    const Int32 Location = ResolveUniform( InName );
    if ( Location < 0 )
    {
        return;
    }

    glm::mat4 Converted( 1.0F );
    for ( USize Column = 0U; Column < 4U; ++Column )
    {
        for ( USize Row = 0U; Row < 4U; ++Row )
        {
            Converted[static_cast<glm::length_t>( Column )][static_cast<glm::length_t>( Row )] = InValue[Column].Data[Row];
        }
    }
    Internal::ProgramUniformMatrix4fv( Program, Location, 1, GL_FALSE, glm::value_ptr( Converted ) );
}

void Lumina::RHI::FRHIPipeline::ResetProgram () noexcept
{
    if ( Program != 0U )
    {
        Internal::DeleteProgram( Program );
    }
    Program = 0U;
    UniformLocations.clear();
}
