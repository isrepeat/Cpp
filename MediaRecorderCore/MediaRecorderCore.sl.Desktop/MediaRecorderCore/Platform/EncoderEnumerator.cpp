#include "pch.h"
#include "EncoderEnumerator.h"

#include <vector>
#include <libhelpers/HSystem.h>
#include <libhelpers/MediaFoundation/MFInclude.h>
#include <libhelpers/CoUniquePtr.h>
#include <libhelpers/Containers/ComPtrArray.h>

void EncoderEnumerator::EnumAudio(std::function<void(const GUID &)> fn) {
    EnumMFTGuids(MFT_CATEGORY_AUDIO_ENCODER, fn);
}

void EncoderEnumerator::EnumVideo(std::function<void(const GUID &)> fn) {
    EnumMFTGuids(MFT_CATEGORY_VIDEO_ENCODER, fn);
}

template<class Fn>
void EncoderEnumerator::EnumMFTGuids(const GUID &category, Fn fn) {
    HRESULT hr = S_OK;
    std::vector<GUID> mftGuids;
    ComPtrArray<IMFActivate, CoDeleter<IMFActivate>> activators;

    hr = MFTEnumEx(category, MFT_ENUM_FLAG_ALL, nullptr, nullptr, activators.GetAddressOf(), activators.GetAddressOfSize());
    H::System::ThrowIfFailed(hr);

    for (auto &i : activators) {
        Microsoft::WRL::ComPtr<IMFTransform> transform;

        hr = i->ActivateObject(IID_PPV_ARGS(transform.GetAddressOf()));
        if (FAILED(hr)) {
            continue;
        }

        Microsoft::WRL::ComPtr<IMFMediaType> type;

        hr = transform->GetOutputAvailableType(0, 0, type.GetAddressOf());
        if (hr != S_OK) {
            continue;
        }

        GUID subtypeGuid;

        hr = type->GetGUID(MF_MT_SUBTYPE, &subtypeGuid);
        if (hr != S_OK) {
            continue;
        }

        fn(subtypeGuid);
    }
}