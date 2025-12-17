#pragma once

#include <mutex>
#include <condition_variable>
#include <libhelpers\MediaFoundation\MFInclude.h>

class IMFVideoSampleAllocatorSimpleNotify :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<
    Microsoft::WRL::RuntimeClassType::ClassicCom>,
    IMFVideoSampleAllocatorNotify>
{
public:
    IMFVideoSampleAllocatorSimpleNotify(IMFVideoSampleAllocator *allocator);

    // Notifies object that someone is going to call <IMFVideoSampleAllocator.AllocateSample>
    // Call this method only if you are going to use <Wait>
    void NotifyAllocStart();

    // Checks <hrAlloc> which is result of the <IMFVideoSampleAllocator.AllocateSample> call and waits if <IMFVideoSampleAllocator> has no samples
    void Wait(HRESULT hrAlloc, bool *stopped = nullptr);

    // After this any thread will not wait
    void NotifyStop();

    // Allows threads to enter waiting
    void ResetStop();

private:
    std::mutex mtx;
    std::condition_variable cv;
    int alloc; // count of <NotifyAllocStart> calls that may enter wait state
    int hasSample; // count of <NotifyRelease> calls when <alloc> != 0
    bool stop;

    // Called by sample when it's released. Must not be called by user
    HRESULT STDMETHODCALLTYPE NotifyRelease() override;
};