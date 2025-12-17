#include "pch.h"
#include "AmrNbCodecFactory.h"
#include "PlatformHelpers.h"

#include <libhelpers/HSystem.h>

Microsoft::WRL::ComPtr<IMFTransform> AmrNbCodecFactory::CreateIMFTransform() {
    return PlatformHelpers::CreateTransform(L".3gpp",
        [](uint32_t i, uint32_t numChannels, uint32_t sampleRate, uint32_t bitsPerSample)
    {
        return AmrNbCodecFactory::CreateOutType(i, numChannels, sampleRate, bitsPerSample);
    });
}

Microsoft::WRL::ComPtr<IMFMediaType> AmrNbCodecFactory::CreateOutType(
    uint32_t typeIndex,
    uint32_t /*numChannels*/,
    uint32_t /*sampleRate*/,
    uint32_t /*bitsPerSample*/)
{
    if (typeIndex >= 1) {
        return nullptr;
    }

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFMediaType> type;

    hr = MFCreateMediaType(type.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    H::System::ThrowIfFailed(hr);

    hr = type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AMR_NB);
    H::System::ThrowIfFailed(hr);

    hr = type->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 1525);
    H::System::ThrowIfFailed(hr);

    hr = type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 1);
    H::System::ThrowIfFailed(hr);

    hr = type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 8000);
    H::System::ThrowIfFailed(hr);

    hr = type->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 2);
    H::System::ThrowIfFailed(hr);

    hr = type->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
    H::System::ThrowIfFailed(hr);

    /*if (typeIndex >= 1) {
        hr = type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 1);
        H::System::ThrowIfFailed(hr);

        hr = type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 8000);
        H::System::ThrowIfFailed(hr);

        hr = type->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 2);
        H::System::ThrowIfFailed(hr);

        hr = type->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
        H::System::ThrowIfFailed(hr);
    }*/

    return type;
}

/*
            0 :
                MF_MT_AUDIO_AVG_BYTES_PER_SECOND = 1525
                MF_MT_AUDIO_BLOCK_ALIGNMENT = 2
                MF_MT_AUDIO_NUM_CHANNELS = 1
                MF_MT_MAJOR_TYPE = MFMediaType_Audio
                MF_MT_AUDIO_SAMPLES_PER_SECOND = 8000
                MF_MT_AUDIO_BITS_PER_SAMPLE = 16
                MF_MT_SUBTYPE = MFAudioFormat_AMR_NB

            1 :
                MF_MT_AUDIO_AVG_BYTES_PER_SECOND = 1525
                MF_MT_AUDIO_BLOCK_ALIGNMENT = 2
                MF_MT_AUDIO_NUM_CHANNELS = 1
                MF_MT_MAJOR_TYPE = MFMediaType_Audio
                MF_MT_AUDIO_SAMPLES_PER_SECOND = 8000
                MF_MT_AUDIO_BITS_PER_SAMPLE = 16
                MF_MT_SUBTYPE = {73616D72-767A-494D-B478-F29D25DC9037}
            */