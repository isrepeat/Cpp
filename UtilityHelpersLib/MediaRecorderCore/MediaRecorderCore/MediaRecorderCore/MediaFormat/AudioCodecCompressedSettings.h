#pragma once
#include "IAudioCodecSettings.h"

class AudioCodecCompressedSettings : public IAudioCodecSettings {
public:
    AudioCodecCompressedSettings(AudioCodecType codecType);

    const AudioCodecBasicSettings *GetBasicSettings() const override;
    void SetBasicSettings(const AudioCodecBasicSettings &v) override;

    const AudioCodecBitrateSettings *GetBitrateSettings() const override;
    void SetBitrateSettings(const AudioCodecBitrateSettings &v) override;

    std::unique_ptr<IAudioCodecSettings> Clone() override;
    std::shared_ptr<IAudioCodecSettings> CloneShared() override;

private:
    AudioCodecBasicSettings basicSettings;
    AudioCodecBitrateSettings bitrateSettings;
};