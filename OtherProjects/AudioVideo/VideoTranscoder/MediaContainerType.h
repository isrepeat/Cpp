#pragma once
#include <cstdint>
#define MediaContainerType_Enum     \
    Unknown,                        \
    MP4,                            \
    WMV,                            \
    MP3,                            \
    M4A,                            \
    FLAC,                           \
    WMA,                            \
    WAV,                            \
    ThreeGPP,

enum class MediaContainerType : uint32_t {
    MediaContainerType_Enum

    CountHelper,
    Count = CountHelper - 1,
    First = MP4
};