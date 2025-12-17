#include "pch.h"
#include "MFGenericAsyncResult.h"

#include <wrl.h>
#include <mfapi.h>
#include "HSystem.h"
#include "MediaFoundation\MFUser.h"

class MFGenericAsyncResult_MFHelper : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<
    Microsoft::WRL::RuntimeClassType::WinRtClassicComMix>, IMFAsyncCallback>, public MFUser
{
public:
    MFGenericAsyncResult_MFHelper(std::function<void(IMFAsyncResult *)> fn, DWORD flags, DWORD queue)
        : fn(fn), flags(flags), queue(queue)
    {}

    virtual ~MFGenericAsyncResult_MFHelper() {
    }

    HRESULT STDMETHODCALLTYPE GetParameters(
        /* [out] */ __RPC__out DWORD *pdwFlags,
        /* [out] */ __RPC__out DWORD *pdwQueue)
    {
        if (pdwFlags) {
            *pdwFlags = this->flags;
        }

        if (pdwQueue) {
            *pdwQueue = this->queue;
        }

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Invoke(
        /* [in] */ __RPC__in_opt IMFAsyncResult *pAsyncResult) override
    {
        HRESULT hr = S_OK;
        this->fn(pAsyncResult);
        return hr;
    }

private:
    std::function<void(IMFAsyncResult *)> fn;
    DWORD flags;
    DWORD queue;
};

MFGenericAsyncResult::MFGenericAsyncResult()
    : callback(nullptr)
{}

MFGenericAsyncResult::MFGenericAsyncResult(std::function<void(IMFAsyncResult *)> fn)
    : callback(nullptr), fn(fn)
{
    auto tmpCallback = Microsoft::WRL::Make<MFGenericAsyncResult_MFHelper>(fn, 0, MFASYNC_CALLBACK_QUEUE_MULTITHREADED);
    this->callback = tmpCallback.Detach();
}

MFGenericAsyncResult::MFGenericAsyncResult(const MFGenericAsyncResult &other)
    : callback(other.callback), fn(other.fn)
{
    this->callback->AddRef();
}

MFGenericAsyncResult::MFGenericAsyncResult(MFGenericAsyncResult &&other)
    : callback(std::move(other.callback)), fn(std::move(other.fn))
{
    other.callback = nullptr;
}

MFGenericAsyncResult::~MFGenericAsyncResult() {
    if (this->callback) {
        this->callback->Release();
        this->callback = nullptr;
    }
}

MFGenericAsyncResult &MFGenericAsyncResult::operator=(const MFGenericAsyncResult &other) {
    if (this != &other) {
        this->callback = other.callback;
        this->fn = other.fn;

        this->callback->AddRef();
    }

    return *this;
    /*__PRETTY_FUNCTION__ */
}

MFGenericAsyncResult &MFGenericAsyncResult::operator=(MFGenericAsyncResult &&other) {
    if (this != &other) {
        this->callback = std::move(other.callback);
        this->fn = std::move(other.fn);

        other.callback = nullptr;
    }

    return *this;
}

void MFGenericAsyncResult::CreateAsyncResult(IMFAsyncResult **res) {
    HRESULT hr = S_OK;

    hr = MFCreateAsyncResult(nullptr, this->callback, nullptr, res);
    H::System::ThrowIfFailed(hr);
}