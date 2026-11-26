/**
 * @file RHIShader.cpp
 * @brief Implement the FRHIShader class
 */

#include "RHIShader.hpp"
#include "RHIGL.hpp"

/**
 * Public
 */

Lumina::RHI::FRHIShader::FRHIShader ( const UInt32 InId, const ERHIShaderStage InStage ) noexcept : Id( InId ), Stage( InStage )
{
    /* */
}

Lumina::RHI::FRHIShader::FRHIShader ( FRHIShader &&InOther ) noexcept
    : Id( std::exchange( InOther.Id, 0U ) ), Stage( InOther.Stage )
{
    /* */
}

Lumina::RHI::FRHIShader &Lumina::RHI::FRHIShader::operator=( FRHIShader &&InOther ) noexcept
{
    if ( this != &InOther )
    {
        Reset();
        Id    = std::exchange( InOther.Id, 0U );
        Stage = InOther.Stage;
    }
    return *this;
}

Lumina::RHI::FRHIShader::~FRHIShader () noexcept
{
    Reset();
}

Lumina::Bool Lumina::RHI::FRHIShader::IsValid () const noexcept
{
    return Id != 0U;
}

Lumina::RHI::ERHIShaderStage Lumina::RHI::FRHIShader::GetStage () const noexcept
{
    return Stage;
}

void Lumina::RHI::FRHIShader::Reset () noexcept
{
    if ( Id != 0U )
    {
        Internal::DeleteShader( Id );
    }
    Id = 0U;
}
