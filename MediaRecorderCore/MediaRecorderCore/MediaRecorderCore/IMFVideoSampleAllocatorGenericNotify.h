#pragma once

#include <libhelpers\MediaFoundation\MFInclude.h>
#include <algorithm>

template<class Fn>
class IMFVideoSampleAllocatorGenericNotify :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<
    Microsoft::WRL::RuntimeClassType::ClassicCom>,
    IMFVideoSampleAllocatorNotify>
{
public:
    IMFVideoSampleAllocatorGenericNotify(Fn fn)
        : fn(std::move(fn))
    {}

    HRESULT STDMETHODCALLTYPE NotifyRelease() override {
        this->fn();
        return S_OK;
    }

private:
    Fn fn;
};

template<class Fn>
Microsoft::WRL::ComPtr<IMFVideoSampleAllocatorGenericNotify<Fn>> MakeIMFVideoSampleAllocatorGenericNotify(Fn fn) {
    auto obj = Microsoft::WRL::Make<IMFVideoSampleAllocatorGenericNotify<Fn>>(std::move(fn));
    return obj;
}