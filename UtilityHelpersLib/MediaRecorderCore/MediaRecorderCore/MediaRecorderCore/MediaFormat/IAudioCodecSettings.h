#pragma once
#include "AudioCodecBasicSettings.h"
#include "AudioCodecBitrateSettings.h"

#include <optional>
#include <libhelpers/ICloneable.h>

class IAudioCodecSettings : public ICloneable<IAudioCodecSettings> {
public:
    IAudioCodecSettings(AudioCodecType codecType);
    virtual ~IAudioCodecSettings() = default;

    virtual const AudioCodecBasicSettings* GetBasicSettings() const = 0;
    virtual void SetBasicSettings(const AudioCodecBasicSettings &v) = 0;

    virtual const AudioCodecBitrateSettings* GetBitrateSettings() const;
    virtual void SetBitrateSettings(const AudioCodecBitrateSettings &v);

    AudioCodecType GetCodecType() const;

private:
    AudioCodecType codecType;
};
