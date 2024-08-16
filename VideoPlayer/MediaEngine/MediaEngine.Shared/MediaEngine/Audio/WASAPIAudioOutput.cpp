#include "WASAPIAudioOutput.h"
#include "AudioWaveFormat.h"
#include <Helpers/System.h>
#include <Helpers/Memory.h>

WASAPIAudioOutput::WASAPIAudioOutput(IAudioClientFactory* audioClientFactory, H::Chrono::Hns maxDurationHns)
    : audioClient{ audioClientFactory->CreateAudioClient(AudioClientType::RenderAudioClient) }
    , waveFormatExtensible{ this->GetWaveFormatFromAudioClient() }
    , bufferDurationMax{ 1, waveFormatExtensible.Format.nSamplesPerSec, maxDurationHns }
    , bufferDurationCurrent{ 1, waveFormatExtensible.Format.nSamplesPerSec, 0 }
{
    HRESULT hr = S_OK;

    // check format conversion if it's not float
    DWORD clientFlags = 0;
    clientFlags |= AUDCLNT_STREAMFLAGS_EVENTCALLBACK;

    REFERENCE_TIME hnsDefaultDevicePeriod, hnsMinimumDevicePeriod;
    hr = this->audioClient->GetDevicePeriod(&hnsDefaultDevicePeriod, &hnsMinimumDevicePeriod);
    H::System::ThrowIfFailed(hr);

    this->hnsRequestedDuration = hnsMinimumDevicePeriod;

    //this->minFrameTime = 1000000;// sharedModeFrameTime * 10;
    //REFERENCE_TIME hnsRequestedDuration = this->minFrameTime;// *3; // 1 frame for render, 1 frame for process, 1 just in case

    hr = this->audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        clientFlags,
        this->hnsRequestedDuration,
        this->hnsRequestedDuration,
        &this->waveFormatExtensible.Format,
        nullptr);

    if (hr == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED) {
        // Align the buffer if needed, see IAudioClient::Initialize() documentation
        UINT32 nFrames = 0;
        hr = this->audioClient->GetBufferSize(&nFrames);
        H::System::ThrowIfFailed(hr);

        this->hnsRequestedDuration = (REFERENCE_TIME)((double)WASAPIAudioOutput::REFTIMES_PER_SEC / this->waveFormatExtensible.Format.nSamplesPerSec * nFrames + 0.5);
        hr = audioClient->Initialize(
            AUDCLNT_SHAREMODE_EXCLUSIVE,
            clientFlags,
            this->hnsRequestedDuration,
            this->hnsRequestedDuration,
            &this->waveFormatExtensible.Format,
            nullptr);
    }
    H::System::ThrowIfFailed(hr);

    this->hEventAudioClientBufferIsReady = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
    if (this->hEventAudioClientBufferIsReady != nullptr && this->hEventAudioClientBufferIsReady != INVALID_HANDLE_VALUE) {
        hr = this->audioClient->SetEventHandle(this->hEventAudioClientBufferIsReady);
        H::System::ThrowIfFailed(hr);
    }

    hr = this->audioClient->GetBufferSize(&this->bufferFrameCount);
    H::System::ThrowIfFailed(hr);

    hr = this->audioClient->GetService(IID_PPV_ARGS(this->renderClient.GetAddressOf()));
    H::System::ThrowIfFailed(hr);

    this->playbackClockAudio = std::make_shared<PlaybackClockAudio>(this->audioClient, this->waveFormatExtensible.Format);

    //this->testEventThread = std::thread([this] {
    //    while (!this->stopThreads) {
    //        this->GetPendingBufferSamplesCount();
    //        Sleep(200);
    //        //std::unique_lock lk{ mx };
    //        //lk.unlock();
    //        //DWORD dwWait = WaitForSingleObject(this->hEventAudioClientBufferIsReady, INFINITE);
    //        //lk.lock();
    //        //switch (dwWait) {
    //        //case WAIT_ABANDONED:
    //        //    LOG_DEBUG_D("WAIT_ABANDONED");
    //        //    break;
    //        //case WAIT_OBJECT_0:
    //        //    LOG_DEBUG_D("WAIT_OBJECT_0");
    //        //    this->audioClientResetAvailable = true;
    //        //    this->cvAudioClientResetAvailable.notify_all();
    //        //    break;
    //        //case WAIT_TIMEOUT:
    //        //    LOG_DEBUG_D("WAIT_TIMEOUT");
    //        //    break;
    //        //case WAIT_FAILED:
    //        //    LOG_DEBUG_D("WAIT_FAILED");
    //        //    break;
    //        //default:
    //        //    LOG_DEBUG_D("E_FAIL");
    //        //    break;
    //        //}
    //    }
    //    });

    this->bufferMonitorThread = std::thread([this] {
        while (!this->stopThreads) {
            {
                std::unique_lock lk{ mx };
                if (this->audioBuffersQueueOverflow) {
                    if (this->GetPendingBufferSamplesCount() < 10 * this->bufferDurationMax.Value()) {
                        LOG_DEBUG_D("audio buffer available");
                        this->audioBuffersQueueOverflow = false;
                        this->cvAudioBuffersQueueAvailable.notify_all();
                    }
                }
            }
            Sleep(10);
        }
        });
}

