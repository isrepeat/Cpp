#pragma once
#include <Helpers/Time.h>
#include "IAudioOutput.h"
#include "IAudioClientFactory.h"

class WASAPIAudioOutput : public IAudioOutput {
public:
    WASAPIAudioOutput(IAudioClientFactory* audioClientFactory, H::Chrono::Hns maxDurationHns);
    virtual ~WASAPIAudioOutput();

    WAVEFORMATEX GetWaveFormat() override;

    std::weak_ptr<IPlaybackClock> GetPlaybackClock() override;

    void AllocateBuffer() override;
    AudioBufferDesc GetBufferDesc() override;
    AudioSampleMixStatus MixAudioSample(MF::MFAudioSample* mfAudioSample, int64_t startPtsHns, size_t sourceCount) override;
    void SubmitBuffer() override;

    void Play() override;
    void Pause() override;
    void Stop() override;
    void Reset() override;

private:
    WAVEFORMATEXTENSIBLE GetWaveFormatFromAudioClient();
    void Reset(std::unique_lock<std::mutex>& lk);

    std::unique_ptr<FloatAudioBuffer> AllocateFloatAudioBuffer();
    int64_t GetPendingBufferSamplesCount();

private:
    static constexpr DWORD REFTIMES_PER_MILLISEC = std::chrono::duration_cast<H::Chrono::Hns::_MyBase>(1ms).count();
    static constexpr DWORD REFTIMES_PER_SEC = std::chrono::duration_cast<H::Chrono::Hns::_MyBase>(1s).count();

    std::mutex mx;

    Microsoft::WRL::ComPtr<IAudioClient> audioClient;
    WAVEFORMATEXTENSIBLE waveFormatExtensible;
    H::Rational<int64_t> bufferDurationMax;
    H::Rational<int64_t> bufferDurationCurrent;

    HANDLE hEventAudioClientBufferIsReady;
    Microsoft::WRL::ComPtr<IAudioRenderClient> renderClient;
    std::shared_ptr<PlaybackClockAudio> playbackClockAudio;

    uint32_t bufferFrameCount = 0;
    int64_t hnsRequestedDuration = 0;
    int64_t minFrameTime = 0;


    std::unique_ptr<FloatAudioBuffer> floatAudioBuffer;

    //TODO: Mixers should be set in TrackMixRange on calling Track::Render method, 
    //      so that each Track would be able to have it's own mixer.
    DefaultAudioMixer defaultAudioMixer;

    std::atomic<bool> stopThreads = false;
    std::atomic<bool> audioBuffersQueueOverflow = false;
    std::atomic<bool> audioClientResetAvailable = true;
    std::condition_variable cvAudioBuffersQueueAvailable;
    std::condition_variable cvAudioClientResetAvailable;
    std::thread bufferMonitorThread;
    std::thread testEventThread;
};