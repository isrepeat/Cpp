#include "UniqueHandle.h"

namespace HELPERS_NS {
    void UniqueHandleDeleter::operator()(HANDLE handle) noexcept {
        if (handle && handle != INVALID_HANDLE_VALUE) {
            CloseHandle(handle);
        }
    }

    void UniqueFindDeleter::operator()(HANDLE handle) noexcept {
        if (handle && handle != INVALID_HANDLE_VALUE) {
            FindClose(handle);
        }
    }

    void UniqueLibDeleter::operator()(HMODULE handle) noexcept {
        if (handle && handle != INVALID_HANDLE_VALUE) {
            FreeLibrary(handle);
        }
    }
}