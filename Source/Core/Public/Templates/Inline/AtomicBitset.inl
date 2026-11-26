/**
 * @file AtomicBitset.inl
 * @brief Implementation of the FAtomicBitset class.
 */

#pragma once

#include <Templates/AtomicBitset.hpp>

#include <bit>

Lumina::FAtomicBitset::FAtomicBitset ( USize InNumBits )
{
    Resize( InNumBits );
}

void Lumina::FAtomicBitset::Set ( USize InIndex ) noexcept
{
    LUMINA_ASSERT( InIndex < NumBits );

    const USize BlockIndex = InIndex / BitsPerBlock;
    const USize BitOffset  = InIndex % BitsPerBlock;
    const UInt64 Mask      = One << BitOffset;

    /** INFO: Release pairs with the acquire in ForEachSetBitAndClear: data written before Set()
     *       becomes visible to the thread that consumes the dirty bit. */
    Blocks[BlockIndex].fetch_or( Mask, std::memory_order_release );
}

void Lumina::FAtomicBitset::Clear ( USize InIndex ) noexcept
{
    LUMINA_ASSERT( InIndex < NumBits );

    const USize BlockIndex = InIndex / BitsPerBlock;
    const USize BitOffset  = InIndex % BitsPerBlock;
    const UInt64 Mask      = One << BitOffset;

    Blocks[BlockIndex].fetch_and( ~Mask, std::memory_order_relaxed );
}

Lumina::Bool Lumina::FAtomicBitset::Test ( USize InIndex ) const noexcept
{
    LUMINA_ASSERT( InIndex < NumBits );

    const USize BlockIndex = InIndex / BitsPerBlock;
    const USize BitOffset  = InIndex % BitsPerBlock;
    const UInt64 Mask      = One << BitOffset;

    return ( Blocks[BlockIndex].load( std::memory_order_acquire ) & Mask ) != 0ULL;
}

template <typename Callable>
    requires Lumina::Concepts::CIsInvocable<Callable, Lumina::USize>
void Lumina::FAtomicBitset::ForEachSetBitAndClear ( Callable &&InFunc ) noexcept
{
    for ( USize BlockIdx = 0U; BlockIdx < NumBlocks; ++BlockIdx )
    {
        /** INFO: Atomic exchange ensures we consume the bits and clear them in one op.
         *       Acquire pairs with the release in Set() so the consumer sees the data that was
         *       marked dirty, not just the flag. */
        UInt64 Value = Blocks[BlockIdx].exchange( 0ULL, std::memory_order_acquire );

        while ( Value != 0ULL )
        {
            const USize BitIdx = static_cast<USize>( std::countr_zero( Value ) );
            InFunc( ( BlockIdx * BitsPerBlock ) + BitIdx );
            Value &= ~( One << BitIdx );
        }
    }
}

void Lumina::FAtomicBitset::ClearAll () noexcept
{
    for ( USize Index = 0U; Index < NumBlocks; ++Index )
    {
        Blocks[Index].store( 0ULL, std::memory_order_relaxed );
    }
}

LUMINA_FORCE_INLINE Lumina::USize Lumina::FAtomicBitset::GetNumBits () const noexcept
{
    return NumBits;
}
