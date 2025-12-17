#include "pch.h"
#include "IAudioCodecSettings.h"

#include <exception>

IAudioCodecSettings::IAudioCodecSettings(AudioCodecType codecType)
    : codecType(codecType)
{}

const AudioCodecBitrateSettings* IAudioCodecSettings::GetBitrateSettings() const {
    return nullptr;
}

void IAudioCodecSettings::SetBitrateSettings(const AudioCodecBitrateSettings &/*v*/) {
    throw std::exception("not implemeted");
}

AudioCodecType IAudioCodecSettings::GetCodecType() const {
    return this->codecType;
}
