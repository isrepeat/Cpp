#include "System.h"

namespace H {
    namespace System {
        ComException::ComException(HRESULT hr, const std::wstring& message)
            : std::exception(H::WStrToStr(message).c_str())
            , errorMessage{ message }
            , errorCode{ hr }
        {
        }

        ComException::~ComException() = default; // declare in cpp explicitly to allow compile unique_ptr with pimpl

        std::wstring ComException::ErrorMessage() const {
            return errorMessage;
        }

        HRESULT ComException::ErrorCode() const {
            return errorCode;
        }
    }
}