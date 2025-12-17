#include "pch.h"
#include "PlatformHelpers.h"

#include <libhelpers/HSystem.h>

// this values expected by some types inside Wma8CodecFactory::CreateOutType
const uint32_t PlatformHelpers::DefaultAudioNumChannels = 2;
const uint32_t PlatformHelpers::DefaultAudioSampleRate = 44100;
const uint32_t PlatformHelpers::DefaultAudioBitsPerSample = 16;

const uint32_t PlatformHelpers::DefaultAudioBitrate = 128000;

Microsoft::WRL::ComPtr<IMFSinkWriter> PlatformHelpers::CreateMemorySinkWriter(const wchar_t *url, IMFAttributes *attr) {
    auto mfStream = PlatformHelpers::CreateMemoryStream();
    Microsoft::WRL::ComPtr<IMFSinkWriter> sinkWriter;
    HRESULT hr = S_OK;

    hr = MFCreateSinkWriterFromURL(url, mfStream.Get(), attr, sinkWriter.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    return sinkWriter;
}

Microsoft::WRL::ComPtr<IMFByteStream> PlatformHelpers::CreateMemoryStream() {
    auto tmpStream = ref new Windows::Storage::Streams::InMemoryRandomAccessStream();
    auto res = PlatformHelpers::WrapStreamForMediaFoundation(tmpStream);
    return res;
}

Microsoft::WRL::ComPtr<IMFByteStream> PlatformHelpers::WrapStreamForMediaFoundation(Windows::Storage::Streams::IRandomAccessStream ^stream) {
    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFByteStream> mfStream;
    IUnknown *tmpStream2 = reinterpret_cast<IUnknown*>(stream);

    hr = MFCreateMFByteStreamOnStreamEx(tmpStream2, mfStream.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    return mfStream;
}

Microsoft::WRL::ComPtr<IMFMediaType> PlatformHelpers::CreateInType(
    uint32_t numChannels,
    uint32_t sampleRate,
    uint32_t bitsPerSample)
{
    HRESULT hr = S_OK;
    Microsoft::WRL::ComPtr<IMFMediaType> type;

    hr = MFCreateMediaType(type.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    hr = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    H::System::ThrowIfFailed(hr);

    hr = type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    H::System::ThrowIfFailed(hr);

    hr = type->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample);
    H::System::ThrowIfFailed(hr);

    hr = type->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, numChannels);
    H::System::ThrowIfFailed(hr);

    hr = type->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sampleRate);
    H::System::ThrowIfFailed(hr);

    hr = type->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, (bitsPerSample / 8) * numChannels * sampleRate);
    H::System::ThrowIfFailed(hr);

    hr = type->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, (bitsPerSample / 8) * numChannels);
    H::System::ThrowIfFailed(hr);

    hr = type->SetUINT32(MF_MT_COMPRESSED, FALSE);
    H::System::ThrowIfFailed(hr);

    hr = type->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
    H::System::ThrowIfFailed(hr);

    return type;
}