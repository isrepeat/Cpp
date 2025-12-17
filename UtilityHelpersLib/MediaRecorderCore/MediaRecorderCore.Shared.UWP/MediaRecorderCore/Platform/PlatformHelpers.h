#pragma once

#include <cstdint>
#include <libhelpers/HSystem.h>
#include <libhelpers/MediaFoundation/MFInclude.h>

class PlatformHelpers {
public:
    static const uint32_t DefaultAudioNumChannels;
    static const uint32_t DefaultAudioSampleRate;
    static const uint32_t DefaultAudioBitsPerSample;
    static const uint32_t DefaultAudioBitrate;

    static Microsoft::WRL::ComPtr<IMFSinkWriter> CreateMemorySinkWriter(const wchar_t *url, IMFAttributes *attr = nullptr);
    static Microsoft::WRL::ComPtr<IMFByteStream> CreateMemoryStream();
    static Microsoft::WRL::ComPtr<IMFByteStream> WrapStreamForMediaFoundation(Windows::Storage::Streams::IRandomAccessStream ^stream);

    static Microsoft::WRL::ComPtr<IMFMediaType> CreateInType(
        uint32_t numChannels,
        uint32_t sampleRate,
        uint32_t bitsPerSample);

	// getInParams(uint32_t &numChannels, uint32_t &sampleRate, uint32_t &bitsPerSample)
    // createOutType(int typeIndex, uint32_t numChannels, uint32_t sampleRate, uint32_t bitsPerSample)
    template<class FnGetInParams, class Fn>
    static Microsoft::WRL::ComPtr<IMFTransform> CreateTransform2(const wchar_t *url, FnGetInParams getInParams, Fn createOutType) {
        /*
    CoCreateInstance doesn't work under UWP.
    In order to get transform we need to create IMFSinkWriter and get IMFTransform through IMFSinkWriterEx interface.
    Parameters of transform isn't very important because this method must only to return transform with desired codec.
    */

		uint32_t numChannels = 0;
		uint32_t sampleRate = 0;
		uint32_t bitsPerSample = 0;

		getInParams(numChannels, sampleRate, bitsPerSample);

        auto typeIn = PlatformHelpers::CreateInType(numChannels, sampleRate, bitsPerSample);

        for (uint32_t i = 0; ; i++) {
            auto typeOut = createOutType(i, numChannels, sampleRate, bitsPerSample);
            if (!typeOut) {
                break;
            }

            HRESULT hr = S_OK;
            DWORD audioStreamIdx = 0;
            Microsoft::WRL::ComPtr<IMFSinkWriter> sinkWriter = PlatformHelpers::CreateMemorySinkWriter(url);

            hr = sinkWriter->AddStream(typeOut.Get(), &audioStreamIdx);
            if (FAILED(hr)) {
                continue;
            }

            hr = sinkWriter->SetInputMediaType(audioStreamIdx, typeIn.Get(), NULL);
            if (FAILED(hr)) {
                continue;
            }

            Microsoft::WRL::ComPtr<IMFSinkWriterEx> sinkWriterEx;

            hr = sinkWriter.As(&sinkWriterEx);
            if (FAILED(hr)) {
                continue;
            }

            for (DWORD idx = 0; ; idx++) {
                Microsoft::WRL::ComPtr<IMFTransform> transform;
                GUID cat = GUID_NULL;

                hr = sinkWriterEx->GetTransformForStream(audioStreamIdx, idx, &cat, transform.GetAddressOf());
                if (hr != S_OK) {
                    break;
                }

                if (cat == MFT_CATEGORY_AUDIO_ENCODER) {
                    return transform;
                }
            }
        }

        return nullptr;
    }

	// createOutType(int typeIndex, uint32_t numChannels, uint32_t sampleRate, uint32_t bitsPerSample)
	template<class Fn>
	static Microsoft::WRL::ComPtr<IMFTransform> CreateTransform(const wchar_t *url, Fn createOutType) {
		return CreateTransform2(url,
			[](uint32_t &numChannels, uint32_t &sampleRate, uint32_t &bitsPerSample)
		{
			numChannels = PlatformHelpers::DefaultAudioNumChannels;
			sampleRate = PlatformHelpers::DefaultAudioSampleRate;
			bitsPerSample = PlatformHelpers::DefaultAudioBitsPerSample;
		},
			createOutType);
	}
};