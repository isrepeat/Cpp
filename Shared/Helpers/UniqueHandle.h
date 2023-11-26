#pragma once

#include <memory>
#include <type_traits>
#include "HWindows.h"

struct UniqueHandleDeleter {
    void operator()(HANDLE handle) noexcept;
};

struct UniqueFindDeleter {
    void operator()(HANDLE handle) noexcept;
};

struct UniqueLibDeleter {
    void operator()(HMODULE handle) noexcept;
};

typedef std::unique_ptr<std::remove_pointer<HANDLE>::type, UniqueHandleDeleter> UHANDLE;
typedef std::unique_ptr<std::remove_pointer<HANDLE>::type, UniqueFindDeleter> UFind;
typedef std::unique_ptr<std::remove_pointer<HMODULE>::type, UniqueLibDeleter> UHMODULE;
