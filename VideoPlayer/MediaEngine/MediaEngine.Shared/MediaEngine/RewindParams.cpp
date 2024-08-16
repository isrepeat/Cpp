#include "RewindParams.h"
#include "AvSourceStreamManager.h"
#include "Track.h"

//
// AvReaderRewindAsyncResult
//
AvReaderRewindAsyncResult::AvReaderRewindAsyncResult()
    : events{ std::make_unique<Events>() }
    , status{ Status::Completed }
    , gotFirstVideoSample{ false }
    , gotFirstAudioSample{ false }
{}

AvReaderRewindAsyncResult::~AvReaderRewindAsyncResult() {
    assert(this->gotFirstVideoSample);
    assert(this->gotFirstAudioSample);
    assert(this->status == Status::Completed);

    this->status = Status::Completed;
    this->cvGotFirstVideoSample.notify_all();
    this->cvGotFirstAudioSample.notify_all();
}

std::unique_ptr<AvReaderRewindAsyncResult::Events>& AvReaderRewindAsyncResult::GetEvents() {
    return this->events;
}


void AvReaderRewindAsyncResult::NotifyAboutGotFirstVideoSample(MF::SampleInfo mfVideoSample) {
    LOG_DEBUG_D("NotifyAboutGotFirstVideoSample(mfVideoSample = {}ms)", H::Chrono::milliseconds_f{ mfVideoSample.pts }.count());
    std::unique_lock lk{ mx };
    if (this->status == Status::Pending && !this->gotFirstVideoSample) {
        if (this->events->gotFirstVideoSampleEvent) {
            this->events->gotFirstVideoSampleEvent(mfVideoSample);
        }
        this->gotFirstVideoSample = true;
        this->cvGotFirstVideoSample.notify_all();

        if (this->gotFirstAudioSample) {
            this->status = Status::Completed;
        }
    }
}

void AvReaderRewindAsyncResult::NotifyAboutGotFirstAudioSample(MF::SampleInfo mfAudioSample) {
    LOG_DEBUG_D("NotifyAboutGotFirstAudioSample(mfAudioSample = {}ms)", H::Chrono::milliseconds_f{ mfAudioSample.pts }.count());
    std::unique_lock lk{ mx };
    if (this->status == Status::Pending && !this->gotFirstAudioSample) {
        if (this->events->gotFirstAudioSampleEvent) {
            this->events->gotFirstAudioSampleEvent(mfAudioSample);
        }
        this->gotFirstAudioSample = true;
        this->cvGotFirstAudioSample.notify_all();

        if (this->gotFirstVideoSample) {
            this->status = Status::Completed;
        }
    }
}

void AvReaderRewindAsyncResult::WaitFirstVideoSample() {
    LOG_DEBUG_D("WaitFirstVideoSample()");
    std::unique_lock lk{ mx };
    LOG_DEBUG_D("WaitFirstVideoSample(): mx locked");
    this->cvGotFirstVideoSample.wait(lk, [this] {
        return this->gotFirstVideoSample || this->status == Status::Completed;
        });
    LOG_DEBUG_D("WaitFirstVideoSample(): cv unlocked");
}

void AvReaderRewindAsyncResult::WaitFirstAudioSample() {
    std::unique_lock lk{ mx };
    this->cvGotFirstAudioSample.wait(lk, [this] {
        return this->gotFirstAudioSample || this->status == Status::Completed;
        });
}

//void AvReaderRewindAsyncResult::SeekStart(H::ThreadSafeObject<std::mutex, std::unique_ptr<AvSourceStreamManager>>::_Locked& avSourceStreamManager) {
void AvReaderRewindAsyncResult::SeekStart(H::ThreadSafeObject<std::recursive_mutex, std::unique_ptr<AvSourceStreamManager>>::_Locked& avSourceStreamManager) {
    std::unique_lock lk{ mx };
    if (!avSourceStreamManager->GetActiveVideoStream()) {
        this->gotFirstVideoSample = true;
        this->cvGotFirstVideoSample.notify_all(); // ensure that we unlocked some thread that started waiting early
    }
    if (!avSourceStreamManager->GetActiveAudioStream()) {
        this->gotFirstAudioSample = true;
        this->cvGotFirstAudioSample.notify_all();
    }

    if (this->gotFirstVideoSample && this->gotFirstAudioSample) {
        LOG_WARNING_D("There is no active stream, rewind status = Completed");
        this->status = Status::Completed;
        return;
    }

    this->status = Status::Pending;
}

AvReaderRewindAsyncResult::Status AvReaderRewindAsyncResult::GetStatus() {
    std::unique_lock lk{ mx };
    return this->status;
}


//
// RewindAsyncResult
//
RewindAsyncResult::RewindAsyncResult(Callbacks callbacks)
    : callbacks{ std::make_unique<Callbacks>(callbacks) }
{}

// TODO: add guard for queue
std::weak_ptr<AvReaderRewindAsyncResult> RewindAsyncResult::MakeAvReaderRewindAsyncResult(Track* currentTrack) {
    auto avReaderRewindAsyncResult = std::make_shared<AvReaderRewindAsyncResult>();

    avReaderRewindAsyncResult->GetEvents()->gotFirstVideoSampleEvent.AddFinish([this, currentTrack](MF::SampleInfo mfVideoSample) {
        if (this->callbacks->GotFirstTrackVideoSample) {
            this->callbacks->GotFirstTrackVideoSample(mfVideoSample, currentTrack);
        }
        });

    avReaderRewindAsyncResult->GetEvents()->gotFirstAudioSampleEvent.AddFinish([this, currentTrack](MF::SampleInfo mfAudioSample) {
        if (this->callbacks->GotFirstTrackAudioSample) {
            this->callbacks->GotFirstTrackAudioSample(mfAudioSample, currentTrack);
        }
        });

    this->avReaderRewindAsyncResultsQueue.push(avReaderRewindAsyncResult);
    return this->avReaderRewindAsyncResultsQueue.back();
}


void RewindAsyncResult::Wait() {
    LOG_DEBUG_D("Wait ...");
    while (!this->avReaderRewindAsyncResultsQueue.empty()) {
        auto avReaderRewindAsyncResult = std::move(this->avReaderRewindAsyncResultsQueue.front());
        this->avReaderRewindAsyncResultsQueue.pop();

        avReaderRewindAsyncResult->WaitFirstVideoSample();
        avReaderRewindAsyncResult->WaitFirstAudioSample();
    }
    LOG_DEBUG_D("Rewind completed");
}



//
// AvReaderRewindParams
//
AvReaderRewindParams::AvReaderRewindParams(H::Chrono::Hns pts, std::weak_ptr<AvReaderRewindAsyncResult> asyncResultWeak)
    : pts{ pts }
    , asyncResultWeak{ asyncResultWeak }
{}



//
// RewindParams
//
RewindParams::RewindParams(H::Chrono::Hns pts, RewindAsyncResult::Callbacks rewindAsyncResultCallbacks)
    : pts{ pts }
    , asyncResult{ rewindAsyncResultCallbacks }
{}


AvReaderRewindParams RewindParams::MakeAvReaderRewindParams(Track* currentTrack) {
    return AvReaderRewindParams{ 
        H::Chrono::Hns{ this->pts - currentTrack->GetOffsetPts() },
        this->asyncResult.MakeAvReaderRewindAsyncResult(currentTrack) 
    };
}


std::shared_ptr<RewindParams> MakeRewindParams(H::Chrono::Hns pts, RewindAsyncResult::Callbacks rewindAsyncResultCallbacks) {
    return std::make_shared<RewindParams>(pts, rewindAsyncResultCallbacks);
}