#pragma once
#include "Helpers.h"
#include <Windows.h>
#include <stdexcept>
#include <comdef.h>
#include <memory>


namespace H {
    namespace System {
        class ComException : public std::exception {
        public:
            ComException(HRESULT hr, const std::wstring& message);
            ~ComException();

            ComException(ComException&&) = default;

            std::wstring ErrorMessage() const;
            HRESULT ErrorCode() const;

        private:
            std::wstring errorMessage;
            HRESULT errorCode = S_OK;
        };

        inline void ThrowIfFailed(HRESULT hr) {
            if (FAILED(hr)) {
                throw ComException(hr, _com_error(hr).ErrorMessage());
            }
        }
    }
}