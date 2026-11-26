/**
 * @file RHIGraphicsPipeline.cpp
 * @brief Graphics pipeline implementation.
 */

#include <RHI/RHIPipeline.hpp>

/**
 * Public
 */

Lumina::RHI::FRHIGraphicsPipeline::FRHIGraphicsPipeline ( const UInt32 InProgram,
                                                          const UInt32 InVertexArray,
                                                          const FRHIGraphicsState &InState,
                                                          const TSpan<const FRHIVertexBinding> InBindings )
    : FRHIPipeline( InProgram ), VertexArray( InVertexArray ), State( InState ),
      VertexBindings( InBindings.begin(), InBindings.end() )
{
    /* */
}

Lumina::RHI::FRHIGraphicsPipeline::FRHIGraphicsPipeline ( FRHIGraphicsPipeline &&InOther ) noexcept
    : FRHIPipeline( std::move( InOther ) ), VertexArray( std::exchange( InOther.VertexArray, 0U ) ), State( InOther.State ),
      VertexBindings( std::move( InOther.VertexBindings ) )
{
    /* */
}

Lumina::RHI::FRHIGraphicsPipeline &Lumina::RHI::FRHIGraphicsPipeline::operator=( FRHIGraphicsPipeline &&InOther ) noexcept
{
    if ( this != &InOther )
    {
        Reset();
        Lumina::RHI::FRHIPipeline::operator=( std::move( InOther ) );
        VertexArray    = std::exchange( InOther.VertexArray, 0U );
        State          = InOther.State;
        VertexBindings = std::move( InOther.VertexBindings );
    }
    return *this;
}

Lumina::RHI::FRHIGraphicsPipeline::~FRHIGraphicsPipeline () noexcept
{
    Reset();
}

void Lumina::RHI::FRHIGraphicsPipeline::Reset () noexcept
{
    if ( VertexArray != 0U )
    {
        Internal::DeleteVertexArrays( 1, &VertexArray );
    }
    VertexArray = 0U;
    VertexBindings.clear();
    ResetProgram();
}
