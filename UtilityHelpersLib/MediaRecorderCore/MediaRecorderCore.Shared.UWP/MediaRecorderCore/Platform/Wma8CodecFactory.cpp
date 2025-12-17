#include "pch.h"
#include "Wma8CodecFactory.h"
#include "PlatformHelpers.h"

#include <libhelpers/HSystem.h>

Microsoft::WRL::ComPtr<IMFTransform> Wma8CodecFactory::CreateIMFTransform() {
    return PlatformHelpers::CreateTransform(L".wma",
        [](uint32_t i, uint32_t numChannels, uint32_t sampleRate, uint32_t bitsPerSample)
    {
        return Wma8CodecFactory::CreateOutType(i, numChannels, sampleRate, bitsPerSample);
    });
}

Microsoft::WRL::ComPtr<IMFMediaType> Wma8CodecFactory::CreateOutType(
    uint32_t typeIndex,
    uint32_t numChannels,
    uint32_t sampleRate,
    uint32_t bitsPerSample)
{
    if (typeIndex >= 4) {
        return nullptr;
    }

    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFMediaType> type;

    hr = MFCreateMediaType(type.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    H::System::ThrowIfFailed(hr);

    hr = type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_WMAudioV8);
    H::System::ThrowIfFailed(hr);

    hr = type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, numChannels);
    H::System::ThrowIfFailed(hr);

    hr = type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sampleRate);
    H::System::ThrowIfFailed(hr);

    if (typeIndex >= 1) {
        hr = type->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE);
        H::System::ThrowIfFailed(hr);

        hr = type->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
        H::System::ThrowIfFailed(hr);

        hr = type->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample);
        H::System::ThrowIfFailed(hr);
    }

    if (typeIndex >= 2) {
        WMAUDIO2WAVEFORMAT wma2fmt = {};

        hr = type->SetUINT32(MF_MT_AUDIO_PREFER_WAVEFORMATEX, TRUE);
        H::System::ThrowIfFailed(hr);

        // typeIndex 2 and 3:
        // valid values for 2 channel 16 bit audio with 44100 sample rate
        // taken from IMFTransform from MFTEnum

        switch (typeIndex) {
        case 2:
            hr = type->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 6003);
            H::System::ThrowIfFailed(hr);

            hr = type->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 2230);
            H::System::ThrowIfFailed(hr);

            wma2fmt.wEncodeOptions = 31;
            break;
        case 3:
            hr = type->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 40001);
            H::System::ThrowIfFailed(hr);

            hr = type->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 14861);
            H::System::ThrowIfFailed(hr);

            wma2fmt.wEncodeOptions = 15;
            break;
        default:
            break;
        }

        switch (typeIndex) {
        case 2:
        case 3:
            wma2fmt.dwSamplesPerBlock = 34816;
            wma2fmt.dwSuperBlockAlign = 0;
            break;
        default:
            break;
        }

        const auto userDataExpectedSize = (sizeof WMAUDIO2WAVEFORMAT - sizeof WAVEFORMATEX);
        const auto userDataOffset = sizeof WAVEFORMATEX;

        uint8_t *bytes = reinterpret_cast<uint8_t*>(&wma2fmt);
        bytes += userDataOffset;

        hr = type->SetBlob(MF_MT_USER_DATA, bytes, userDataExpectedSize);
        H::System::ThrowIfFailed(hr);
    }

    return type;
}