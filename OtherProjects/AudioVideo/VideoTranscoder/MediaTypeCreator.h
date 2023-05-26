#pragma once
#include "MFInclude.h"
#include "IAudioCodecSettings.h"
#include "IVideoCodecSettings.h"
#include <stdint.h>

class MediaTypeCreator {
public:
    static Microsoft::WRL::ComPtr<IMFMediaType> CreateAudioInMediaType(const IAudioCodecSettings* settings, uint32_t bitsPerSample);
    static Microsoft::WRL::ComPtr<IMFMediaType> CreateAudioOutMediaType(const IAudioCodecSettings* settings, uint32_t bitsPerSample);
    static Microsoft::WRL::ComPtr<IMFMediaType> CreateAudioAACOutMediaType(const IAudioCodecSettings* settings);

    static Microsoft::WRL::ComPtr<IMFMediaType> CreateVideoInMediaType(const IVideoCodecSettings* settings, bool nv12VideoSamples);
    static Microsoft::WRL::ComPtr<IMFMediaType> CreateVideoOutMediaType(const IVideoCodecSettings* settings);
};