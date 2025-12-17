#include "pch.h"
#include "MediaFormatCodecs.h"

MediaFormatCodecs::MediaFormatCodecs()
    : containerType(MediaContainerType::Unknown)
{}

MediaFormatCodecs::MediaFormatCodecs(
    MediaContainerType containerType,
    std::vector<AudioCodecType> audioCodecs,
    std::vector<VideoCodecType> videoCodecs)
    : containerType(containerType),
    audioCodecs(std::move(audioCodecs)),
    videoCodecs(std::move(videoCodecs))
{}

MediaContainerType MediaFormatCodecs::GetMediaContainerType() const {
    return this->containerType;
}

const std::vector<AudioCodecType> &MediaFormatCodecs::GetAudioCodecs() const {
    return this->audioCodecs;
}

const std::vector<VideoCodecType> &MediaFormatCodecs::GetVideoCodecs() const {
    return this->videoCodecs;
}