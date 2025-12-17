#pragma once
#include "common.h"

#include <type_traits>

namespace HELPERS_NS {
    /*
    Code below does this:
    uint64_t res =
        ((v >> 56) & 0x00000000000000FF) |
        ((v >> 40) & 0x000000000000FF00) |
        ((v >> 24) & 0x0000000000FF0000) |
        ((v >> 8) & 0x00000000FF000000) |
        ((v << 8) & 0x000000FF00000000) |
        ((v << 24) & 0x0000FF0000000000) |
        ((v << 40) & 0x00FF000000000000) |
        ((v << 56) & 0xFF00000000000000)
    ;

    This implementation maps to bswap assembler instruction according to https://godbolt.org/
    Settings for msvc: /O2 /std:c++20
    Settings for clang/gcc: -O2

    ByteMoveMasked does part such as ((v >> 56) & 0x00000000000000FF) or ((v << 56) & 0xFF00000000000000)

    Byteswap does iterations for all shifts and masks
    */

    // Moves 1 byte from SrcByteIdx to DstByteIdx and masks result with 0xFF mask
    // For int64 SrcByteIdx/DstByteIdx in range from 0 to 7
    // For int32 SrcByteIdx/DstByteIdx in range from 0 to 3
    // For int16 SrcByteIdx/DstByteIdx in range from 0 to 1
    template<size_t SrcByteIdx, size_t DstByteIdx, typename T>
    T ByteMoveMasked(T v) {
        constexpr size_t Bits = 8;
        constexpr T ByteMask = 0xFF;

        constexpr size_t DstBitIdx = Bits * DstByteIdx;
        constexpr size_t SrcBitIdx = Bits * SrcByteIdx;
        constexpr T DstByteMask = ByteMask << DstBitIdx;

        if constexpr (DstBitIdx > SrcBitIdx) {
            constexpr size_t ShiftBits = DstBitIdx - SrcBitIdx;
            return (v << ShiftBits) & DstByteMask;
        }
        else if constexpr (SrcBitIdx > DstBitIdx) {
            constexpr size_t ShiftBits = SrcBitIdx - DstBitIdx;
            return (v >> ShiftBits) & DstByteMask;
        }
        else // DstByteIdx == SrcByteIdx
        {
            return v & DstByteMask;
        }
    }

    namespace details {
        template<typename T, size_t... Idx>
        T Byteswap(T v, std::index_sequence<Idx...>) {
            constexpr size_t LastIdx = sizeof...(Idx) - 1;

            T res = (ByteMoveMasked<Idx, LastIdx - Idx>(v) | ...);
            return res;
        }
    }

    // Reverses the bytes in v
    template<typename T>
    T Byteswap(T v) {
        return details::Byteswap(v, std::make_index_sequence<sizeof(T)>());
    }
}
