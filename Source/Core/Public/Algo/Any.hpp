/**
 * @file Any.hpp
 * @brief Any algorithm implementation.
 */

#pragma once

#include <Core/CoreDefinitions.hpp>
#include <Core/CoreTypes.hpp>

#include <Algo/RangeTraits.hpp>

#include <Concepts/ConvertibleTo.hpp>
#include <Templates/Invoke.hpp>

namespace Lumina
{

namespace Algo
{

    /**
     * @brief Returns true if any element in the range satisfies Predicate.
     *
     * @param Object The range to iterate over.
     * @param Predicate The predicate to test against each element.
     */
    template <class TObject, typename TPredicate>
        requires( Concepts::CConvertibleTo<TInvokeResultT<TPredicate, Internal::TRangeReferenceT<TObject>>, Bool> )
    LUMINA_FORCE_INLINE constexpr Bool AnyOf ( TObject &&Object, TPredicate &&Predicate );

} // namespace Algo

} // namespace Lumina

#include "Inline/Any.inl"
