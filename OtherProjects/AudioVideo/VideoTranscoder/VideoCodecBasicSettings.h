#pragma once
#include <cstdint>

struct VideoCodecBasicSettings {
    uint32_t width;
    uint32_t height;
    uint32_t bitrate;
    uint32_t fps;

    VideoCodecBasicSettings();
};