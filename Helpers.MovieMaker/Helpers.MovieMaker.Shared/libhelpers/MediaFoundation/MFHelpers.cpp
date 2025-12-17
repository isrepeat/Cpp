#include "pch.h"
#include "MFHelpers.h"

int64_t MFHelpers::Convert(int64_t value, int64_t valueUnits, int64_t dstUnits) {
    int64_t res = MFllMulDiv(value, dstUnits, valueUnits, 0);
    return res;
}

int64_t MFHelpers::ConvertRound(int64_t value, int64_t valueUnits, int64_t dstUnits) {
    int64_t res = MFllMulDiv(value, dstUnits, valueUnits, valueUnits / 2);
    return res;
}

int64_t MFHelpers::ConvertCeil(int64_t value, int64_t valueUnits, int64_t dstUnits) {
    int64_t res = MFllMulDiv(value, dstUnits, valueUnits, valueUnits);
    return res;
}