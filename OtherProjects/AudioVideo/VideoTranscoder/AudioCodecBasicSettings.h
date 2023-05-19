#pragma once
#include "AudioCodecType.h"
#include <cstdint>

struct AudioCodecBasicSettings {
    uint32_t numChannels;
    uint32_t sampleRate;

    AudioCodecBasicSettings();
};