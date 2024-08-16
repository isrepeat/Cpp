#include "AudioRenderer.h"
#include "WASAPIAudioOutput.h"


AudioRenderer::AudioRenderer(IAudioClientFactory* audioClientFactory)
    : audioClientFactory(audioClientFactory)
    , state(State::Stopped)
    , needResetOutput(false)
    , lastOutputChannelCount(2)
    , lastOutputSampleRate(44100)
    , dynamicPlaybackClockAudio{ std::make_shared<PlaybackClockDynamic>() }
{
    ZeroMemory(&this->waveFmt, sizeof(this->waveFmt));
    this->ResetOutput();
}

AudioRenderer::~AudioRenderer() {
}

WAVEFORMATEX AudioRenderer::GetWaveFormat() const {
    return this->waveFmt;
}

std::weak_ptr<PlaybackClockDynamic> AudioRenderer::GetPlaybackClock() {
    std::lock_guard lk{ mx };
    return this->GetPlaybackClock(lk);
}

void AudioRenderer::Play() {
    std::lock_guard lk{ mx };
    this->Play(lk);
}

void AudioRenderer::Pause() {
    std::lock_guard lk{ mx };
    this->Pause(lk);
}

void AudioRenderer::Stop() {
    std::lock_guard lk{ mx };
    this->Stop(lk);
}

void AudioRenderer::Rewind(int64_t ptsHns) {
    std::lock_guard lk{ mx };
    this->Rewind(lk, ptsHns);
}

IAudioOutput* AudioRenderer::GetAudioOutput() {
    std::lock_guard lk{ mx };
    return this->output.get();
}

void AudioRenderer::AllocateBuffer() {
    std::lock_guard lk{ mx };
    this->SafeCallToOutput([&]() {
        this->output->AllocateBuffer();
        });
}

void AudioRenderer::SubmitBuffer() {
    std::lock_guard lk{ mx };
    this->SafeCallToOutput([&]() {
        this->output->SubmitBuffer();
        });
}


//
// Internal implementaitions
//
std::weak_ptr<PlaybackClockDynamic> AudioRenderer::GetPlaybackClock(H::MethodIsGuarded lk) {
    return this->dynamicPlaybackClockAudio;
}

void AudioRenderer::Play(H::MethodIsGuarded lk) {
    // this->mx must be locked
    this->state = State::Playing;

    this->SafeCallToOutput([&]() {
        this->output->Play();
        });
}

void AudioRenderer::Pause(H::MethodIsGuarded lk) {
    this->state = State::Paused;

    this->SafeCallToOutput([this]() {
        this->output->Pause();
        });
}

void AudioRenderer::Stop(H::MethodIsGuarded lk) {
    this->state = State::Stopped;

    this->SafeCallToOutput([this]() {
        this->output->Stop();
        });
}

void AudioRenderer::Rewind(H::MethodIsGuarded lk, int64_t ptsHns) {
    this->Stop(lk);

    this->SafeCallToOutput([this]() {
        this->output->Reset();
        });

    this->dynamicPlaybackClockAudio->GetLocked()->playbackClockInterface->SetOffsetPts(H::Chrono::Hns{ ptsHns });
    //this->dynamicPlaybackClockAudio->GetLocked()->playbackClockInterface->UpdateLastPlaybackPts(H::Chrono::Hns{ 0 });
}


void AudioRenderer::SafeCallToOutput(std::function<void()> fn) {
    int tryIdx = 0;
    bool failed = true;

    if (this->needResetOutput) {
        this->needResetOutput = false;
        this->ResetOutput();
    }

    while (tryIdx < 4 && failed) {
        try {
            fn();
            failed = false;
        }
        catch (...) {
            this->ResetOutput();
            failed = true;
        }

        tryIdx++;
    }
}

void AudioRenderer::ResetOutput() {
    bool updateLastOutputFmtParams = false;

    H::Rational<float> desiredAudioFps{ 30, 1 };
    H::Chrono::Hns audioBufferMaxDuration = std::chrono::duration_cast<H::Chrono::Hns::_MyBase>(
        desiredAudioFps.Inversed().As<H::Chrono::seconds_f>()
        );

    this->output = std::make_unique<WASAPIAudioOutput>(this->audioClientFactory, audioBufferMaxDuration);
    updateLastOutputFmtParams = true;

    this->dynamicPlaybackClockAudio->ChangeInterface(this->output->GetPlaybackClock());

    auto outputWaveFormat = this->output->GetWaveFormat();

    if (updateLastOutputFmtParams) {
        this->lastOutputChannelCount = outputWaveFormat.nChannels;
        this->lastOutputSampleRate = outputWaveFormat.nSamplesPerSec;
    }

    this->waveFmt = outputWaveFormat;

    switch (this->state) {
    case State::Paused:
        this->output->Pause();
        break;
    case State::Playing:
        this->output->Play();
        break;
    case State::Stopped:
        this->output->Stop();
        break;
    default:
        break;
    }
}