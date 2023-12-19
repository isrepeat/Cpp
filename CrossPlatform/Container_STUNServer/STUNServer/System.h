#pragma once
#include "Platform.h"
#if defined(__WINDOWS_OS__)
#include <Windows.h>
#include <stdexcept>
#include <comdef.h>

namespace H {
    namespace System {
        inline void ThrowIfFailed(HRESULT hr) {
            if (FAILED(hr)) {
                throw std::runtime_error(std::string{ reinterpret_cast<const char*>(_com_error{ hr }.ErrorMessage()) });
            }
        }
    }
}
#elif defined(__UNIX_OS__)
#include <msquic_posix.h>
#include <stdexcept>

namespace H {
    namespace System {
        inline void ThrowIfFailed(int hr) {
            if (QUIC_FAILED(hr)) {
                throw std::runtime_error("thrown");
            }
        }
    }
}
#endif