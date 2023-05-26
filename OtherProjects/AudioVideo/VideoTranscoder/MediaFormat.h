#pragma once
#include "AudioCodecType.h"
#include "VideoCodecType.h"
#include "MediaContainerType.h"
#include "IAudioCodecSettings.h"
#include "IVideoCodecSettings.h"

#include <vector>
#include <memory>

class MediaFormat {
public:
    MediaFormat();
    MediaFormat(
        MediaContainerType containerType,
        std::unique_ptr<IAudioCodecSettings> audioCodecSettings,
        std::unique_ptr<IVideoCodecSettings> videoCodecSettings);

    MediaFormat(const MediaFormat &other);
    MediaFormat(MediaFormat &&other);

    MediaFormat &operator=(MediaFormat other);

    MediaContainerType GetMediaContainerType() const;

    IAudioCodecSettings *GetAudioCodecSettings();
    IVideoCodecSettings *GetVideoCodecSettings();

    const IAudioCodecSettings *GetAudioCodecSettings() const;
    const IVideoCodecSettings *GetVideoCodecSettings() const;

    friend void swap(MediaFormat &a, MediaFormat &b) noexcept;

private:
    MediaContainerType containerType;
    std::unique_ptr<IAudioCodecSettings> audioCodecSettings;
    std::unique_ptr<IVideoCodecSettings> videoCodecSettings;
};