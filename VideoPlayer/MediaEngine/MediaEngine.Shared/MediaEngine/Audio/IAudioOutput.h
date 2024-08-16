#pragma once
#include "../HMath.h"
#include "../MediaSample.h"
#include "../PlaybackClock.h"
#include "SimpleAudioMixer.h"
#include <Audioclient.h>
#include <memory>
#include <list>
#include <ppl.h>
#include <wrl.h>


class IAudioOutput {
public:
    IAudioOutput() = default;
    virtual ~IAudioOutput() = default;

    virtual WAVEFORMATEX GetWaveFormat() = 0;
    //virtual int64_t GetMinFrameTimeHns() = 0;

    // TODO: implement custom safe wrapper under IPlaybackClock to avoid shared_ptr
    virtual std::weak_ptr<IPlaybackClock> GetPlaybackClock() = 0;

    virtual void AllocateBuffer() = 0;
    virtual AudioBufferDesc GetBufferDesc() = 0;
    // Mb return interface to audio mixer instead?
    virtual AudioSampleMixStatus MixAudioSample(MF::MFAudioSample * mfAudioSample, int64_t startPtsHns, size_t sourceCount) = 0;

    virtual void SubmitBuffer() = 0;

    virtual void Play() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;
    virtual void Reset() = 0;
};