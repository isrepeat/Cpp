#pragma once
#include "AudioCodecBasicSettings.h"
#include "AudioCodecBitrateSettings.h"

#include <optional>

struct AudioCodecSettingsItem {
    std::optional<AudioCodecBasicSettings> basicSettings;
    std::optional<AudioCodecBitrateSettings> bitrateSettings;
};
