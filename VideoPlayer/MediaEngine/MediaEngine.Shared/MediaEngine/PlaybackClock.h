#pragma once
#include <Audioclient.h>
#include <Helpers/ThreadSafeObject.hpp>
#include <Helpers/Dx/DxHelpers.h>
#include <Helpers/Time.h>
#include "HMath.h"
#include <cassert>

// TODO: wrap to namespace MovieMakerNative or smth else

enum class PlaybackClockState {
    Playing,
    Paused,
    Stopped
};

// TODO: rename to IClock because this interface may be used for recording media also?
class IPlaybackClock {
public:
    IPlaybackClock() = default;
    virtual ~IPlaybackClock() {};

    virtual void Start() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;
    virtual void SetOffsetPts(H::Chrono::Hns offsetPts) = 0;
    virtual void SyncronizeWith(IPlaybackClock* otherPlaybackClock, H::Chrono::Hns eps) = 0;
    virtual void UpdateLastPlaybackPts(H::Chrono::Hns lastPlaybackPts) = 0;
    virtual H::Chrono::Hns GetLastPlaybackPts() = 0;
    virtual H::Chrono::Hns GetPlaybackPts() = 0;
};


class PlaybackClockDefault : public IPlaybackClock {
public:
    PlaybackClockDefault();
    ~PlaybackClockDefault() = default;

    void Start() override;
    void Pause() override;
    void Stop() override;
    void SetOffsetPts(H::Chrono::Hns offsetPts) override;
    void SyncronizeWith(IPlaybackClock* otherPlaybackClock, H::Chrono::Hns eps) override;
    void UpdateLastPlaybackPts(H::Chrono::Hns lastPlaybackPts) override;
    H::Chrono::Hns GetLastPlaybackPts() override;
    H::Chrono::Hns GetPlaybackPts() override;

protected:
    std::mutex mx;
    H::Chrono::Hns offsetPts;
    H::Chrono::Hns playbackPts;
};


class PlaybackClockVideo : public PlaybackClockDefault {
public:
    PlaybackClockVideo() = default;
    ~PlaybackClockVideo() = default;
};


class PlaybackClockAudio : public IPlaybackClock {
public:
    PlaybackClockAudio(Microsoft::WRL::ComPtr<IAudioClient> audioClient, WAVEFORMATEX waveFmt);
    ~PlaybackClockAudio() = default;

    void Start() override;
    void Pause() override;
    void Stop() override;
    void SetOffsetPts(H::Chrono::Hns offsetPts) override;
    void SyncronizeWith(IPlaybackClock* otherPlaybackClock, H::Chrono::Hns eps) override;
    void UpdateLastPlaybackPts(H::Chrono::Hns lastPlaybackPts) override;
    H::Chrono::Hns GetLastPlaybackPts() override;
    H::Chrono::Hns GetPlaybackPts() override;

    H::Chrono::Hns GetPlaybackPtsLocal();

private:
    std::recursive_mutex mx;
    Microsoft::WRL::ComPtr<IAudioClient> audioClient;
    Microsoft::WRL::ComPtr<IAudioClock> audioClock;
    WAVEFORMATEX waveFmt;
    H::Chrono::Hns offsetPts;
    H::Chrono::Hns syncCorrectionPts;
    H::Chrono::Hns lastPlaybackPtsLocal;
};



class PlaybackClockTotal : public IPlaybackClock {
public:
    PlaybackClockTotal();
    ~PlaybackClockTotal();

    void Start() override;
    void Pause() override;
    void Stop() override;
    void SetOffsetPts(H::Chrono::Hns offsetPts) override;
    void SyncronizeWith(IPlaybackClock* otherPlaybackClock, H::Chrono::Hns eps) override;
    void UpdateLastPlaybackPts(H::Chrono::Hns lastPlaybackPts) override;
    H::Chrono::Hns GetLastPlaybackPts() override;
    H::Chrono::Hns GetPlaybackPts() override;

protected:
    std::mutex mx;
    H::Chrono::Hns offsetPts;
    H::Chrono::Hns playbackPts;
    std::chrono::steady_clock::time_point lastTimePoint;
    std::atomic<bool> isPlaying = false;
    std::atomic<bool> stopThreads = false;
    std::thread workerThread;
};



// TODO: try refactor without shared_ptr
struct PlaybackClockWrapper {
    std::shared_ptr<IPlaybackClock> playbackClockInterface = nullptr;
};

class PlaybackClockDynamic {
public:
    using _PlaybackClockInterfaceSafeObj = H::ThreadSafeObject<std::recursive_mutex, PlaybackClockWrapper>;
    PlaybackClockDynamic(std::weak_ptr<IPlaybackClock> playbackClockInterface = {});

    _PlaybackClockInterfaceSafeObj::_Locked GetLocked();
    void ChangeInterface(std::weak_ptr<IPlaybackClock> newPlaybackClockInterface);

private:
    _PlaybackClockInterfaceSafeObj playbackClockInterfaceSafeObj;
};


class PlaybackClock {
public:
    PlaybackClock();

    void SetPlaybackClockState(PlaybackClockState state);
    PlaybackClockState GetPlaybackClockState() const;

    void SetPlaybackClockVideo(std::weak_ptr<PlaybackClockDynamic> playbackClockVideo);
    void SetPlaybackClockAudio(std::weak_ptr<PlaybackClockDynamic> playbackClockAudio);

    std::shared_ptr<PlaybackClockDynamic> VideoClock() const;
    std::shared_ptr<PlaybackClockDynamic> AudioClock() const;
    const std::unique_ptr<PlaybackClockTotal>& TotalClock() const;

    void Rewind(H::Chrono::Hns seekPts);

private:
    mutable std::mutex mx;
    std::atomic<PlaybackClockState> playbackClockState;
    std::weak_ptr<PlaybackClockDynamic> dynamicPlaybackClockVideoWeak;
    std::weak_ptr<PlaybackClockDynamic> dynamicPlaybackClockAudioWeak;
    const std::unique_ptr<PlaybackClockTotal> playbackClockTotal;
};