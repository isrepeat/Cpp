#pragma once
#include "VideoCodecType.h"
#include "VideoCodecBasicSettings.h"

#include <libhelpers/ICloneable.h>

class IVideoCodecSettings : public ICloneable<IVideoCodecSettings> {
public:
    IVideoCodecSettings(VideoCodecType codecType);
    virtual ~IVideoCodecSettings() = default;

    virtual const VideoCodecBasicSettings* GetBasicSettings() const = 0;
    virtual void SetBasicSettings(const VideoCodecBasicSettings &v) = 0;

    VideoCodecType GetCodecType() const;

private:
    VideoCodecType codecType;
};
