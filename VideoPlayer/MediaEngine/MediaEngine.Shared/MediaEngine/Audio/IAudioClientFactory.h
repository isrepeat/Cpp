#pragma once
#include <Audioclient.h>
#include <wrl.h>

enum AudioClientType {
    RenderAudioClient,
    CaptureAudioClient
};

class IAudioClientFactory {
public:
    IAudioClientFactory() = default;
    virtual ~IAudioClientFactory() = default;

    virtual Microsoft::WRL::ComPtr<IAudioClient> CreateAudioClient(AudioClientType type) = 0;
};