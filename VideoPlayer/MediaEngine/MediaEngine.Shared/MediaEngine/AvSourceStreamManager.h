#pragma once
#include "IAvReaderEffect.h"
#include "Audio/AudioWaveFormat.h"
#include "AvStreamType.h"
#include "MediaSample.h"

#include <Helpers/MediaFoundation/MFInclude.h>
#include <Helpers/ThreadSafeObject.hpp>
#include <Helpers/Rational.h>
#include <Helpers/Memory.h>
#include <Helpers/Math.h>

#include <ppltasks.h>
#include <cstdint>
#include <queue>


enum class TextureType : int {
    Unknown = -1,

    I420,
    NV12,
    RGB32,
    ARGB32 = RGB32,
    ABGR32,
    R8,

    Count
};


class AvStreamBase {
public:
    AvStreamBase(AvStreamType streamType, int32_t streamIdx);
    virtual ~AvStreamBase() = default;

    AvStreamType GetStreamType() const;
    int32_t GetStreamIdx() const;

private:
    AvStreamType streamType;
    int32_t streamIdx;
};

//class AvStreamVideoSamplesQueue {
//public:
//    void Push(std::unique_ptr<MFVideoSample> videoSample) {
//        std::lock_guard lk{ mx };
//        videoSamples.push(std::move(videoSample));
//    }
//
//    std::unique_ptr<MFVideoSample> Pop() {
//        std::lock_guard lk{ mx };
//        std::unique_ptr<MFVideoSample> videoSample;
//        if (videoSamples.size() > 0) {
//            videoSample = std::move(videoSamples.front());
//            videoSamples.pop();
//        }
//        return videoSample;
//    }
//
//    size_t GetSamplesCount() {
//        std::lock_guard lk{ mx };
//        return videoSamples.size();
//    }
//
//    void ClearSamples() {
//        std::lock_guard lk{ mx };
//        videoSamples = {};
//    }
//
//private:
//    std::mutex mx;
//    std::queue<std::unique_ptr<MFVideoSample>> videoSamples;
//};



class AvStreamSamplesProcessedQueue {
public:
    void AddPushRequest();
    void Push(std::unique_ptr<MF::MFSample> sample);
    void Process(IAvReaderEffect* streamEffect);
    MF::MFSample* Peek();
    std::unique_ptr<MF::MFSample> Pop();

    int GetSamplesRequests();
    size_t GetRawSamplesCount();
    size_t GetProcessedSamplesCount();

    void ClearSamples();

protected:
    virtual std::unique_ptr<MF::MFSample> ProcessSample(std::unique_ptr<MF::MFSample> mfSample);

private:
    std::mutex mxSamplesRaw;
    std::mutex mxSamplesProcessed;
    std::queue<std::unique_ptr<MF::MFSample>> samplesRaw;
    std::queue<std::unique_ptr<MF::MFSample>> samplesProcessed;
    std::atomic<int> requestedSamples = 0;

protected:
    int64_t prevSampleEnd = -1;
};



class AvStreamVideo : public AvStreamBase, public AvStreamSamplesProcessedQueue {
public:
    AvStreamVideo(
        int32_t streamIdx,
        H::Size size,
        TextureType textureType,
        H::Rational<float> frameRate,
        uint32_t avgBitRate,
        int rotation,
        bool isImage,
        std::unique_ptr<IAvReaderEffect>&& effectList);

    H::Size GetSize() const;
    TextureType GetTextureType() const;
    H::Rational<float> GetFrameRate() const;
    uint32_t GetAvgBitRate() const;
    int GetRotation() const;
    bool IsImage() const;
    IAvReaderEffect* GetEffect() const;

private:
    TextureType textureType;
    H::Size size;
    H::Rational<float> frameRate;
    uint32_t avgBitRate;
    int rotation;
    bool isImage;
    std::unique_ptr<IAvReaderEffect> effectList;
};




