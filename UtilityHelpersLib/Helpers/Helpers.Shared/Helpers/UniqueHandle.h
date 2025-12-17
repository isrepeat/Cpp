#pragma once
#include "common.h"
#include "HWindows.h"
#include <type_traits>
#include <memory>

namespace HELPERS_NS {
    struct UniqueHandleDeleter {
        void operator()(HANDLE handle) noexcept;
    };

    struct UniqueFindDeleter {
        void operator()(HANDLE handle) noexcept;
    };

    struct UniqueLibDeleter {
        void operator()(HMODULE handle) noexcept;
    };

    using UHMODULE = std::unique_ptr<std::remove_pointer<HMODULE>::type, UniqueLibDeleter>;
    using UHANDLE = std::unique_ptr<std::remove_pointer<HANDLE>::type, UniqueHandleDeleter>;
    using UFind = std::unique_ptr<std::remove_pointer<HANDLE>::type, UniqueFindDeleter>;
}