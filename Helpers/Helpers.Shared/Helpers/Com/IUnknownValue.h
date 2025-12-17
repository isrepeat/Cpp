#pragma once
#include <Helpers/common.h>
#include <Unknwn.h>

namespace HELPERS_NS {
    namespace Com {
        // [Guid("F646CF6A-9552-4BAA-B518-3E33A12A4C11")]
        MIDL_INTERFACE("F646CF6A-9552-4BAA-B518-3E33A12A4C11")
            IUnknownValue : public IUnknown{
            public:
                virtual HRESULT STDMETHODCALLTYPE Copy(
                    __RPC__out void* dest) = 0;

                virtual HRESULT STDMETHODCALLTYPE Move(
                    __RPC__out void* dest) = 0;
        };
    }
}