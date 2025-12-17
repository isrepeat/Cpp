#pragma once

#include <memory>
#include <type_traits>
#include <Windows.h>

struct UniqueHandleDeleter {
    void operator()(HANDLE handle);
};

struct UniqueFindDeleter {
    void operator()(HANDLE handle);
};

using UniqueHandle = std::unique_ptr<std::remove_pointer<HANDLE>::type, UniqueHandleDeleter>;
using UniqueFind = std::unique_ptr<std::remove_pointer<HANDLE>::type, UniqueFindDeleter>;
