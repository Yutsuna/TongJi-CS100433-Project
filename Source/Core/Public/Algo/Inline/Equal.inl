/**
 * @file Equal.inl
 * @brief Inline implementation of Equal algorithms.
 */

#pragma once

#include <Algo/Equal.hpp>

#include <utility>

/**
 * Public
 */

template <typename TRangeA, typename TRangeB, typename TPredicate>
    requires( Lumina::Concepts::CConvertibleTo<Lumina::TInvokeResultT<TPredicate,
                                                                      Lumina::Algo::Internal::TRangeReferenceT<TRangeA>,
                                                                      Lumina::Algo::Internal::TRangeReferenceT<TRangeB>>,
                                               Lumina::Bool> )
constexpr Lumina::Bool Lumina::Algo::Equal ( TRangeA &&InRangeA, TRangeB &&InRangeB, TPredicate &&InPredicate )
{
    auto &&Pred = std::forward<TPredicate>( InPredicate );
    auto ItA    = Begin( InRangeA );
    auto ItB    = Begin( InRangeB );
    auto EndA   = End( InRangeA );
    auto EndB   = End( InRangeB );

    for ( ; ItA != EndA and ItB != EndB; ++ItA, ++ItB )
    {
        if ( not Lumina::Invoke( Pred, *ItA, *ItB ) )
        {
            return false;
        }
    }

    return ItA == EndA and ItB == EndB;
}

template <typename TRangeA, typename TRangeB>
constexpr Lumina::Bool Lumina::Algo::Equal ( TRangeA &&InRangeA, TRangeB &&InRangeB )
{
    return Equal( std::forward<TRangeA>( InRangeA ), std::forward<TRangeB>( InRangeB ),
                  [] ( auto &&A, auto &&B ) noexcept -> Lumina::Bool
    {
        return A == B;
    } );
}
