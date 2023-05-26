#pragma once
#include "IAudioCodecSettings.h"
#include <exception>

IAudioCodecSettings::IAudioCodecSettings(AudioCodecType codecType)
    : codecType(codecType)
{}

IAudioCodecSettings::~IAudioCodecSettings()
{}

const AudioCodecBitrateSettings* IAudioCodecSettings::GetBitrateSettings() const {
    return nullptr;
}

void IAudioCodecSettings::SetBitrateSettings(const AudioCodecBitrateSettings& v) {
    throw std::exception("not implemeted");
}

bool IAudioCodecSettings::HasBasicSettings() const {
    return this->GetBasicSettings() != nullptr;
}

bool IAudioCodecSettings::HasBitrateSettings() const {
    return this->GetBitrateSettings() != nullptr;
}

AudioCodecType IAudioCodecSettings::GetCodecType() const {
    return this->codecType;
}