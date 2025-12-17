#pragma once
#include "VideoCodecTypeDef.h"

enum class VideoCodecType {
    VideoCodecType_Enum

    CountHelper,
    Count = CountHelper - 1, // don't count Unknown
    First = H264
};