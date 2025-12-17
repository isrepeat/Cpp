#pragma once
#include "IMediaRecorder.h"
#include "MediaRecorderParams.h"
#include "MediaRecorderMessageEnum.h"

#include <cstdint>
#include <array>
#include <limits>
#include <libhelpers\Event.hpp>
#include <libhelpers\Macros.h>
#include <libhelpers\MediaFoundation\MFUser.h>
#include "ChunkMerger.h"

// default profile of H264 can fail on sink->Finalize with video bitrate > 80 mbits.
// Old setting bool useCPUForEncoding is implicitly enabled by use of this->params.DxBufferFactory
// New setting UseHardwareTransformsForEncoding can be used with or without this->params.DxBufferFactory
class MediaRecorder : public MFUser, public IMediaRecorder {
public:
    static constexpr uint32_t AudioSampleBits = 16;

    NO_COPY(MediaRecorder);

    static const uint32_t AllowedNumChannels[2];
    static const uint32_t AllowedSampleRate[2];
    static const uint32_t AllowedBytesPerSecond[4];

    MediaRecorder() = default;
    MediaRecorder(
        IMFByteStream* outputStream,
        MediaRecorderParams params,
        UseHardwareTransformsForEncoding hardwareTransformsForEncoding,
        UseNv12VideoSamples nv12VideoSamples,
        std::shared_ptr<IEvent<Native::MediaRecorderEventArgs>> recordEventCallback = nullptr);

    MediaRecorder(MediaRecorder&&) = default;

    MediaRecorder &operator=(MediaRecorder&&) = default;

    // IMediaRecorder
    bool HasAudio() const override;
    bool HasVideo() const override;

    int64_t LastAudioPtsHns() const override;
    int64_t LastVideoPtsHns() const override;
    int64_t LastPtsHns() const override;

    MF::SampleInfo LastWritedAudioSample() const override;
    MF::SampleInfo LastWritedVideoSample() const override;

    bool ChunkAudioSamplesWritten() const override;
    bool ChunkVideoSamplesWritten() const override;

    bool IsChunkMergerEnabled() override;

    void StartRecord() override;
    void Record(const Microsoft::WRL::ComPtr<IMFSample> &sample, bool audio) override;
    void RecordVideoSample(const Microsoft::WRL::ComPtr<IMFSample> &sample);
    void RecordAudioBuffer(const float* audioSamples, size_t samplesCountForAllChannels);
    void EndRecord() override;

    void Restore(IMFByteStream* outputStream, std::vector<std::wstring>&& chunks) override;

    const MediaRecorderParams &GetParams() const;

    Microsoft::WRL::ComPtr<IMFMediaType> GetVideoTypeIn() const;
    Microsoft::WRL::ComPtr<IMFMediaType> GetVideoTypeOut() const;

    Microsoft::WRL::ComPtr<IMFMediaType> CreateAudioAACOutMediaType();
    Microsoft::WRL::ComPtr<IMFMediaType> CreateAudioFlacOutMediaType();

    [[deprecated("Write(const float* audioSamples, size_t valuesCount, int64_t hns) is deprecated, use RecordAudioBuffer(...) instead")]]
    void Write(const float* audioSamples, size_t valuesCount, int64_t hns = -1);
    // without sample allocator it can cause memory leak
    void Write(ID3D11DeviceContext* d3dCtx, ID3D11Texture2D* tex, int64_t hns = -1, int64_t durationHns = -1);
    // without sample allocator it can cause memory leak
    void Write(const void* videoData, size_t rowPitch, int64_t hns = -1, int64_t durationHns = -1);

    static Microsoft::WRL::ComPtr<IMFMediaType> CreateAudioInMediaType(
        const IAudioCodecSettings* settings,
        uint32_t bitsPerSample);
    static Microsoft::WRL::ComPtr<IMFMediaType> CreateAudioOutMediaType(
        const IAudioCodecSettings* settings,
        uint32_t bitsPerSample);
    static Microsoft::WRL::ComPtr<IMFMediaType> CreateVideoInMediaType(
        const IVideoCodecSettings* settings, UseNv12VideoSamples nv12VideoSamples);
    static Microsoft::WRL::ComPtr<IMFMediaType> CreateVideoOutMediaType(
        const IVideoCodecSettings* settings, MediaContainerType containerType, bool useChunkMerger);

private:
    Microsoft::WRL::ComPtr<IMFByteStream> stream;
    const MediaRecorderParams params;

    DWORD audioStreamIdx = MediaRecorder::GetInvalidStreamIdx();
    DWORD videoStreamIdx = MediaRecorder::GetInvalidStreamIdx();
    Microsoft::WRL::ComPtr<IMFSinkWriter> sinkWriter;

    int64_t audioPtsHns = 0;
    int64_t videoPtsHns = 0;
    
    std::optional<MF::SampleInfo> lastWritedAudioSample;
    std::optional<MF::SampleInfo> lastWritedVideoSample;

    // hardwareTransformsForEncoding by default true because it reduces memory usage
    UseHardwareTransformsForEncoding hardwareTransformsForEncoding = UseHardwareTransformsForEncoding{ true };
    UseNv12VideoSamples nv12VideoSamples = UseNv12VideoSamples{ false };
    const std::wstring containerExt;
    int64_t chunkAudioPtsHns = 0;
    int64_t chunkVideoPtsHns = 0;
    int samplesNumber = 0;
    int framesNumber = 0;
    bool recordingErrorOccured = false;

    std::wstring chunkFile;
    std::wstring chunksDisk;
    std::wstring targetRecordDisk;
    uint64_t recordedChunksSize = 0;
    uint64_t lastChunkCreatedTime = 0;

    uint32_t chunkNumber = 0;
    Microsoft::WRL::ComPtr<IMFByteStream> currentOutputStream;

    Microsoft::WRL::ComPtr<IMFMediaType> videoTypeOut, videoTypeIn;

    std::shared_ptr<IEvent<Native::MediaRecorderEventArgs>> recordEventCallback;

    void InitializeSinkWriter(
        IMFByteStream *outputStream,
        UseHardwareTransformsForEncoding hardwareTransformsForEncoding,
        UseNv12VideoSamples nv12VideoSamples);

    static Microsoft::WRL::ComPtr<IMFMediaType> GetBestOutputType(
        IMFTransform *transform,
        IMFMediaType *defaultType,
        const GUID &subtypeGuid,
        uint32_t numChannels,
        uint32_t sampleRate,
        uint32_t bitsPerSample,
        uint32_t bitrate);

    void WriteVideoSample(const Microsoft::WRL::ComPtr<IMFMediaBuffer> &buffer, int64_t hns = -1, int64_t durationHns = -1);
    void WriteSample(const Microsoft::WRL::ComPtr<IMFMediaBuffer> &buffer, int64_t positionHNS, int64_t durationHNS, DWORD streamIndex);
    void WriteSample(const Microsoft::WRL::ComPtr<IMFSample> &sample, DWORD streamIndex);

    int64_t GetDefaultVideoFrameDuration() const;

    IMFByteStream* StartNewChunk();
    void ResetSinkWriterOnNewChunk();
    void FinalizeRecord(bool useRecordEventCallback);
    void MergeChunks(IMFByteStream* outputStream, std::vector<std::wstring>&& chunks);
    std::wstring GetChunkFilePath(size_t chunkIndex);

    static constexpr DWORD GetInvalidStreamIdx();
};
