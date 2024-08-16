#include "PlaybackClock.h"

//
// PlaybackClockDefault
//
PlaybackClockDefault::PlaybackClockDefault()
    : playbackPts{ 0_hns }
{}

void PlaybackClockDefault::Start() {
    std::lock_guard lk{ mx };
}
void PlaybackClockDefault::Pause() {
    std::lock_guard lk{ mx };
}
void PlaybackClockDefault::Stop() {
    std::lock_guard lk{ mx };
}

void PlaybackClockDefault::SetOffsetPts(H::Chrono::Hns offsetPts) {
    std::lock_guard lk{ mx };
    this->offsetPts = offsetPts;
}
void PlaybackClockDefault::SyncronizeWith(IPlaybackClock* otherPlaybackClock, H::Chrono::Hns eps) {
    assert(false);
}
void PlaybackClockDefault::UpdateLastPlaybackPts(H::Chrono::Hns lastPlaybackPts) {
    std::lock_guard lk{ mx };
    this->playbackPts = lastPlaybackPts;
}
H::Chrono::Hns PlaybackClockDefault::GetLastPlaybackPts() {
    std::lock_guard lk{ mx };
    return this->offsetPts + this->playbackPts;
}
H::Chrono::Hns PlaybackClockDefault::GetPlaybackPts() {
    std::lock_guard lk{ mx };
    return this->offsetPts + this->playbackPts;
}



//
// PlaybackClockAudio
//
PlaybackClockAudio::PlaybackClockAudio(Microsoft::WRL::ComPtr<IAudioClient> audioClient, WAVEFORMATEX waveFmt)
    : audioClient{ audioClient }
    , audioClock{}
    , waveFmt{ waveFmt }
    , offsetPts{ 0_hns }
    , syncCorrectionPts{ 0_hns }
    , lastPlaybackPtsLocal{ 0_hns }
{
    Microsoft::WRL::ComPtr<IAudioClock> audioClock;
    HRESULT hr = this->audioClient->GetService(IID_PPV_ARGS(this->audioClock.GetAddressOf()));
    H::System::ThrowIfFailed(hr);
}

void PlaybackClockAudio::Start() {
    std::lock_guard lk{ mx };

    HRESULT hr = this->audioClient->Start();
    if (hr == AUDCLNT_E_NOT_STOPPED)
        return;

    LOG_FAILED(hr);
    H::System::ThrowIfFailed(hr);
}

void PlaybackClockAudio::Pause() {
    std::lock_guard lk{ mx };
    HRESULT hr = this->audioClient->Stop();
    LOG_FAILED(hr);
    H::System::ThrowIfFailed(hr);
}

void PlaybackClockAudio::Stop() {
    std::lock_guard lk{ mx };
    HRESULT hr = S_OK;

    hr = this->audioClient->Stop();
    LOG_FAILED(hr);
    H::System::ThrowIfFailed(hr);

    do {
        Sleep(10);
        hr = this->audioClient->Reset(); // reset audioClock position to 0
        if (hr == AUDCLNT_E_BUFFER_OPERATION_PENDING) {
            LOG_DEBUG_D("AUDCLNT_E_BUFFER_OPERATION_PENDING");
            LOG_FAILED(hr);
        }
    } while (hr == AUDCLNT_E_BUFFER_OPERATION_PENDING);
    H::System::ThrowIfFailed(hr);

    this->syncCorrectionPts = 0_hns;
}

void PlaybackClockAudio::SetOffsetPts(H::Chrono::Hns offsetPts) {
    std::lock_guard lk{ mx };
    this->offsetPts = offsetPts;
}

void PlaybackClockAudio::SyncronizeWith(IPlaybackClock* otherPlaybackClock, H::Chrono::Hns eps) {
    std::lock_guard lk{ mx };
    auto distanceToOtherClock = otherPlaybackClock->GetPlaybackPts() - this->GetLastPlaybackPts();
    if (std::abs(distanceToOtherClock.count()) > eps.count()) {
        LOG_DEBUG_D("\n"
            "Synchronize this clock with other: \n"
            "  this->offsetPts = {}ms \n"
            "  this->syncCorrectionPts = {}ms \n"
            "  this->lastPlaybackPtsLocal = {}ms \n"
            "  this->GetLastPlaybackPts() = {}ms \n"
            "  other->GetPlaybackPts() = {}ms [dt = {}ms] \n"
            , H::Chrono::milliseconds_f{ this->offsetPts }.count()
            , H::Chrono::milliseconds_f{ this->syncCorrectionPts }.count()
            , H::Chrono::milliseconds_f{ this->lastPlaybackPtsLocal }.count()
            , H::Chrono::milliseconds_f{ this->GetLastPlaybackPts() }.count()
            , H::Chrono::milliseconds_f{ otherPlaybackClock->GetPlaybackPts() }.count(), H::Chrono::milliseconds_f{ distanceToOtherClock }.count()
        );
        this->syncCorrectionPts += distanceToOtherClock;
    }
}

