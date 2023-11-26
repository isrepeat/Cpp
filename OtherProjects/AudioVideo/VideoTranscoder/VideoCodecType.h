#pragma once
#define VideoCodecType_Enum \
    Unknown,                \
    H264,                   \
    HEVC,                   \
    WMV1,                   \
    WMV3,

enum class VideoCodecType {
    VideoCodecType_Enum

    CountHelper,
    Count = CountHelper - 1, // don't count Unknown
    First = H264
};