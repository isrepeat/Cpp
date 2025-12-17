#pragma once
#include "MediaFormatCodecs.h"
#include "MediaFormatCodecsSupport.h"

#include <vector>
#include <guiddef.h>

class MediaFormatCodecsEnumerator {
public:
    MediaFormatCodecsEnumerator();

    MediaFormatCodecs GetMediaFormatCodecs(MediaContainerType containerType) const;
    std::vector<MediaFormatCodecs> EnumerateMediaFormatCodecs() const;

private:
    template<class T>
    static std::vector<T> Intersect(const std::vector<T> &a, const std::vector<T> &b);

    MediaFormatCodecsSupport* codecSup;
    std::vector<AudioCodecType> audioCodecs;
    std::vector<VideoCodecType> videoCodecs;
};