#include "pch.h"
#include "AudioCodecLosslessSettings.h"

AudioCodecLosslessSettings::AudioCodecLosslessSettings(AudioCodecType codecType)
    : IAudioCodecSettings(codecType)
{}

const AudioCodecBasicSettings *AudioCodecLosslessSettings::GetBasicSettings() const {
    return &this->basicSettings;
}

void AudioCodecLosslessSettings::SetBasicSettings(const AudioCodecBasicSettings &v) {
    this->basicSettings = v;
}

std::unique_ptr<IAudioCodecSettings> AudioCodecLosslessSettings::Clone() {
    return std::make_unique<AudioCodecLosslessSettings>(*this);
}

std::shared_ptr<IAudioCodecSettings> AudioCodecLosslessSettings::CloneShared() {
    return std::make_shared<AudioCodecLosslessSettings>(*this);
}