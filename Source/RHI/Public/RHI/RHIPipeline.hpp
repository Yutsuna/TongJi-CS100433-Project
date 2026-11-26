/**
 * @file RHIPipeline.hpp
 * @brief Compiled shaders and directly linked OpenGL pipelines.
 */

#pragma once

#include <Core/NonCopyable.hpp>
#include <Templates/Expected.hpp>
#include <Templates/Vector.hpp>

#include <Maths/Matrix.hpp>

#include "RHIShader.hpp"
#include <RHI/RHITypes.hpp>
#include <RHIExport.hpp>

namespace Lumina
{

namespace RHI
{

    /**
     * @struct FRHIGraphicsState
     * @brief State of the graphics pipeline, including topology, culling, depth test, and blending.
     */
    struct FRHIGraphicsState
    {
        ERHIPrimitiveTopology Topology = ERHIPrimitiveTopology::Triangles;
        ERHICullMode CullMode          = ERHICullMode::Back;
        Bool bDepthTest                = false;
        Bool bDepthWrite               = false;
        Bool bBlend                    = false;
    };

    /**
     * @struct FRHIGraphicsPipelineDesc
     * @brief Descriptor for a graphics pipeline, specifying shaders and state.
     */
    struct FRHIGraphicsPipelineDesc
    {
        const FRHIShader *VertexShader   = nullptr;
        const FRHIShader *FragmentShader = nullptr;
        TSpan<const FRHIVertexAttribute> VertexAttributes;
        TSpan<const FRHIVertexBinding> VertexBindings;
        FRHIGraphicsState State;
        FStringView DebugName;
    };

    /**
     * @struct FRHIComputePipelineDesc
     * @brief Descriptor for a compute pipeline, specifying the compute shader.
     */
    struct FRHIComputePipelineDesc
    {
        const FRHIShader *ComputeShader = nullptr;
        FStringView DebugName;
    };

    /**
     * @class FRHIGraphicsPipeline
     * @brief Graphics pipeline for rendering with shaders and state.
     */
    class RHI_EXPORT FRHIPipeline : public FNonCopyable
    {
    public:

        [[nodiscard]] Bool IsValid () const noexcept;

    public:

        void SetUniform ( FStringView InName, Int32 InValue ) noexcept;
        void SetUniform ( FStringView InName, UInt32 InValue ) noexcept;
        void SetUniform ( FStringView InName, Float32 InValue ) noexcept;
        void SetUniform ( FStringView InName, const Maths::FVec2f &InValue ) noexcept;
        void SetUniform ( FStringView InName, const Maths::FVec3f &InValue ) noexcept;
        void SetUniform ( FStringView InName, const Maths::FVec4f &InValue ) noexcept;
        void SetUniform ( FStringView InName, const Maths::FMat4x4f &InValue ) noexcept;

    protected:

        FRHIPipeline () noexcept = default;
        explicit FRHIPipeline ( UInt32 InProgram ) noexcept;
        FRHIPipeline ( FRHIPipeline &&InOther ) noexcept;
        FRHIPipeline &operator=( FRHIPipeline &&InOther ) noexcept;
        ~FRHIPipeline () noexcept;

        void ResetProgram () noexcept;

    private:

        struct FRHIUniformLocation
        {
            FString Name;
            Int32 Location = -1;
        };

        [[nodiscard]] Int32 ResolveUniform ( FStringView InName ) noexcept;

        UInt32 Program = 0U;
        TVector<FRHIUniformLocation> UniformLocations;

        friend class FRHICommandList;
    };

    /**
     * @class FRHIGraphicsPipeline
     * @brief Graphics pipeline for rendering with shaders and state.
     */
    class RHI_EXPORT FRHIGraphicsPipeline final : public FRHIPipeline
    {
    public:

        FRHIGraphicsPipeline () noexcept = default;
        FRHIGraphicsPipeline ( FRHIGraphicsPipeline &&InOther ) noexcept;
        FRHIGraphicsPipeline &operator=( FRHIGraphicsPipeline &&InOther ) noexcept;
        ~FRHIGraphicsPipeline () noexcept;

    public:

        void Reset () noexcept;

    private:

        FRHIGraphicsPipeline ( UInt32 InProgram,
                               UInt32 InVertexArray,
                               const FRHIGraphicsState &InState,
                               TSpan<const FRHIVertexBinding> InBindings );

    private:

        UInt32 VertexArray      = 0U;
        FRHIGraphicsState State = {};
        TVector<FRHIVertexBinding> VertexBindings;

        friend class FRHIDevice;
        friend class FRHICommandList;
    };

    /**
     * @class FRHIComputePipeline
     * @brief Compute pipeline for executing compute shaders.
     */
    class RHI_EXPORT FRHIComputePipeline final : public FRHIPipeline
    {
    public:

        FRHIComputePipeline () noexcept = default;
        explicit FRHIComputePipeline ( UInt32 InProgram ) noexcept;
        FRHIComputePipeline ( FRHIComputePipeline &&InOther ) noexcept           = default;
        FRHIComputePipeline &operator=( FRHIComputePipeline &&InOther ) noexcept = default;
        ~FRHIComputePipeline () noexcept                                         = default;
    };

} // namespace RHI

} // namespace Lumina
