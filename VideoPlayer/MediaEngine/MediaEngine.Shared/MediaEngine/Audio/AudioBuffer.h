//#pragma once
//#include "IAudioOutput.h"
////#include "SimpleAudioMixer.h"
//#include "../MediaSample.h"
//#include <memory>
//
//
//class SimpleAudioMixer;
//
//struct AudioBufferInitData {
//    FloatAudioBuffer floatAudioBuffer;
//    int64_t pts = 0;
//};
//
//class AudioBuffer {
//public:
//    AudioBuffer(int64_t recordStartHns);
//    AudioBuffer(const AudioBuffer &other);
//    AudioBuffer(AudioBuffer &&other);
//    virtual ~AudioBuffer();
//
//    AudioBuffer &operator=(const AudioBuffer &other);
//    AudioBuffer &operator=(AudioBuffer &&other);
//
//    const float *GetData() const;
//    uint32_t GetSampleRate() const;
//    uint32_t GetChannelCount() const;
//    int64_t GetPtsHns() const;
//    int64_t GetDurationHns() const;
//    int64_t GetPts() const;
//    int64_t GetDuration() const;
//
//    void Mix(MF::MFAudioSample* mfAudioSample, bool& endMixed, bool& oldSample, bool& futureSample, int64_t startPtsHns, size_t sourceCount);
//
//protected:
//    void SetInitData(const AudioBufferInitData &initData);
//
//    void SetPts(int64_t v);
//    void SetDuration(int64_t v);
//
//private:
//    AudioBufferInitData audioBufferInitData;
//    int64_t recordStartHns;
//
//    friend class SimpleAudioMixer;
//
//    //TODO: Mixers should be set in TrackMixRange on calling Track::Render method, 
//    //      so that each Track would be able to have it's own mixer.
//    std::unique_ptr<SimpleAudioMixer> defaultMixer;
//};