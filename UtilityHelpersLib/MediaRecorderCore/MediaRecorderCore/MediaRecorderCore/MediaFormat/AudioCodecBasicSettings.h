#pragma once
#include "AudioCodecType.h"

#include <cstdint>

struct AudioCodecBasicSettings {
    uint32_t numChannels = 0;
    uint32_t sampleRate = 0;
};
