#pragma once
#include <stdint.h>

namespace HData {
    size_t CombineHash(size_t a, size_t b) {
        // based on http://en.cppreference.com/w/cpp/utility/hash
        return a ^ (b << 1);
    }
}