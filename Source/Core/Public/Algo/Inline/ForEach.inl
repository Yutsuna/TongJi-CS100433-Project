/**
 * @file ForEach.inl
 * @brief Inline implementation of ForEach algorithm.
 */

#pragma once

#include <Algo/Any.hpp>
#include <Algo/ForEach.hpp>

#include <utility>

/**
 * Public
 */

template <typename TObject, typename TPredicate, typename TFunc>
    requires( Lumina::Concepts::CConvertibleTo<Lumina::TInvokeResultT<TPredicate, Lumina::Algo::Internal::TRangeReferenceT<TObject>>,
                                              Lumina::Bool> )
constexpr void Lumina::Algo::ForEach ( TObject &&Object, TPredicate &&Predicate, TFunc &&Func )
{
    /**
     * INFO: Bind the callable once to avoid forwarding an rvalue callable on every iteration.
     *       Otherwise it would move-from it repeatedly.
     */
    auto &&Pred     = std::forward<TPredicate>( Predicate );
    auto &&Callable = std::forward<TFunc>( Func );

    for ( auto &&Item : Object )
    {
        if ( Invoke( Pred, Item ) )
        {
            Invoke( Callable, Item );
        }
    }
}

template <typename TObject, typename TFunc> constexpr void Lumina::Algo::ForEach ( TObject &&Object, TFunc &&Func )
{
    auto &&Callable = std::forward<TFunc>( Func );

    for ( auto &&Item : Object )
    {
        Invoke( Callable, Item );
    }
}

template <typename TObject, typename TFunc>
    requires Lumina::Algo::Internal::CPointerReturnRange<TObject> &&
             Lumina::Concepts::CConvertibleTo<Lumina::TInvokeResultT<TFunc, Lumina::Algo::Internal::TRangeReferenceT<TObject>>,
                                             Lumina::Bool>
constexpr Lumina::Algo::Internal::TRangeElementPtr<TObject> Lumina::Algo::ForEachUntil ( TObject &&Object, TFunc &&Func )
{
    auto &&Callable = std::forward<TFunc>( Func );

    for ( auto &&Item : Object )
    {
        if ( Invoke( Callable, Item ) )
        {
            return std::addressof( Item );
        }
    }
    return nullptr;
}

template <typename TObject, typename TPredicate, typename TFunc>
    requires( Lumina::Concepts::CConvertibleTo<Lumina::TInvokeResultT<TPredicate, Lumina::Algo::Internal::TRangeReferenceT<TObject>>,
                                              Lumina::Bool> &&
              Lumina::Concepts::CConvertibleTo<Lumina::TInvokeResultT<TFunc, Lumina::Algo::Internal::TRangeReferenceT<TObject>>,
                                              Lumina::Bool> )
constexpr Lumina::Bool Lumina::Algo::ForEachUntil ( TObject &&Object, TPredicate &&Predicate, TFunc &&Func )
{
    auto &&Pred     = std::forward<TPredicate>( Predicate );
    auto &&Callable = std::forward<TFunc>( Func );

    return Algo::AnyOf( Object, [&Pred, &Callable] ( auto &&Item ) -> Bool
    {
        return Invoke( Pred, Item ) and Invoke( Callable, Item );
    } );
}
