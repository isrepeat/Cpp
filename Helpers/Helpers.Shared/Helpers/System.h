#pragma once
#include "common.h"
#include "Exceptions.h"
#include "Logger.h"
#include <comdef.h>

namespace HELPERS_NS {
    namespace System {
#if COMPILE_FOR_DESKTOP
        inline void ThrowIfFailed(HRESULT hr) {
            if (FAILED(hr)) {
                LOG_FAILED(hr);
                Dbreak;
                throw ComException(hr, _com_error(hr).ErrorMessage());
            }
        }

        inline ComException MakeWin32Exception() {
            auto err = GetLastError();
            HRESULT hr = E_FAIL;

            if (err != ERROR_SUCCESS) {
                hr = HRESULT_FROM_WIN32(err);
                LOG_FAILED(hr);
            }

            return ComException(hr, _com_error(hr).ErrorMessage());
        }
#endif
#if COMPILE_FOR_CX_or_WINRT
        inline void ThrowIfFailed(HRESULT hr) {
            if (FAILED(hr)) {
                LOG_FAILED(hr);
                Dbreak;
                auto comErr = _com_error(hr, nullptr);
                LOG_ERROR_D(L"Com exception = [{:#08x}] [Cx / WinRt]", static_cast<unsigned int>(hr));
                throw comErr;
            }
        }
#endif
#if COMPILE_FOR_WINRT
        namespace WinRt {
            inline void ThrowIfFailed(HRESULT hr) {
                if (FAILED(hr)) {
                    LOG_FAILED(hr);
                    Dbreak;
                    LOG_ERROR_D(L"Com exception = [{:#08x}] [WinRt]", static_cast<unsigned int>(hr));
                    throw ref new Platform::COMException(hr);
                }
            }
        }
#endif
    }
}