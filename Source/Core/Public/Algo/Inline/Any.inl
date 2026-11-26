/**
 * @file Any.inl
 * @brief Inline implementation of AnyOf algorithm.
 */

#pragma once

#include <Algo/Any.hpp>

#include <utility>

/**
 * Public
 */

template <typename TObject, typename TPredicate>
    requires(
        Lumina::Concepts::CConvertibleTo<Lumina::TInvokeResultT<TPredicate, Lumina::Algo::Internal::TRangeReferenceT<TObject>>,
                                         Lumina::Bool> )
constexpr Lumina::Bool Lumina::Algo::AnyOf ( TObject &&Object, TPredicate &&Predicate )
{
    auto &&Pred = std::forward<TPredicate>( Predicate );

    /** NOLINTNEXTLINE(readability-use-anyofallof) */
    for ( auto &&Item : Object )
    {
        if ( Invoke( Pred, Item ) )
        {
            return true;
        }
    }

    return false;
}
