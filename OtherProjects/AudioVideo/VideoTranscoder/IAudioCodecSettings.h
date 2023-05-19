#pragma once
#include "AudioCodecBasicSettings.h"
#include "AudioCodecBitrateSettings.h"

#include "ICloneable.h"

class IAudioCodecSettings : public ICloneable<IAudioCodecSettings> {
public:
    IAudioCodecSettings(AudioCodecType codecType);
    IAudioCodecSettings(const IAudioCodecSettings&) = default;
    IAudioCodecSettings(IAudioCodecSettings&&) = default;
    virtual ~IAudioCodecSettings();

    IAudioCodecSettings& operator=(const IAudioCodecSettings&) = default;
    IAudioCodecSettings& operator=(IAudioCodecSettings&&) = default;

    virtual const AudioCodecBasicSettings* GetBasicSettings() const = 0;
    virtual void SetBasicSettings(const AudioCodecBasicSettings& v) = 0;

    virtual const AudioCodecBitrateSettings* GetBitrateSettings() const;
    virtual void SetBitrateSettings(const AudioCodecBitrateSettings& v);

    bool HasBasicSettings() const;
    bool HasBitrateSettings() const;

    AudioCodecType GetCodecType() const;

private:
    AudioCodecType codecType;
};