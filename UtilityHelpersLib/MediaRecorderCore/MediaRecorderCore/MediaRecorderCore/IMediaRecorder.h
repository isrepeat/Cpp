#pragma once
#include <Helpers/MediaFoundation/SampleInfo.h>
#include <cstdint>
#include <wrl.h>
#include <mfapi.h>
#include <vector>
#include <string>

// https://accu.org/journals/overload/27/152/boger_2683/
// use strong types to enforce errors in dependent code to check if use of UseHardwareTransformsForEncoding right in dependent code context
enum class UseHardwareTransformsForEncoding : bool {};
enum class UseNv12VideoSamples : bool {};

class IMediaRecorder {
public:
    IMediaRecorder() {}
    virtual ~IMediaRecorder() {}

    virtual bool HasAudio() const = 0;
    virtual bool HasVideo() const = 0;

    virtual int64_t LastVideoPtsHns() const = 0;
    virtual int64_t LastAudioPtsHns() const = 0;
    // default implementation is to return max(LastVideoPtsHns(), LastAudioPtsHns())
    virtual int64_t LastPtsHns() const = 0;

    virtual MF::SampleInfo LastWritedAudioSample() const = 0;
    virtual MF::SampleInfo LastWritedVideoSample() const = 0;

    virtual bool ChunkAudioSamplesWritten() const = 0;
    virtual bool ChunkVideoSamplesWritten() const = 0;

    virtual bool IsChunkMergerEnabled() = 0;

    virtual void StartRecord() = 0;
    // <audio> == true for audio sample; <audio> == false for video sample
    virtual void Record(const Microsoft::WRL::ComPtr<IMFSample> &sample, bool audio) = 0;
    virtual void EndRecord() = 0;
    virtual void Restore(IMFByteStream* outputStream, std::vector<std::wstring>&& chunks) = 0;
};
