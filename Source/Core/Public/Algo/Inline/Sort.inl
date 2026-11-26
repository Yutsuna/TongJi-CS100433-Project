/**
 * @file Sort.inl
 * @brief Inline implementation of the Sort algorithm.
 */

#pragma once

#include <Algo/IntroSort.hpp>
#include <Algo/MergeSort.hpp>
#include <Algo/Sort.hpp>

#include <functional>
#include <utility>

/**
 * Public
 */

template <typename TRangeType>
    requires Lumina::Concepts::CSortableRange<TRangeType, std::less<>>
void Lumina::Algo::Sort ( TRangeType &&InRange ) noexcept
{
    IntroSort( std::forward<TRangeType>( InRange ) );
}

template <typename TRangeType, typename TPredicate>
    requires Lumina::Concepts::CSortableRange<TRangeType, TPredicate>
void Lumina::Algo::Sort ( TRangeType &&InRange, TPredicate InPredicate ) noexcept
{
    IntroSort( std::forward<TRangeType>( InRange ), std::move( InPredicate ) );
}

template <typename TRangeType>
    requires Lumina::Concepts::CSortableRange<TRangeType, std::less<>>
void Lumina::Algo::StableSort ( TRangeType &&InRange ) noexcept
{
    MergeSort( std::forward<TRangeType>( InRange ) );
}

template <typename TRangeType, typename TPredicate>
    requires Lumina::Concepts::CSortableRange<TRangeType, TPredicate>
void Lumina::Algo::StableSort ( TRangeType &&InRange, TPredicate InPredicate ) noexcept
{
    MergeSort( std::forward<TRangeType>( InRange ), std::move( InPredicate ) );
}
