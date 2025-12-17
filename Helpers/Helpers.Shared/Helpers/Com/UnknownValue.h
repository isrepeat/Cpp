#pragma once
#include <Helpers/common.h>
#include "IUnknownValue.h"
#include <memory>
#include <wrl.h>

namespace HELPERS_NS {
    namespace Com {
        template<class T>
        class UnknownValue : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<
            Microsoft::WRL::RuntimeClassType::WinRtClassicComMix>,
            IUnknownValue>
        {
        public:
            UnknownValue() {}
            virtual ~UnknownValue() {
                int xx = 9;
            }

            HRESULT STDMETHODCALLTYPE Copy(
                __RPC__out void* dest) override
            {
                HRESULT hr = S_OK;
                auto tmpDest = static_cast<T*>(dest);

                *tmpDest = this->value;

                return hr;
            }

            HRESULT STDMETHODCALLTYPE Move(
                __RPC__out void* dest) override
            {
                HRESULT hr = S_OK;
                auto tmpDest = static_cast<T*>(dest);

                *tmpDest = std::move(this->value);

                return hr;
            }

            void SetValue(T val) {
                this->value = std::move(val);
            }

        private:
            T value;
        };


        ///// <summary>
        ///// casts IUnknown to WinRT object. Reference count of IUnknown incremented(internally when assigning to WinRT object).
        ///// </summary>
        //template<class T>
        //T com_cx_cast(IUnknown* val) {
        //    T result;
        //    Platform::Object^ tmp = reinterpret_cast<Platform::Object^>(val);

        //    result = dynamic_cast<T>(tmp);

        //    return result;
        //}

        ///// <summary>
        ///// casts COM object to WinRT object. Reference count of COM object incremented(internally when assigning to WinRT object).
        ///// </summary>
        //template<class T, class T2>
        //T com_cx_cast(const Microsoft::WRL::ComPtr<T2>& v)
        //{
        //    HRESULT hr = S_OK;
        //    T result;
        //    Platform::Object^ obj;
        //    Microsoft::WRL::ComPtr<IInspectable> insp;

        //    hr = v.As(&insp);
        //    if (FAILED(hr)) {
        //        return nullptr;
        //    }

        //    obj = reinterpret_cast<Platform::Object^>(insp.Get());
        //    result = dynamic_cast<T>(obj);

        //    return result;
        //}
    }
}