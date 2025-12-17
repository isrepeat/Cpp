#pragma once

#include <optional>

template<typename T>
std::optional<T> as_optional(const T* ptr) {
    if (ptr) {
        return *ptr;
    }

    return {};
}