void PlaybackClockAudio::UpdateLastPlaybackPts(H::Chrono::Hns lastPlaybackPts) {
    std::lock_guard lk{ mx };
    this->lastPlaybackPtsLocal = lastPlaybackPts;
}

H::Chrono::Hns PlaybackClockAudio::GetLastPlaybackPts() {
    std::lock_guard lk{ mx };
    return this->offsetPts + this->syncCorrectionPts + this->lastPlaybackPtsLocal;
}

H::Chrono::Hns PlaybackClockAudio::GetPlaybackPts() {
    std::lock_guard lk{ mx };
    return this->offsetPts + this->syncCorrectionPts + this->GetPlaybackPtsLocal();
}

H::Chrono::Hns PlaybackClockAudio::GetPlaybackPtsLocal() {
    HRESULT hr = S_OK;
    uint64_t frequency;
    uint64_t position, perfCounterPosition;

    hr = this->audioClock->GetFrequency(&frequency);
    H::System::ThrowIfFailed(hr);

    hr = this->audioClock->GetPosition(&position, &perfCounterPosition);
    H::System::ThrowIfFailed(hr);

    // position and frequency measured in bytes
    // need convert them to audio frames
    position /= this->waveFmt.nBlockAlign;
    frequency /= this->waveFmt.nBlockAlign;

    auto res = static_cast<int64_t>(HMath::Convert2(position, frequency, 10000000ULL));
    return H::Chrono::Hns{ res };
}



//
// PlaybackClockTotal
//
PlaybackClockTotal::PlaybackClockTotal()
    : playbackPts{ 0_hns }
    , lastTimePoint{ std::chrono::high_resolution_clock::now() }
{
    this->workerThread = std::thread([this] {
        while (!this->stopThreads) {
            Sleep(1);
            std::lock_guard lk{ mx };

            if (this->isPlaying) {
                H::Chrono::Hns dt = std::chrono::duration_cast<H::Chrono::Hns::_MyBase>(
                    std::chrono::high_resolution_clock::now() - this->lastTimePoint);

                if (dt <= 20ms) {
                    //LOG_DEBUG_D("playbackPts [{}ms] += dt [{}ms] = {}ms"
                    //    , H::Chrono::milliseconds_f{ this->playbackPts }.count()
                    //    , H::Chrono::milliseconds_f{ dt }.count()
                    //    , H::Chrono::milliseconds_f{ this->playbackPts + dt }.count()
                    //);
                    this->playbackPts += dt;
                }
                else {
                    LOG_DEBUG_D("playbackPts = {}ms, but dt = {}ms !!!"
                        , H::Chrono::milliseconds_f{ this->playbackPts }.count()
                        , H::Chrono::milliseconds_f{ dt }.count()
                    );
                    this->playbackPts += 20ms;
                }
            }
            this->lastTimePoint = std::chrono::high_resolution_clock::now();
        }
        });
}

PlaybackClockTotal::~PlaybackClockTotal() {
    this->stopThreads = true;
    if (this->workerThread.joinable())
        this->workerThread.join();
}

void PlaybackClockTotal::Start() {
    std::lock_guard lk{ mx };
    this->isPlaying = true;
}
void PlaybackClockTotal::Pause() {
    std::lock_guard lk{ mx };
    this->isPlaying = false;
}
void PlaybackClockTotal::Stop() {
    std::lock_guard lk{ mx };
    this->playbackPts = 0_hns;
    this->offsetPts = 0_hns;
    this->isPlaying = false;
}

void PlaybackClockTotal::SetOffsetPts(H::Chrono::Hns offsetPts) {
    std::lock_guard lk{ mx };
    this->offsetPts = offsetPts;
    LOG_DEBUG_D("offsetPts = {}ms", H::Chrono::milliseconds_f{ this->offsetPts }.count());
}

