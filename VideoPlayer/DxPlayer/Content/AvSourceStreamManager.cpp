#include "pch.h"
#include "AvSourceStreamManager.h"


AvStreamBase::AvStreamBase(AvStreamType streamType, int32_t streamIdx)
    : streamType{ streamType }
    , streamIdx{ streamIdx }
{}

AvStreamType AvStreamBase::GetStreamType() const {
    return streamType;
}

int32_t AvStreamBase::GetStreamIdx() const {
    return streamIdx;
}




void AvStreamSamplesProcessedQueue::AddPushRequest() {
    ++requestedSamples;
    // TODO: you can also allocate place in SamplesRaw here
}

void AvStreamSamplesProcessedQueue::Push(std::unique_ptr<MF::MFSample> sample) {
    if (requestedSamples > 0) {
        std::unique_lock lkSamplesRaw{ mxSamplesRaw };
        samplesRaw.push(std::move(sample));
    }
    else {
        assert(false);
    }
}

void AvStreamSamplesProcessedQueue::Process(IAvReaderEffect* streamEffect) {
    std::unique_ptr<MF::MFSample> sampleRaw;
    {
        std::unique_lock lkSamplesRaw{ mxSamplesRaw };
        if (samplesRaw.size() > 0) {
            sampleRaw = std::move(samplesRaw.front());
            samplesRaw.pop();
        }
    }

    std::unique_ptr<MF::MFSample> sampleProcessed;
    if (sampleRaw) {
        if (streamEffect) {
            // NOTE: it may take long time
            sampleProcessed = streamEffect->Process(std::move(sampleRaw));
        }
        else {
            sampleProcessed = std::move(sampleRaw);
        }
    }

    if (sampleProcessed) {
        std::unique_lock lkSamplesProcessed{ mxSamplesProcessed };
        samplesProcessed.push(this->ProcessSample(std::move(sampleProcessed)));
    }
    else {
        samplesProcessed.push(nullptr);
    }
}

MF::MFSample* AvStreamSamplesProcessedQueue::Peek() {
    std::unique_lock lkSamplesProcessed{ mxSamplesProcessed };
    MF::MFSample* sampleProcessed = nullptr;
    if (samplesProcessed.size() > 0) {
        sampleProcessed = samplesProcessed.front().get();
    }
    return sampleProcessed;
}

std::unique_ptr<MF::MFSample> AvStreamSamplesProcessedQueue::Pop() {
    std::unique_lock lkSamplesProcessed{ mxSamplesProcessed };
    std::unique_ptr<MF::MFSample> sampleProcessed;
    if (samplesProcessed.size() > 0) {
        sampleProcessed = std::move(samplesProcessed.front());
        samplesProcessed.pop();
        --requestedSamples;
    }

    return sampleProcessed;
}

int AvStreamSamplesProcessedQueue::GetSamplesRequests() {
    return requestedSamples;
}

size_t AvStreamSamplesProcessedQueue::GetRawSamplesCount() {
    std::unique_lock lkSamplesRaw{ mxSamplesRaw };
    return samplesRaw.size();
}
size_t AvStreamSamplesProcessedQueue::GetProcessedSamplesCount() {
    std::unique_lock lkSamplesProcessed{ mxSamplesProcessed };
    return samplesProcessed.size();
}

void AvStreamSamplesProcessedQueue::ClearSamples() {
    std::unique_lock lkSamplesRaw{ mxSamplesRaw };
    std::unique_lock lkSamplesProcessed{ mxSamplesProcessed };
    samplesRaw = {};
    samplesProcessed = {};
    requestedSamples = 0;
    prevSampleEnd = -1;
}

std::unique_ptr<MF::MFSample> AvStreamSamplesProcessedQueue::ProcessSample(std::unique_ptr<MF::MFSample> mfSample) {
    return mfSample;
}



