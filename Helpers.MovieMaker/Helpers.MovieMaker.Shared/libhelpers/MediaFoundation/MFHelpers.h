#pragma once
#include "MFInclude.h"

#include <cstdint>

class MFHelpers {
public:
    // drop fractional part
    // works ok if temporal multiplication will result in 128bit result and after division will return to 64bit
    static int64_t Convert(int64_t value, int64_t valueUnits, int64_t dstUnits);

    // round +0.5
    // works ok if temporal multiplication will result in 128bit result and after division will return to 64bit
    static int64_t ConvertRound(int64_t value, int64_t valueUnits, int64_t dstUnits);

    // ceil rounding
    // works ok if temporal multiplication will result in 128bit result and after division will return to 64bit
    static int64_t ConvertCeil(int64_t value, int64_t valueUnits, int64_t dstUnits);
    
    static inline VARIANT MakeVariantUINT(UINT val) {
        VARIANT variant{};
        variant.vt = VT_UI4;
        variant.uintVal = val;
        return variant;
    }
};