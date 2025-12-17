#pragma once
#include "IAudioCodecSettings.h"

class AudioCodecLosslessSettings : public IAudioCodecSettings {
public:
    AudioCodecLosslessSettings(AudioCodecType codecType);

    const AudioCodecBasicSettings *GetBasicSettings() const override;
    void SetBasicSettings(const AudioCodecBasicSettings &v) override;

    std::unique_ptr<IAudioCodecSettings> Clone() override;
    std::shared_ptr<IAudioCodecSettings> CloneShared() override;

private:
    AudioCodecBasicSettings basicSettings;
};