AvStreamVideo::AvStreamVideo(
    int32_t streamIdx,
    H::Size size,
    TextureType textureType,
    H::Rational<float> frameRate,
    uint32_t avgBitRate,
    int rotation,
    bool isImage,
    std::unique_ptr<IAvReaderEffect>&& effectList)
    : AvStreamBase{ AvStreamType::Video, streamIdx }
    , size{ size }
    , textureType{ textureType }
    , frameRate{ frameRate }
    , avgBitRate{ avgBitRate }
    , rotation{ rotation }
    , isImage{ isImage }
    , effectList{ std::move(effectList) }
{}

H::Size AvStreamVideo::GetSize() const {
    return size;
}
TextureType AvStreamVideo::GetTextureType() const {
    return textureType;
}
H::Rational<float> AvStreamVideo::GetFrameRate() const {
    return frameRate;
}
uint32_t AvStreamVideo::GetAvgBitRate() const {
    return avgBitRate;
}
int AvStreamVideo::GetRotation() const {
    return rotation;
}
bool AvStreamVideo::IsImage() const {
    return isImage;
}
IAvReaderEffect* AvStreamVideo::GetEffect() const {
    return effectList.get();
}



AvStreamAudio::AvStreamAudio(
    int32_t streamIdx,
    size_t channelNum,
    uint32_t sampleRate,
    uint32_t avgBytesPerSecond,
    std::unique_ptr<IAvReaderEffect>&& effectList)
    : AvStreamBase{ AvStreamType::Audio, streamIdx }
    , channelNum{ channelNum }
    , channelNumOriginal{ channelNum }
    , sampleRate{ sampleRate }
    , sampleRateOriginal{ sampleRate }
    , avgBytesPerSecond{ avgBytesPerSecond }
    , effectList{ std::move(effectList) }
{}

void AvStreamAudio::SetOriginalAudioChannelNum(size_t channelNumOriginal) {
    this->channelNumOriginal = channelNumOriginal;
}
void AvStreamAudio::SetOriginAudioSampleRate(uint32_t sampleRateOriginal) {
    this->sampleRateOriginal = sampleRateOriginal;
}

size_t AvStreamAudio::GetChannelNumOriginal() const {
    return channelNumOriginal;
}
uint32_t AvStreamAudio::GetSampleRateOriginal() const {
    return sampleRateOriginal;
}
size_t AvStreamAudio::GetChannelNum() const {
    return channelNum;
}
uint32_t AvStreamAudio::GetSampleRate() const {
    return sampleRate;
}
uint32_t AvStreamAudio::GetAvgBytesPerSecond() const {
    return avgBytesPerSecond;
}
IAvReaderEffect* AvStreamAudio::GetEffect() const {
    return effectList.get();
}

std::unique_ptr<MF::MFSample> AvStreamAudio::ProcessSample(std::unique_ptr<MF::MFSample> mfSample) {
    //return this->CorrectSamlpePts(std::move(mfSample));
    return std::move(mfSample);
}

