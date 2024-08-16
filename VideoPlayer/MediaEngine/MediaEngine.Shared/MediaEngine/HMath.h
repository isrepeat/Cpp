#pragma once
#include <stdint.h>

namespace HMath {
    // Drop fractional part
    template<class T> static T Convert2(T value, T valueUnits, T dstUnits) {
        /*T tmp = value * dstUnits;
        T res = (tmp / valueUnits) + !!(tmp % valueUnits);*/

        /*T tmp = value * dstUnits * byteSize;
        T res = (tmp + (valueUnits / 2)) / (valueUnits * byteSize);*/

        T tmp = value * dstUnits;
        T res = (tmp) / valueUnits;


        /*double ratio = (double) dstUnits / (double) valueUnits;
        T res = (T)((double) value * ratio);*/

        /*T tmp = value * dstUnits;
        T res = (tmp) / valueUnits;*/

        return res;
    }

    // Using round up +1.0
    template<class T> static T ConvertCeil(T value, T valueUnits, T dstUnits) {
        /*T tmp = value * dstUnits;
        T res = (tmp / valueUnits) + !!(tmp % valueUnits);*/

        /*T tmp = value * dstUnits * byteSize;
        T res = (tmp + (valueUnits / 2)) / (valueUnits * byteSize);*/


        T tmp = value * dstUnits;
        T tmp2 = valueUnits;
        T res = (tmp + tmp2) / valueUnits;


        /*double ratio = (double) dstUnits / (double) valueUnits;
        T res = (T)((double) value * ratio);*/

        /*T tmp = value * dstUnits;
        T res = (tmp) / valueUnits;*/

        return res;
    }

    // Using round up +0.5
    template<class T> static T Convert(T value, T valueUnits, T dstUnits) {
        /*T tmp = value * dstUnits;
        T res = (tmp / valueUnits) + !!(tmp % valueUnits);*/

        /*T tmp = value * dstUnits * byteSize;
        T res = (tmp + (valueUnits / 2)) / (valueUnits * byteSize);*/


        T tmp = value * dstUnits;
        T tmp2 = valueUnits / 2;
        T res = (tmp + tmp2) / valueUnits;


        /*double ratio = (double) dstUnits / (double) valueUnits;
        T res = (T)((double) value * ratio);*/

        /*T tmp = value * dstUnits;
        T res = (tmp) / valueUnits;*/

        return res;
    }

    template<class T>
    static T Clamp(T value, T min, T max) {
        T res = (std::min)((std::max)(min, value), max);
        return res;
    }


    // round up for power of two multiples
    template<class T> static T RoundUpPOT(T numToRound, T potMultiple) {
        T res = (numToRound + potMultiple - 1) & ~(potMultiple - 1);
        return res;
    }

    template<class T> static T RoundUpPOT(T v) {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;
        return v;
    }
}