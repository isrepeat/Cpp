#include "pch.h"
#include "AudioCodecCompressedSettings.h"

AudioCodecCompressedSettings::AudioCodecCompressedSettings(AudioCodecType codecType)
    : IAudioCodecSettings(codecType)
{}

const AudioCodecBasicSettings *AudioCodecCompressedSettings::GetBasicSettings() const {
    return &this->basicSettings;
}

void AudioCodecCompressedSettings::SetBasicSettings(const AudioCodecBasicSettings &v) {
    this->basicSettings = v;
}

const AudioCodecBitrateSettings *AudioCodecCompressedSettings::GetBitrateSettings() const {
    return &this->bitrateSettings;
}

void AudioCodecCompressedSettings::SetBitrateSettings(const AudioCodecBitrateSettings &v) {
    this->bitrateSettings = v;
}

std::unique_ptr<IAudioCodecSettings> AudioCodecCompressedSettings::Clone() {
    return std::make_unique<AudioCodecCompressedSettings>(*this);
}

std::shared_ptr<IAudioCodecSettings> AudioCodecCompressedSettings::CloneShared() {
    return std::make_shared<AudioCodecCompressedSettings>(*this);
}