// TODO: refactoring
#define CORRECT_IN_HNS 0
std::unique_ptr<MF::MFSample> AvStreamAudio::CorrectSamlpePts(std::unique_ptr<MF::MFSample> mfSample) {
//    auto mfAudioSample = H::dynamic_unique_cast<MFAudioSample>(std::move(mfSample));
//    if (!mfAudioSample) {
//        LOG_WARNING_D("Unexpected 'mfAudioSample == nullptr'");
//        return nullptr;
//    }
//
//    auto sampleRateTmp = (int64_t)this->sampleRate;
//
//#if CORRECT_IN_HNS == 1
//    auto samplePts = sample->GetPtsHnsOriginal();
//    auto sampleDuration = sample->GetDurationHnsOriginal();
//#else
//    auto samplePts = H::Math::Convert((int64_t)mfAudioSample->pts, 10000000LL, sampleRateTmp);
//    auto sampleDuration = H::Math::Convert((int64_t)mfAudioSample->duration, 10000000LL, sampleRateTmp);
//
//    //H::Rational<int64_t> samplePts = { 1, this->sampleRate, mfAudioSample->pts };
//    //H::Rational<int64_t> sampleDuration = { 1, this->sampleRate, mfAudioSample->duration };
//#endif
//
//    // test for negative duration
//    // must never happen
//    if (sampleDuration < 0) {
//        LOG_WARNING_D("Unexpected 'sampleDuration < 0'");
//    }
//
//    std::unique_ptr<MF::MFSample> mfSampleNew;
//
//    if (this->prevSampleEnd != -1) {
//        // need always correct sample time
//        // after rewind there may be cases when sample time is greater than end of previous sample.
//        if (samplePts != this->prevSampleEnd) {
//            // TODO remove later _DEBUG ifdef
//#if DEBUG_SAMPLE_PTS_CORRECTION != 0
//            auto correctionAmount = this->prevSampleEnd - samplePts;
//#if CORRECT_IN_HNS == 1
//            double correctionVal = (double)correctionAmount / 10000000.0;
//#else
//            double correctionVal = (double)correctionAmount / (double)this->sampleRate;
//#endif
//
//            H::System::DebugOutput("Corrected by: " + std::to_string(correctionVal) + "s");
//
//            // correction should be not greater than 1s
//            if (std::fabs(correctionVal) > 1.0) {
//                int stop = 234;
//            }
//#endif
//            samplePts = this->prevSampleEnd;
//        }
//
//#if CORRECT_IN_HNS == 1
//        auto finalSamplePts = samplePts;
//#else
//        auto finalSamplePts = H::Math::Convert2(samplePts, sampleRateTmp, 10000000LL);
//#endif
//
//        MF::MFSample mfSampleTmp{
//            H::Chrono::Hns{ finalSamplePts },
//            mfAudioSample->duration,
//            mfAudioSample->offset,
//            mfAudioSample->buffer
//#ifdef _DEBUG
//            , mfAudioSample->trackId
//#endif
//        };
//
//        mfSampleNew = std::make_unique<MFAudioSample>(mfSampleTmp, std::move(mfAudioSample->audioData));
//    }
//    else {
//        mfSampleNew = std::move(mfAudioSample);
//    }
//
//    this->prevSampleEnd = samplePts + sampleDuration;
//    return mfSampleNew;
    return nullptr;
}



// TODO: rewrite with WeakRef
// TODO: write wrapper under sourceReader that will lock access to IMFSourceReader api and use it here and in AvReader
AvSourceStreamManager::AvSourceStreamManager(Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader)
    : sourceReader{ sourceReader }
{}

void AvSourceStreamManager::AddStream(std::unique_ptr<AvStreamBase> stream) {
    std::lock_guard lk{ mx };
    this->AddStream(lk, std::move(stream));
}
size_t AvSourceStreamManager::GetStreamCount(AvStreamType streamType) {
    std::lock_guard lk{ mx };
    return this->GetStreamCount(lk, streamType);
}

AvStreamVideo* AvSourceStreamManager::GetVideoStream(int32_t streamIdx) {
    std::lock_guard lk{ mx };
    return this->GetVideoStream(lk, streamIdx);
}
AvStreamAudio* AvSourceStreamManager::GetAudioStream(int32_t streamIdx) {
    std::lock_guard lk{ mx };
    return this->GetAudioStream(lk, streamIdx);
}

AvStreamVideo* AvSourceStreamManager::GetFirstVideoStream() {
    std::lock_guard lk{ mx };
    return this->GetFirstVideoStream(lk);
}
AvStreamAudio* AvSourceStreamManager::GetFirstAudioStream() {
    std::lock_guard lk{ mx };
    return this->GetFirstAudioStream(lk);
}


void AvSourceStreamManager::DisableAllStreams() {
    std::lock_guard lk{ mx };
    this->DisableAllStreams(lk);
}
void AvSourceStreamManager::DisableStream(AvStreamType streamType, int32_t streamIdx) {
    std::lock_guard lk{ mx };
    this->DisableStream(lk, streamType, streamIdx);
}

void AvSourceStreamManager::SetActiveStream(AvStreamBase* stream) {
    std::lock_guard lk{ mx };
    this->SetActiveStream(lk, stream);
}
void AvSourceStreamManager::SetActiveStream(AvStreamType streamType, int32_t streamIdx) {
    std::lock_guard lk{ mx };
    this->SetActiveStream(lk, streamType, streamIdx);
}

