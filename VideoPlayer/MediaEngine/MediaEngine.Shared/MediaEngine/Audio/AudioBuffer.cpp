//#include "AudioBuffer.h"
//
//
//AudioBuffer::AudioBuffer(int64_t recordStartHns)
//    : recordStartHns(recordStartHns)
//{
//    this->defaultMixer = std::make_unique<SimpleAudioMixer>();
//}
//
//AudioBuffer::AudioBuffer(const AudioBuffer &other)
//    : audioBufferInitData(other.audioBufferInitData)
//    , recordStartHns(other.recordStartHns)
//{
//    assert(false);
//}
//
//AudioBuffer::AudioBuffer(AudioBuffer &&other)
//    : audioBufferInitData(std::move(other.audioBufferInitData))
//    , recordStartHns(std::move(other.recordStartHns))
//    , defaultMixer(std::move(other.defaultMixer))
//{
//    assert(false);
//}
//
//AudioBuffer::~AudioBuffer() {
//}
//
//AudioBuffer &AudioBuffer::operator=(const AudioBuffer &other) {
//    if (this != &other) {
//        this->audioBufferInitData = other.audioBufferInitData;
//        this->recordStartHns = other.recordStartHns;
//        this->defaultMixer = std::make_unique<SimpleAudioMixer>();
//    }
//    return *this;
//}
//
//AudioBuffer &AudioBuffer::operator=(AudioBuffer &&other) {
//    if (this != &other) {
//        this->audioBufferInitData = std::move(other.audioBufferInitData);
//        this->recordStartHns = std::move(other.recordStartHns);
//        this->defaultMixer = std::move(other.defaultMixer);
//    }
//    return *this;
//}
//
//const float *AudioBuffer::GetData() const {
//    return this->audioBufferInitData.floatAudioBuffer.data;
//}
//
//uint32_t AudioBuffer::GetSampleRate() const {
//    return this->audioBufferInitData.floatAudioBuffer.waveFormatEx.nSamplesPerSec;
//}
//
//uint32_t AudioBuffer::GetChannelCount() const {
//    return this->audioBufferInitData.floatAudioBuffer.waveFormatEx.nChannels;
//}
//
//int64_t AudioBuffer::GetPtsHns() const {
//    int64_t ptsHns = HMath::Convert(this->audioBufferInitData.pts, (int64_t)this->audioBufferInitData.floatAudioBuffer.waveFormatEx.nSamplesPerSec, 10000000LL);
//    return ptsHns;
//}
//
//int64_t AudioBuffer::GetDurationHns() const {
//    int64_t durationHns = HMath::Convert(this->audioBufferInitData.floatAudioBuffer.duration, (int64_t)this->audioBufferInitData.floatAudioBuffer.waveFormatEx.nSamplesPerSec, 10000000LL);
//    return durationHns;
//}
//
//int64_t AudioBuffer::GetPts() const {
//    return this->audioBufferInitData.pts;
//}
//
//int64_t AudioBuffer::GetDuration() const {
//    return this->audioBufferInitData.floatAudioBuffer.duration;
//}
//
//void AudioBuffer::Mix(MF::MFAudioSample* mfAudioSample, bool& endMixed, bool& oldSample, bool& futureSample, int64_t startPtsHns, size_t sourceCount) {
//    int64_t samplePts = HMath::Convert((int64_t)mfAudioSample->pts - startPtsHns - this->recordStartHns, 10000000LL, (int64_t)this->audioBufferInitData.floatAudioBuffer.waveFormatEx.nSamplesPerSec);
//    int64_t sampleDuration = HMath::Convert((int64_t)mfAudioSample->duration, 10000000LL, (int64_t)this->audioBufferInitData.floatAudioBuffer.waveFormatEx.nSamplesPerSec);
//    int64_t bufPts = this->audioBufferInitData.pts;
//    int64_t bufDuration = this->audioBufferInitData.floatAudioBuffer.duration;
//
//    int64_t bufferIdx = HMath::Clamp(samplePts - bufPts, 0LL, this->audioBufferInitData.floatAudioBuffer.duration);
//    int64_t sampleIdx = HMath::Clamp(bufPts - samplePts, 0LL, sampleDuration);
//
//    futureSample = bufferIdx == bufDuration;
//    oldSample = sampleIdx == sampleDuration;
//
//    if (!futureSample && !oldSample) {
//        int64_t mixCount;
//        int64_t sampleMixCount = sampleDuration - sampleIdx;
//        int64_t bufferMixCount = bufDuration - bufferIdx;
//
//        if (sampleMixCount <= bufferMixCount) {
//            mixCount = sampleMixCount;
//            endMixed = true;
//        }
//        else {
//            mixCount = bufferMixCount;
//            endMixed = false;
//        }
//
//        int64_t tmpbufferIdx = bufferIdx;
//        int64_t tmpsampleIdx = sampleIdx;
//        int64_t tmpmixCount = mixCount;
//
//        auto mfAudioSampleDataAccessor = mfAudioSample->GetDataAccessor();
//        
//        SimpleMixerParams simpleParams;
//        simpleParams.audioBuffer = this;
//        simpleParams.audioSourceCount = sourceCount;
//        simpleParams.bufferIdx = bufferIdx;
//        simpleParams.mixCount = mixCount;
//        simpleParams.sample = static_cast<const float*>(mfAudioSampleDataAccessor.GetData());
//        simpleParams.sampleFloatCount = (int64_t)(mfAudioSampleDataAccessor.GetSize() / sizeof(float));
//        simpleParams.sampleIdx = sampleIdx;
//        
//        defaultMixer->Mix(&simpleParams);
//    }
//    //LOG_DEBUG_D("\n"
//    //    "samplePts = {} [{}ms]\n"
//    //    "sampleDuration = {} [{}ms]\n"
//    //    "bufPts = {} [{}ms]\n"
//    //    "bufDuration = {} [{}ms]\n"
//    //    "endMixed = {}\n"
//    //    "oldSample = {}\n"
//    //    "futureSample = {}\n"
//    //    , samplePts, std::chrono::duration_cast<H::Chrono::milliseconds_f>(mfAudioSample->pts).count()
//    //    , sampleDuration, std::chrono::duration_cast<H::Chrono::milliseconds_f>(mfAudioSample->duration).count()
//    //    , bufPts, this->audioBufferInitData.pts
//    //    , bufDuration, this->audioBufferInitData.floatAudioBuffer.duration
//    //    , endMixed
//    //    , oldSample
//    //    , futureSample
//    //);
//}
//
//
//void AudioBuffer::SetInitData(const AudioBufferInitData &initData) {
//    this->audioBufferInitData = initData;
//}
//
//void AudioBuffer::SetPts(int64_t v) {
//    assert(false);
//}
//
//void AudioBuffer::SetDuration(int64_t v) {
//    assert(false);
//}