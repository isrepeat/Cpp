#pragma once
#include "MediaFormat.h"

#include <vector>
#include <cstdint>
#include <memory>

struct AudioCodecSettingsValues {
    std::vector<uint32_t> NumChannels;
    std::vector<uint32_t> SampleRate;
    std::vector<uint32_t> Bitrate;
};

class MediaFormatFactory {
public:
    MediaFormatFactory();

    MediaFormat CreateMediaFormat(MediaContainerType container, AudioCodecType audioCodec);
    MediaFormat CreateMediaFormat(MediaContainerType container, VideoCodecType videoCodec);
    MediaFormat CreateMediaFormat(
        MediaContainerType container,
        AudioCodecType audioCodec,
        VideoCodecType videoCodec);

    //AudioCodecSettingsValues GetSettingsValues(AudioCodecType codec) const;

private:
    AudioCodecSettingsValues unknownAudioSettings;
    AudioCodecSettingsValues aacSettingsValues;
    AudioCodecSettingsValues mp3SettingsValues;
    AudioCodecSettingsValues amrNbSettingsValues;
    AudioCodecSettingsValues losslessSettingsValues;

    std::unique_ptr<IAudioCodecSettings> CreateAudioCodecSettings(AudioCodecType codec) const;
    std::unique_ptr<IVideoCodecSettings> CreateVideoCodecSettings(VideoCodecType codec) const;

    void CheckCodecs(
        MediaContainerType container,
        AudioCodecType audioCodec,
        VideoCodecType videoCodec);
};