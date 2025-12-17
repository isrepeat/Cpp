#include "pch.h"
#include "MediaRecorderFactory.h"
#include "MediaRecorder.h"
#include "CodecsTable.h"


MediaRecorderFactory::MediaRecorderFactory(
    IMFByteStream* outputStream,
    MediaRecorderParams params,
    UseHardwareTransformsForEncoding hardwareTransformsForEncoding,
    UseNv12VideoSamples nv12VideoSamples,
    std::shared_ptr<IEvent<Native::MediaRecorderEventArgs>> recordEventCallback)
    : outputStream(outputStream)
    , params(std::move(params))
    , hardwareTransformsForEncoding(hardwareTransformsForEncoding)
    , nv12VideoSamples(nv12VideoSamples)
    , recordEventCallback(recordEventCallback)
{
}

std::unique_ptr<IMediaRecorder> MediaRecorderFactory::CreateMediaRecorder() {
    auto rec = std::make_unique<MediaRecorder>(
        this->outputStream.Get(),
        this->params,
        this->hardwareTransformsForEncoding,
        this->nv12VideoSamples,
        this->recordEventCallback
        );
    return rec;
}

std::vector<uint32_t> MediaRecorderFactory::GetAvailableAudioNumChannels() {
    // As for now the implementation uses static fields but it may be changed if more audio codecs will be added to <MediaRecorder>
    auto params = std::vector<uint32_t>(std::begin(MediaRecorder::AllowedNumChannels), std::end(MediaRecorder::AllowedNumChannels));
    return params;
}

std::vector<uint32_t> MediaRecorderFactory::GetAvailableAudioSampleRate() {
    // As for now the implementation uses static fields but it may be changed if more audio codecs will be added to <MediaRecorder>
    auto params = std::vector<uint32_t>(std::begin(MediaRecorder::AllowedSampleRate), std::end(MediaRecorder::AllowedSampleRate));
    return params;
}

std::vector<uint32_t> MediaRecorderFactory::GetAvailableAudioBitsPerSecond() {
    // As for now the implementation uses static fields but it may be changed if more audio codecs will be added to <MediaRecorder>
    std::vector<uint32_t> params;

    params.reserve(ARRAY_SIZE(MediaRecorder::AllowedBytesPerSecond));

    std::transform(
        std::begin(MediaRecorder::AllowedBytesPerSecond),
        std::end(MediaRecorder::AllowedBytesPerSecond),
        std::back_inserter(params),
        [](uint32_t val)
    {
        // convert to bits
        return val * 8;
    });

    return params;
}
