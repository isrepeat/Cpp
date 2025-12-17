#pragma once
#include "VideoRateControlModeDef.h"

#include <cstdint>

enum class VideoRateControlMode {
    VideoRateControlMode_Enum
};

struct VideoCodecBasicSettings {
    uint32_t width = 0;
    uint32_t height = 0;

    uint32_t bitrate = 0;
    VideoRateControlMode rateControlMode = VideoRateControlMode::Default;

    uint32_t fps = 0;
};
