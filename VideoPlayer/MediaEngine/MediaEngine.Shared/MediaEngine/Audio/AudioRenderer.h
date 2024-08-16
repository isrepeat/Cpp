#pragma once
#include "AudioBuffer.h"
#include "IAudioOutput.h"
#include "IAudioClientFactory.h"
#include <Helpers/Macros.h>

#include <memory>
#include <list>
#include <ppl.h>
#include <wrl.h>
#include <Audioclient.h>

class AudioRenderer {
public:
    AudioRenderer(IAudioClientFactory* audioClientFactory);
    ~AudioRenderer();

    WAVEFORMATEX GetWaveFormat() const;

    std::weak_ptr<PlaybackClockDynamic> GetPlaybackClock();

    void Play();
    void Pause();
    void Stop();
    void Rewind(int64_t ptsHns);

    IAudioOutput* GetAudioOutput();

    void AllocateBuffer();
    void SubmitBuffer();

private:
    //
    // Internal implementaitions
    //
    std::weak_ptr<PlaybackClockDynamic> GetPlaybackClock(H::MethodIsGuarded lk);

    void Play(H::MethodIsGuarded lk);
    void Pause(H::MethodIsGuarded lk);
    void Stop(H::MethodIsGuarded lk);
    void Rewind(H::MethodIsGuarded lk, int64_t ptsHns);

    void SafeCallToOutput(std::function<void()> fn);
    void ResetOutput();

    enum class State {
        Paused,
        Playing,
        Stopped,
    };

private:
    std::mutex mx;

    IAudioClientFactory* audioClientFactory;

    bool needResetOutput;
    std::unique_ptr<IAudioOutput> output;

    // lastOutput params are used to make switch to/from NullOutput faster
    uint32_t lastOutputChannelCount;
    uint32_t lastOutputSampleRate;
    WAVEFORMATEX waveFmt;
    State state;

    std::shared_ptr<PlaybackClockDynamic> dynamicPlaybackClockAudio;
};