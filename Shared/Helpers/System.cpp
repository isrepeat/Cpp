#include "System.h"
//#include "Logger.h"


namespace H {
    namespace System {
        ComException::ComException(HRESULT hr, const std::wstring& message)
            : std::exception(H::WStrToStr(message).c_str())
            , errorMessage{message}
            , errorCode{hr}
        {
            //LOG_ERROR_D("Com exception = [{:#08x}] {}", static_cast<unsigned int>(hr), H::WStrToStr(message));
        }

        std::wstring ComException::ErrorMessage() const {
            return errorMessage;
        }

        HRESULT ComException::ErrorCode() const {
            return errorCode;
        }
    }
}