/**
 * @file RHIShader.hpp
 * @brief Shader class for the RHI
 */

#pragma once

#include <RHI/RHITypes.hpp>
#include <RHIExport.hpp>

#include <Core/NonCopyable.hpp>

namespace Lumina
{

namespace RHI
{

    /**
     * @class FRHIShader
     * @brief Represents a shader for the RHI
     */
    class RHI_EXPORT FRHIShader final : public FNonCopyable
    {
    public:

        FRHIShader () noexcept = default;
        FRHIShader ( FRHIShader &&InOther ) noexcept;
        FRHIShader &operator=( FRHIShader &&InOther ) noexcept;
        ~FRHIShader () noexcept;

    public:

        [[nodiscard]] Bool IsValid () const noexcept;
        [[nodiscard]] ERHIShaderStage GetStage () const noexcept;
        void Reset () noexcept;

    private:

        FRHIShader ( UInt32 InId, ERHIShaderStage InStage ) noexcept;

        UInt32 Id             = 0U;
        ERHIShaderStage Stage = ERHIShaderStage::Vertex;

        friend class FRHIDevice;
    };

} // namespace RHI

} // namespace Lumina
