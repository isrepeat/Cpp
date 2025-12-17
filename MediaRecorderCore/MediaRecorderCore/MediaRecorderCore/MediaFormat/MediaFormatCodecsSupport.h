#pragma once
#include "MediaFormatCodecs.h"

#include <vector>
#include <Windows.h>

class MediaFormatCodecsSupport {
public:
    static MediaFormatCodecsSupport * Instance();

    const MediaFormatCodecs &GetCodecsSupport(MediaContainerType container) const;

    AudioCodecType MapAudioCodec(const GUID &codecGuid) const;
    VideoCodecType MapVideoCodec(const GUID &codecGuid) const;

    GUID MapAudioCodec(AudioCodecType type) const;
    GUID MapVideoCodec(VideoCodecType type) const;

private:
    static const MediaFormatCodecs UnknownFmt;
    std::vector<MediaFormatCodecs> codecsSupport;
    std::vector<std::pair<GUID, AudioCodecType>> audioCodecMap;
    std::vector<std::pair<GUID, VideoCodecType>> videoCodecMap;

    MediaFormatCodecsSupport();

    void Add(
        MediaContainerType containerType,
        std::vector<AudioCodecType> audioCodecs,
        std::vector<VideoCodecType> videoCodecs);

    static size_t MediaContainerTypeToIdx(MediaContainerType containerType);
};