AvStreamBase* AvSourceStreamManager::GetActiveStream(AvStreamType streamType) {
    std::lock_guard lk{ mx };
    return this->GetActiveStream(lk, streamType);
}
AvStreamVideo* AvSourceStreamManager::GetActiveVideoStream() {
    std::lock_guard lk{ mx };
    return this->GetActiveVideoStream(lk);
}
AvStreamAudio* AvSourceStreamManager::GetActiveAudioStream() {
    std::lock_guard lk{ mx };
    return this->GetActiveAudioStream(lk);
}

AudioWaveFormat AvSourceStreamManager::GetWaveFormatForActiveAudioStream() {
    std::lock_guard lk{ mx };
    return this->GetWaveFormatForActiveAudioStream(lk);
}


//
// Internal implementaitions
//
void AvSourceStreamManager::AddStream(H::MethodIsGuarded lk, std::unique_ptr<AvStreamBase> stream) {
    if (auto videoStream = H::dynamic_unique_cast<AvStreamVideo>(std::move(stream))) {
        videoStreams.push_back(std::move(videoStream));
    }
    else if (auto audioStream = H::dynamic_unique_cast<AvStreamAudio>(std::move(stream))) {
        audioStreams.push_back(std::move(audioStream));
    }
    else {
        assert(false);
    }
}
size_t AvSourceStreamManager::GetStreamCount(H::MethodIsGuarded lk, AvStreamType streamType) {
    switch (streamType) {
    case AvStreamType::Video:
        return videoStreams.size();

    case AvStreamType::Audio:
        return audioStreams.size();
    }
    return 0;
}

AvStreamVideo* AvSourceStreamManager::GetVideoStream(H::MethodIsGuarded lk, int32_t streamIdx) {
    for (auto& videoStream : videoStreams) {
        if (videoStream->GetStreamIdx() == streamIdx) {
            return videoStream.get();
        }
    }
    return nullptr;
}
AvStreamAudio* AvSourceStreamManager::GetAudioStream(H::MethodIsGuarded lk, int32_t streamIdx) {
    for (auto& audioStream : audioStreams) {
        if (audioStream->GetStreamIdx() == streamIdx) {
            return audioStream.get();
        }
    }
    return nullptr;
}

AvStreamVideo* AvSourceStreamManager::GetFirstVideoStream(H::MethodIsGuarded lk) {
    if (videoStreams.empty()) {
        return nullptr;
    }
    return videoStreams[0].get();
}
AvStreamAudio* AvSourceStreamManager::GetFirstAudioStream(H::MethodIsGuarded lk) {
    if (audioStreams.empty()) {
        return nullptr;
    }
    return audioStreams[0].get();
}


void AvSourceStreamManager::DisableAllStreams(H::MethodIsGuarded lk) {
    HRESULT hr = this->sourceReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, false);
    H::System::ThrowIfFailed(hr);

    if (activeVideoStream) {
        this->DisableStream(lk, AvStreamType::Video, activeVideoStream->GetStreamIdx());
    }
    if (activeAudioStream) {
        this->DisableStream(lk, AvStreamType::Audio, activeAudioStream->GetStreamIdx());
    }
}

void AvSourceStreamManager::DisableStream(H::MethodIsGuarded lk, AvStreamType streamType, int32_t streamIdx) {
    HRESULT hr = S_OK;

    switch (streamType) {
    case AvStreamType::Video:
        if (auto videoStream = this->GetVideoStream(lk, streamIdx)) {
            hr = this->sourceReader->SetStreamSelection(streamIdx, false);
            H::System::ThrowIfFailed(hr);

            if (activeVideoStream) {
                activeVideoStream->ClearSamples();
            }
            activeVideoStream = nullptr;
        }
        break;

    case AvStreamType::Audio:
        if (auto audioStream = this->GetAudioStream(lk, streamIdx)) {
            hr = this->sourceReader->SetStreamSelection(streamIdx, false);
            H::System::ThrowIfFailed(hr);

            if (activeAudioStream) {
                activeAudioStream->ClearSamples();
            }
            activeAudioStream = nullptr;
        }
        break;
    }
}

