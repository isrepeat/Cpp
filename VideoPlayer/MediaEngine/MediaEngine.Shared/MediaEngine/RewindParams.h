#pragma once
#include <Helpers/MediaFoundation/SampleInfo.h>
#include <Helpers/ThreadSafeObject.hpp>
#include <Helpers/Signal.h>

#include "AvStreamType.h"

#include <condition_variable>
#include <functional>
#include <queue>

class Track;
class AvReader;
class AvSourceStreamManager;

class AvReaderRewindAsyncResult {
    friend AvReader; // TODO: rewrite with PassKey

public:
    AvReaderRewindAsyncResult();
    ~AvReaderRewindAsyncResult();

    enum class Status {
        Completed,
        Pending,
    };

    struct Events {
        H::SignalOnce<void(MF::SampleInfo)> gotFirstVideoSampleEvent;
        H::SignalOnce<void(MF::SampleInfo)> gotFirstAudioSampleEvent;
    };

    std::unique_ptr<Events>& GetEvents();

    void NotifyAboutGotFirstVideoSample(MF::SampleInfo mfVideoSample);
    void NotifyAboutGotFirstAudioSample(MF::SampleInfo mfAudioSample);
    void WaitFirstVideoSample();
    void WaitFirstAudioSample();

private:
    //void SeekStart(H::ThreadSafeObject<std::mutex, std::unique_ptr<AvSourceStreamManager>>::_Locked& avSourceStreamManager);
    void SeekStart(H::ThreadSafeObject<std::recursive_mutex, std::unique_ptr<AvSourceStreamManager>>::_Locked& avSourceStreamManager);
    Status GetStatus();

private:
    std::mutex mx;
    std::unique_ptr<Events> events;
    std::atomic<Status> status;
    std::atomic<bool> gotFirstVideoSample;
    std::atomic<bool> gotFirstAudioSample;
    std::condition_variable cvGotFirstVideoSample;
    std::condition_variable cvGotFirstAudioSample;
};


class RewindAsyncResult {
public:
    struct Callbacks {
        std::function<void(MF::SampleInfo, Track*)> GotFirstTrackVideoSample;
        std::function<void(MF::SampleInfo, Track*)> GotFirstTrackAudioSample;
    };

    RewindAsyncResult(Callbacks callbacks = {});

    std::weak_ptr<AvReaderRewindAsyncResult> MakeAvReaderRewindAsyncResult(Track* currentTrack);
    //const std::unique_ptr<const Callbacks>& GetCallbacks() const;
    void Wait();

private:
    const std::unique_ptr<const Callbacks> callbacks;
    std::queue<std::shared_ptr<AvReaderRewindAsyncResult>> avReaderRewindAsyncResultsQueue;
};



struct AvReaderRewindParams {
    H::Chrono::Hns pts;
    std::weak_ptr<AvReaderRewindAsyncResult> asyncResultWeak;

    AvReaderRewindParams(H::Chrono::Hns pts, std::weak_ptr<AvReaderRewindAsyncResult> asyncResultWeak);
};

struct RewindParams {
    H::Chrono::Hns pts;
    RewindAsyncResult asyncResult;

    RewindParams(H::Chrono::Hns pts, RewindAsyncResult::Callbacks rewindAsyncResultCallbacks);
    AvReaderRewindParams MakeAvReaderRewindParams(Track* currentTrack);
};

std::shared_ptr<RewindParams> MakeRewindParams(H::Chrono::Hns pts, RewindAsyncResult::Callbacks rewindAsyncResultCallbacks = {});