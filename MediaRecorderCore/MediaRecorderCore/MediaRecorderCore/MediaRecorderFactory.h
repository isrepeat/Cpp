#pragma once
#include "IMediaRecorderFactory.h"
#include <libhelpers/Event.hpp>
#include "MediaRecorderParams.h"
#include "MediaRecorderMessageEnum.h"

#include <vector>

class MediaRecorderFactory : public IMediaRecorderFactory {
public:
    MediaRecorderFactory(
        IMFByteStream* outputStream,
        MediaRecorderParams params,
        UseHardwareTransformsForEncoding hardwareTransformsForEncoding,
        UseNv12VideoSamples nv12VideoSamples,
        std::shared_ptr<IEvent<Native::MediaRecorderEventArgs>> recordEventCallback);

    std::unique_ptr<IMediaRecorder> CreateMediaRecorder() override;

    static std::vector<uint32_t> GetAvailableAudioNumChannels();
    static std::vector<uint32_t> GetAvailableAudioSampleRate();
    static std::vector<uint32_t> GetAvailableAudioBitsPerSecond();

private:
    // hardwareTransformsForEncoding by default true because it reduces memory usage
    UseHardwareTransformsForEncoding hardwareTransformsForEncoding = UseHardwareTransformsForEncoding{ true };
    UseNv12VideoSamples nv12VideoSamples = UseNv12VideoSamples{ false };
    MediaRecorderParams params;
    Microsoft::WRL::ComPtr<IMFByteStream> outputStream;
    std::shared_ptr<IEvent<Native::MediaRecorderEventArgs>> recordEventCallback;
};