WASAPIAudioOutput::~WASAPIAudioOutput() {
    this->stopThreads = true;
    if (this->bufferMonitorThread.joinable()) {
        this->bufferMonitorThread.join();
    }
    this->audioClient->Stop();
}

WAVEFORMATEX WASAPIAudioOutput::GetWaveFormat() {
    return this->waveFormatExtensible.Format;
}

std::weak_ptr<IPlaybackClock> WASAPIAudioOutput::GetPlaybackClock() {
    return this->playbackClockAudio;
}

void WASAPIAudioOutput::AllocateBuffer() {
    std::unique_lock lk{ mx };
    assert(this->floatAudioBuffer == nullptr);

    if (this->GetPendingBufferSamplesCount() >= 10 * this->bufferDurationMax.Value()) {
        this->audioBuffersQueueOverflow = true;
        LOG_DEBUG_D("audio buffer queue overflow, wait");
    }

    this->cvAudioBuffersQueueAvailable.wait(lk, [this] {
        return !this->audioBuffersQueueOverflow;
        });

    this->floatAudioBuffer = this->AllocateFloatAudioBuffer();
    this->bufferDurationCurrent += this->floatAudioBuffer->duration;
    //LOG_DEBUG_D("buffer allocated, bufferDurationCurrent = {}", this->bufferDurationCurrent);
}

AudioBufferDesc WASAPIAudioOutput::GetBufferDesc() {
    std::unique_lock lk{ mx };
    if (this->floatAudioBuffer) {
        AudioBufferDesc audioBufferDesc = *this->floatAudioBuffer; // copy casted
        return audioBufferDesc;
    }
    return AudioBufferDesc{ WAVEFORMATEX{} };
}

AudioSampleMixStatus WASAPIAudioOutput::MixAudioSample(MF::MFAudioSample* mfAudioSample, int64_t startPtsHns, size_t sourceCount) {
    std::unique_lock lk{ mx };
    return this->defaultAudioMixer.MixAudioSample(mfAudioSample, this->floatAudioBuffer.get(), this->playbackClockAudio.get(),
        H::Chrono::Hns{ startPtsHns }, sourceCount);
}


void WASAPIAudioOutput::SubmitBuffer() {
    std::unique_lock lk{ mx };

    if (!floatAudioBuffer) {
        LOG_DEBUG_D("floatAudioBuffer is empty, ignore");
        return;
    }

    HRESULT hr = S_OK;
    //LOG_DEBUG_D("buffer released, audioBuffer.duration = {}", floatAudioBuffer->duration);
    hr = this->renderClient->ReleaseBuffer(static_cast<uint32_t>(this->floatAudioBuffer->duration.Value()), 0);
    LOG_FAILED(hr);

    this->playbackClockAudio->UpdateLastPlaybackPts(this->bufferDurationCurrent);
    floatAudioBuffer = nullptr;
}

void WASAPIAudioOutput::Play() {
    LOG_DEBUG_D("Play()");
    std::unique_lock lk{ mx };
    this->playbackClockAudio->Start();
    //HRESULT hr = S_OK;
    //hr = this->audioClient->Start();
    //H::System::ThrowIfFailed(hr);
}

void WASAPIAudioOutput::Pause() {
    LOG_DEBUG_D("Pause()");
    std::unique_lock lk{ mx };
    this->playbackClockAudio->Pause();
    //HRESULT hr = S_OK;
    //hr = this->audioClient->Stop();
    //H::System::ThrowIfFailed(hr);
}

void WASAPIAudioOutput::Stop() {
    LOG_DEBUG_D("Stop()");
    std::unique_lock lk{ mx };
    HRESULT hr = S_OK;

    //this->GetPendingBufferSamplesCount();

    if (this->floatAudioBuffer) {
        LOG_DEBUG_D("buffer released, audioBuffer.duration = {}", this->floatAudioBuffer->duration.Value());
        hr = this->renderClient->ReleaseBuffer(static_cast<uint32_t>(this->floatAudioBuffer->duration.Value()), 0);
        LOG_FAILED(hr);

        this->floatAudioBuffer = nullptr;
    }

    //hr = this->audioClient->Stop();
    //H::System::ThrowIfFailed(hr);

    this->Reset(lk);
}

