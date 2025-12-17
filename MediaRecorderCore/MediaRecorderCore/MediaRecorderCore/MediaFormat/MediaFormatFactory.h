#pragma once
#include "MediaFormat.h"
#include "AudioCodecSettingsItem.h"

#include <vector>
#include <cstdint>
#include <memory>
#include <libhelpers\MediaFoundation\MFInclude.h>

using AudioCodecSettingsValues = std::vector<AudioCodecSettingsItem>;

// encoder restrictions can be found here : https://msdn.microsoft.com/en-us/library/windows/desktop/dd742785(v=vs.85).aspx
class MediaFormatFactory {
public:
    MediaFormat CreateMediaFormat(MediaContainerType container, AudioCodecType audioCodec);
    MediaFormat CreateMediaFormat(MediaContainerType container, VideoCodecType videoCodec);
    MediaFormat CreateMediaFormat(
        MediaContainerType container,
        AudioCodecType audioCodec,
        VideoCodecType videoCodec);

    AudioCodecSettingsValues GetSettingsValues(AudioCodecType codec) const;
    AudioCodecSettingsValues GetClosestSettingsValues(const IAudioCodecSettings& audioCodecSettings) const;

private:
    const AudioCodecSettingsValues pcmSettingsValues = MediaFormatFactory::CreatePcmSettingsValues();

    std::unique_ptr<IAudioCodecSettings> CreateAudioCodecSettings(AudioCodecType codec) const;
    std::unique_ptr<IVideoCodecSettings> CreateVideoCodecSettings(VideoCodecType codec) const;

    void CheckCodecs(
        MediaContainerType container,
        AudioCodecType audioCodec,
        VideoCodecType videoCodec);

    static std::optional<AudioCodecBasicSettings> GetAudioCodecBasicSettings(IMFMediaType& type);
    static std::optional<AudioCodecBitrateSettings> GetAudioCodecBitrateSettings(IMFMediaType& type);

    static AudioCodecSettingsValues CreatePcmSettingsValues();
};