void AvSourceStreamManager::SetActiveStream(H::MethodIsGuarded lk, AvStreamBase* stream) {
    if (!stream) {
        LOG_WARNING_D("stream is null");
        return;
    }

    // NOTE: Do not assign outter stream as active directly, use this->SetActiveStream(...) to find correct stream
    if (auto videoStream = dynamic_cast<AvStreamVideo*>(stream)) {
        this->SetActiveStream(lk, AvStreamType::Video, stream->GetStreamIdx());
    }
    else if (auto audioStream = dynamic_cast<AvStreamAudio*>(stream)) {
        this->SetActiveStream(lk, AvStreamType::Audio, stream->GetStreamIdx());
    }
    else {
        assert(false);
    }
}

void AvSourceStreamManager::SetActiveStream(H::MethodIsGuarded lk, AvStreamType streamType, int32_t streamIdx) {
    HRESULT hr = S_OK;

    switch (streamType) {
    case AvStreamType::Video:
        if (auto videoStream = this->GetVideoStream(lk, streamIdx)) {
            if (activeVideoStream) {
                if (activeVideoStream->GetStreamIdx() == streamIdx) {
                    break; // stream already activated, ignore
                }
                else {
                    hr = this->sourceReader->SetStreamSelection(activeVideoStream->GetStreamIdx(), false);
                    H::System::ThrowIfFailed(hr);
                    activeVideoStream->ClearSamples();
                }
            }
            hr = this->sourceReader->SetStreamSelection(streamIdx, true);
            H::System::ThrowIfFailed(hr);
            activeVideoStream = videoStream;
        }
        break;

    case AvStreamType::Audio:
        if (auto audioStream = this->GetAudioStream(lk, streamIdx)) {
            if (activeAudioStream) {
                if (activeAudioStream->GetStreamIdx() == streamIdx) {
                    break; // stream already activated, ignore
                }
                else {
                    hr = this->sourceReader->SetStreamSelection(activeAudioStream->GetStreamIdx(), false);
                    H::System::ThrowIfFailed(hr);
                    activeAudioStream->ClearSamples();
                }
            }
            hr = this->sourceReader->SetStreamSelection(streamIdx, true);
            H::System::ThrowIfFailed(hr);
            activeAudioStream = audioStream;
        }
        break;
    }
}

AvStreamBase* AvSourceStreamManager::GetActiveStream(H::MethodIsGuarded lk, AvStreamType streamType) {
    switch (streamType) {
    case AvStreamType::Video:
        return activeVideoStream;

    case AvStreamType::Audio:
        return activeAudioStream;
    }
    return nullptr;
}
AvStreamVideo* AvSourceStreamManager::GetActiveVideoStream(H::MethodIsGuarded lk) {
    return activeVideoStream;
}
AvStreamAudio* AvSourceStreamManager::GetActiveAudioStream(H::MethodIsGuarded lk) {
    return activeAudioStream;
}

AudioWaveFormat AvSourceStreamManager::GetWaveFormatForActiveAudioStream(H::MethodIsGuarded lk) {
    if (!activeAudioStream) {
        assert(false);
        return {};
    }

    HRESULT hr = S_OK;

    // Get the complete WAVEFORMAT from the Media Type.
    Microsoft::WRL::ComPtr<IMFMediaType> decodedAudioFullType;
    hr = sourceReader->GetCurrentMediaType(activeAudioStream->GetStreamIdx(), &decodedAudioFullType);
    H::System::ThrowIfFailed(hr);

    UINT32 size = 0;
    WAVEFORMATEX* pWaveFormatEx;
    hr = MFCreateWaveFormatExFromMFMediaType(decodedAudioFullType.Get(), &pWaveFormatEx, &size);
    H::System::ThrowIfFailed(hr);

    AudioWaveFormat audioWaveFormat{ pWaveFormatEx };
    CoTaskMemFree(pWaveFormatEx);

    return audioWaveFormat;
}