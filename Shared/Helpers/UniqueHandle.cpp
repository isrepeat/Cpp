#include "UniqueHandle.h"

void UniqueHandleDeleter::operator()(HANDLE handle) noexcept {
    CloseHandle(handle);
}

void UniqueFindDeleter::operator()(HANDLE handle) noexcept {
    FindClose(handle);
}

void UniqueLibDeleter::operator()(HMODULE handle) noexcept {
    FreeLibrary(handle);
}
