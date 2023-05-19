#pragma once
#include "AudioCodecType.h"
#include "VideoCodecType.h"
#include "MediaContainerType.h"

#include <vector>

class MediaFormatCodecs {
public:
    MediaFormatCodecs();
    MediaFormatCodecs(
        MediaContainerType containerType,
        std::vector<AudioCodecType> audioCodecs,
        std::vector<VideoCodecType> videoCodecs);
    MediaFormatCodecs(const MediaFormatCodecs&) = default;
    MediaFormatCodecs(MediaFormatCodecs&&) = default;

    MediaFormatCodecs &operator=(const MediaFormatCodecs&) = default;
    MediaFormatCodecs &operator=(MediaFormatCodecs&&) = default;

    MediaContainerType GetMediaContainerType() const;

    const std::vector<AudioCodecType> &GetAudioCodecs() const;
    const std::vector<VideoCodecType> &GetVideoCodecs() const;

private:
    MediaContainerType containerType;
    std::vector<AudioCodecType> audioCodecs;
    std::vector<VideoCodecType> videoCodecs;
};