class AvStreamAudio : public AvStreamBase, public AvStreamSamplesProcessedQueue {
public:
    AvStreamAudio(
        int32_t streamIdx,
        size_t channelNum,
        uint32_t sampleRate,
        uint32_t avgBytesPerSecond,
        std::unique_ptr<IAvReaderEffect>&& effectList);

    void SetOriginalAudioChannelNum(size_t channelNumOriginal);
    void SetOriginAudioSampleRate(uint32_t sampleRateOriginal);

    size_t GetChannelNumOriginal() const;
    uint32_t GetSampleRateOriginal() const;
    size_t GetChannelNum() const;
    uint32_t GetSampleRate() const;
    uint32_t GetAvgBytesPerSecond() const;
    IAvReaderEffect* GetEffect() const;

private:
    std::unique_ptr<MF::MFSample> ProcessSample(std::unique_ptr<MF::MFSample> mfSample) override;
    std::unique_ptr<MF::MFSample> CorrectSamlpePts(std::unique_ptr<MF::MFSample> mfAudioSample);

private:
    size_t channelNum;
    size_t channelNumOriginal;
    uint32_t sampleRate;
    uint32_t sampleRateOriginal;
    uint32_t avgBytesPerSecond;
    std::unique_ptr<IAvReaderEffect> effectList;
};



class AvSourceStreamManager {
public:
    // TODO: rewrite with WeakRef
    // TODO: write wrapper under sourceReader that will lock access to IMFSourceReader api and use it here and in AvReader
    AvSourceStreamManager(Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader);

    void AddStream(std::unique_ptr<AvStreamBase> stream);
    size_t GetStreamCount(AvStreamType streamType);

    AvStreamVideo* GetVideoStream(int32_t streamIdx);
    AvStreamAudio* GetAudioStream(int32_t streamIdx);

    AvStreamVideo* GetFirstVideoStream();
    AvStreamAudio* GetFirstAudioStream();

    
    void DisableAllStreams();
    void DisableStream(AvStreamType streamType, int32_t streamIdx);

    void SetActiveStream(AvStreamBase* stream);
    void SetActiveStream(AvStreamType streamType, int32_t streamIdx);

    AvStreamBase* GetActiveStream(AvStreamType streamType);
    AvStreamVideo* GetActiveVideoStream();
    AvStreamAudio* GetActiveAudioStream();

    AudioWaveFormat GetWaveFormatForActiveAudioStream();

private:
    //
    // Internal implementaitions
    //
    void AddStream(H::MethodIsGuarded lk, std::unique_ptr<AvStreamBase> stream);
    size_t GetStreamCount(H::MethodIsGuarded lk, AvStreamType streamType);

    AvStreamVideo* GetVideoStream(H::MethodIsGuarded lk, int32_t streamIdx);
    AvStreamAudio* GetAudioStream(H::MethodIsGuarded lk, int32_t streamIdx);

    AvStreamVideo* GetFirstVideoStream(H::MethodIsGuarded lk);
    AvStreamAudio* GetFirstAudioStream(H::MethodIsGuarded lk);


    void DisableAllStreams(H::MethodIsGuarded lk);
    void DisableStream(H::MethodIsGuarded lk, AvStreamType streamType, int32_t streamIdx);

    void SetActiveStream(H::MethodIsGuarded lk, AvStreamBase* stream);
    void SetActiveStream(H::MethodIsGuarded lk, AvStreamType streamType, int32_t streamIdx);

    AvStreamBase* GetActiveStream(H::MethodIsGuarded lk, AvStreamType streamType);
    AvStreamVideo* GetActiveVideoStream(H::MethodIsGuarded lk);
    AvStreamAudio* GetActiveAudioStream(H::MethodIsGuarded lk);

    AudioWaveFormat GetWaveFormatForActiveAudioStream(H::MethodIsGuarded lk);

private:
    std::mutex mx;
    Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader;
    std::vector<std::unique_ptr<AvStreamVideo>> videoStreams;
    std::vector<std::unique_ptr<AvStreamAudio>> audioStreams;
    AvStreamVideo* activeVideoStream = nullptr;
    AvStreamAudio* activeAudioStream = nullptr;
};