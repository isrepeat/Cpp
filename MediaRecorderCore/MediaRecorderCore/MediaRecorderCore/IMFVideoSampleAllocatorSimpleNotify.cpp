#include "pch.h"
#include "IMFVideoSampleAllocatorSimpleNotify.h"

#include <libhelpers\HSystem.h>
#include <Helpers/Logger.h>

IMFVideoSampleAllocatorSimpleNotify::IMFVideoSampleAllocatorSimpleNotify(IMFVideoSampleAllocator *allocator)
    : alloc(0), hasSample(0), stop(false)
{
    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFVideoSampleAllocatorCallback> callback;

    hr = allocator->QueryInterface(callback.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = callback->SetCallback(this);
    H::System::ThrowIfFailed(hr);
}

HRESULT STDMETHODCALLTYPE IMFVideoSampleAllocatorSimpleNotify::NotifyRelease() {
    {
        std::lock_guard<std::mutex> lk(this->mtx);
        if (this->alloc) {
            this->hasSample++;
            // just to keep <hasSample> == 0 when <allocThreads> == 0
            this->hasSample = (std::min)(this->hasSample, this->alloc);
        }
    }

    this->cv.notify_one();

    return S_OK;
}

void IMFVideoSampleAllocatorSimpleNotify::NotifyAllocStart() {
    std::lock_guard<std::mutex> lk(this->mtx);
    assert(this->hasSample <= this->alloc);
    // thread entered <alloc> state
    this->alloc++;
}

void IMFVideoSampleAllocatorSimpleNotify::Wait(HRESULT hrAlloc, bool *stopped) {
    std::unique_lock<std::mutex> lk(this->mtx);

    if (hrAlloc != MF_E_SAMPLEALLOCATOR_EMPTY) {
        // thread exited <alloc> state
        this->alloc--;
        assert(this->alloc >= 0);

        if (stopped) {
            // Let caller know if it's the time to stop
            *stopped = this->stop;
        }

        if (this->hasSample) {
            this->hasSample--;
        }

        return;
    }

    LOG_DEBUG("hrAlloc == MF_E_SAMPLEALLOCATOR_EMPTY");

    this->cv.wait(lk, [&] { return this->hasSample || this->stop; });

    if (stopped) {
        *stopped = this->stop;
    }

    if (this->hasSample) {
        this->hasSample--;
    }
}

void IMFVideoSampleAllocatorSimpleNotify::NotifyStop() {
    {
        std::lock_guard<std::mutex> lk(this->mtx);
        this->stop = true;
    }

    this->cv.notify_all();
}

void IMFVideoSampleAllocatorSimpleNotify::ResetStop() {
    std::lock_guard<std::mutex> lk(this->mtx);
    this->stop = false;
}