void PlaybackClockTotal::SyncronizeWith(IPlaybackClock* otherPlaybackClock, H::Chrono::Hns eps) {
    assert(false);
}

void PlaybackClockTotal::UpdateLastPlaybackPts(H::Chrono::Hns lastPlaybackPts) {
    assert(false);
}
H::Chrono::Hns PlaybackClockTotal::GetLastPlaybackPts() {
    std::lock_guard lk{ mx };
    return this->offsetPts + this->playbackPts;
}
H::Chrono::Hns PlaybackClockTotal::GetPlaybackPts() {
    std::lock_guard lk{ mx };
    auto res = this->offsetPts + this->playbackPts;
    return res;
}



//
// PlaybackClockDynamic
//
PlaybackClockDynamic::PlaybackClockDynamic(std::weak_ptr<IPlaybackClock> playbackClockInterface)
    : playbackClockInterfaceSafeObj{ PlaybackClockWrapper{ playbackClockInterface.lock() } }
{}

PlaybackClockDynamic::_PlaybackClockInterfaceSafeObj::_Locked PlaybackClockDynamic::GetLocked() {
    return playbackClockInterfaceSafeObj.Lock();
}

void PlaybackClockDynamic::ChangeInterface(std::weak_ptr<IPlaybackClock> newPlaybackClockInterface) {
    auto playbackClockInterfaceLcocked = playbackClockInterfaceSafeObj.Lock();
    playbackClockInterfaceLcocked->playbackClockInterface = newPlaybackClockInterface.lock();
}



//
// PlaybackClock
//
PlaybackClock::PlaybackClock()
    : playbackClockState{ PlaybackClockState::Stopped }
    , playbackClockTotal{ std::make_unique<PlaybackClockTotal>() }
{
}

void PlaybackClock::SetPlaybackClockState(PlaybackClockState state) {
    std::lock_guard lk{ mx };
    this->playbackClockState = state;
    auto videoClock = this->VideoClock()->GetLocked();
    auto audioClock = this->AudioClock()->GetLocked();

    switch (this->playbackClockState) {
    case PlaybackClockState::Playing:
        this->playbackClockTotal->Start();
        videoClock->playbackClockInterface->Start();
        audioClock->playbackClockInterface->Start();
        break;
    case PlaybackClockState::Paused:
        this->playbackClockTotal->Pause();
        videoClock->playbackClockInterface->Pause();
        audioClock->playbackClockInterface->Pause();
        break;
    case PlaybackClockState::Stopped:
        this->playbackClockTotal->Stop();
        videoClock->playbackClockInterface->Stop();
        audioClock->playbackClockInterface->Stop();
        break;
    }
}

PlaybackClockState PlaybackClock::GetPlaybackClockState() const {
    std::lock_guard lk{ mx };
    return this->playbackClockState;
}

void PlaybackClock::SetPlaybackClockVideo(std::weak_ptr<PlaybackClockDynamic> playbackClockVideo) {
    this->dynamicPlaybackClockVideoWeak = playbackClockVideo;
}
void PlaybackClock::SetPlaybackClockAudio(std::weak_ptr<PlaybackClockDynamic> playbackClockAudio) {
    this->dynamicPlaybackClockAudioWeak = playbackClockAudio;
}

std::shared_ptr<PlaybackClockDynamic> PlaybackClock::VideoClock() const {
    if (auto playbackClockVideo = this->dynamicPlaybackClockVideoWeak.lock()) {
        return playbackClockVideo;
    }
    assert(false);
    return nullptr;
}

std::shared_ptr<PlaybackClockDynamic> PlaybackClock::AudioClock() const {
    if (auto playbackClockAudio = this->dynamicPlaybackClockAudioWeak.lock()) {
        return playbackClockAudio;
    }
    assert(false);
    return nullptr;
}

const std::unique_ptr<PlaybackClockTotal>& PlaybackClock::TotalClock() const {
    std::lock_guard lk{ mx };
    return this->playbackClockTotal;
}

void PlaybackClock::Rewind(H::Chrono::Hns seekPts) {
    // TODO: refactoring with safe method guard, pass lk to RewindInternal ...
    auto currentState = this->GetPlaybackClockState();
    {
        std::lock_guard lk{ mx };
        this->playbackClockTotal->Stop();
        this->playbackClockTotal->SetOffsetPts(seekPts);
        // ... to avoid cases when state can be changed before or after this scope
    }
    this->SetPlaybackClockState(currentState);
}
