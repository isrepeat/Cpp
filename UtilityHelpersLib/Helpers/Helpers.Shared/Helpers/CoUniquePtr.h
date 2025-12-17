#pragma once
#include "common.h"
#include "GetAddressOf.h"
#include "HWindows.h"
#include <memory>

template<class T>
struct CoDeleter {
    void operator()(T *ptr) {
        CoTaskMemFree(ptr);
    }

    void operator()(T **ptr) {
        (*this)((T*)ptr);
    }
};

template<typename T> using CoUniquePtr = std::unique_ptr<T, CoDeleter<T>>;
