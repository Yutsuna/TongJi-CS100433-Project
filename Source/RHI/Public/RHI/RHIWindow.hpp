/**
 * @file RHIWindow.hpp
 * @brief GLFW window and OpenGL context owner.
 */

#pragma once

#include <Core/NonCopyable.hpp>
#include <Maths/Vec.hpp>

#include <RHI/RHITypes.hpp>
#include <RHIExport.hpp>

#include <Templates/Expected.hpp>

namespace Lumina
{

namespace RHI
{

    /**
     * @struct FRHIWindowDesc
     * @brief Descriptor for an RHI window.
     */
    struct FRHIWindowDesc
    {
        Maths::FVec2u Size = Maths::FVec2u( 1280U, 720U );
        FStringView Title  = "Lumina";
        Bool bResizable    = true;
        Bool bVisible      = true;
        Bool bVSync        = true;
    };

    /**
     * @class FRHIWindow
     * @brief Represents an RHI window.
     */
    class RHI_EXPORT FRHIWindow final : public FNonCopyable
    {
    public:

        FRHIWindow () noexcept = default;
        FRHIWindow ( FRHIWindow &&InOther ) noexcept;
        FRHIWindow &operator=( FRHIWindow &&InOther ) noexcept;
        ~FRHIWindow () noexcept;

    public:

        [[nodiscard]] static TExpected<FRHIWindow, FRHIError> Create ( const FRHIWindowDesc &InDesc );

    public:

        void PollEvents () const noexcept;
        void SwapBuffers () const noexcept;
        void MakeContextCurrent () const noexcept;
        void SetVSync ( Bool bEnabled ) const noexcept;
        void RequestClose () const noexcept;
        void Reset () noexcept;

        [[nodiscard]] Bool ShouldClose () const noexcept;
        [[nodiscard]] Bool IsValid () const noexcept;
        [[nodiscard]] Maths::FVec2u GetFramebufferSize () const noexcept;

    private:

        explicit FRHIWindow ( void *InWindow ) noexcept;

        void *NativeWindow = nullptr;

        friend class FRHIDevice;
    };

} // namespace RHI

} // namespace Lumina
