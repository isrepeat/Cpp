#pragma once
#include "VideoCodecType.h"
#include "VideoCodecBasicSettings.h"

#include "ICloneable.h"

class IVideoCodecSettings : public ICloneable<IVideoCodecSettings> {
public:
    IVideoCodecSettings(VideoCodecType codecType);
    IVideoCodecSettings(const IVideoCodecSettings&) = default;
    IVideoCodecSettings(IVideoCodecSettings&&) = default;
    virtual ~IVideoCodecSettings();

    IVideoCodecSettings &operator=(const IVideoCodecSettings&) = default;
    IVideoCodecSettings &operator=(IVideoCodecSettings&&) = default;

    virtual const VideoCodecBasicSettings *GetBasicSettings() const = 0;
    virtual void SetBasicSettings(const VideoCodecBasicSettings &v) = 0;

    bool HasBasicSettings() const;

    VideoCodecType GetCodecType() const;

private:
    VideoCodecType codecType;
};