void WASAPIAudioOutput::Reset() {
    LOG_DEBUG_D("Reset()");
    std::unique_lock lk{ mx };
    HRESULT hr = S_OK;

    //this->GetPendingBufferSamplesCount();

    if (this->floatAudioBuffer) {
        LOG_DEBUG_D("buffer released, audioBuffer.duration = {}", this->floatAudioBuffer->duration.Value());
        hr = this->renderClient->ReleaseBuffer(static_cast<uint32_t>(this->floatAudioBuffer->duration.Value()), 0);
        LOG_FAILED(hr);

        this->floatAudioBuffer = nullptr;
    }

    this->Reset(lk);
}


WAVEFORMATEXTENSIBLE WASAPIAudioOutput::GetWaveFormatFromAudioClient() {
    HRESULT hr = S_OK;

    H::CoUniquePtr<WAVEFORMATEX> audioClientWaveFormat;
    hr = this->audioClient->GetMixFormat(audioClientWaveFormat.GetAddressOf());
    H::System::ThrowIfFailed(hr);

    // Use AudioWaveFormat helper to copy memory
    return AudioWaveFormat{ audioClientWaveFormat.get() }.GetWaveFormatExtensible();
}

void WASAPIAudioOutput::Reset(std::unique_lock<std::mutex>& lk) {
    LOG_DEBUG_D("Reset(lk)");
    //HRESULT hr = S_OK;

    //do {
    //    Sleep(10);
    //    //this->GetPendingBufferSamplesCount();
    //    hr = this->audioClient->Reset(); // reset audioClock position to 0
    //    if (hr == AUDCLNT_E_BUFFER_OPERATION_PENDING) {
    //        LOG_DEBUG_D("AUDCLNT_E_BUFFER_OPERATION_PENDING");
    //        LOG_FAILED(hr);
    //    }
    //} while (hr == AUDCLNT_E_BUFFER_OPERATION_PENDING);
    //H::System::ThrowIfFailed(hr);
    this->playbackClockAudio->Stop();

    this->floatAudioBuffer = nullptr;
    this->bufferDurationCurrent = 0;
}


std::unique_ptr<FloatAudioBuffer> WASAPIAudioOutput::AllocateFloatAudioBuffer() {
    HRESULT hr = S_OK;
    auto floatAudioBuffer = std::make_unique<FloatAudioBuffer>(this->waveFormatExtensible.Format);

    uint32_t numFramesPadding = 0;
    hr = this->audioClient->GetCurrentPadding(&numFramesPadding);
    H::System::ThrowIfFailed(hr);

    uint32_t numFramesAvailable = this->bufferFrameCount - numFramesPadding;
    numFramesAvailable = (uint32_t)(std::min)((int64_t)numFramesAvailable, this->bufferDurationMax.Value());

    hr = this->renderClient->GetBuffer(numFramesAvailable, (BYTE**)&floatAudioBuffer->data);
    H::System::ThrowIfFailed(hr);

    floatAudioBuffer->duration = numFramesAvailable;
    floatAudioBuffer->pts = this->playbackClockAudio->GetLastPlaybackPts();

    floatAudioBuffer->dataSize = (size_t)floatAudioBuffer->duration.Value() * sizeof(float) * floatAudioBuffer->waveFormatEx.nChannels;
    std::memset(floatAudioBuffer->data, 0, floatAudioBuffer->dataSize);

    floatAudioBuffer->dataFloatCount = (int64_t)(floatAudioBuffer->dataSize / sizeof(float));
    return floatAudioBuffer;
}

int64_t WASAPIAudioOutput::GetPendingBufferSamplesCount() {
    if (this->bufferDurationCurrent <= 0) {
        return 0;
    }

    this->playbackClockAudio->GetPlaybackPtsLocal();
    int64_t localPlaybackPos = HMath::ConvertCeil((int64_t)this->playbackClockAudio->GetPlaybackPtsLocal(), 10000000LL, (int64_t)this->waveFormatExtensible.Format.nSamplesPerSec);

    int64_t pendingBufferSamplesCount = this->bufferDurationCurrent.Value() - localPlaybackPos;

    //LOG_DEBUG_D("bufferDurationCurrent = {}, localPlaybackPos = {}, pendingBufferSamplesCount = {}"
    //    , this->bufferDurationCurrent
    //    , localPlaybackPos
    //    , pendingBufferSamplesCount
    //);

    assert(pendingBufferSamplesCount >= -2); // '-2' used instead '0' to avoid rounding problems during convertation above
    return pendingBufferSamplesCount;
}