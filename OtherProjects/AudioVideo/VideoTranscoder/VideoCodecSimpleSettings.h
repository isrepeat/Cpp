#pragma once
#include "IVideoCodecSettings.h"

class VideoCodecSimpleSettings : public IVideoCodecSettings {
public:
    VideoCodecSimpleSettings(VideoCodecType codecType);

    const VideoCodecBasicSettings *GetBasicSettings() const override;
    void SetBasicSettings(const VideoCodecBasicSettings &v) override;

    std::unique_ptr<IVideoCodecSettings> Clone() override;
    std::shared_ptr<IVideoCodecSettings> CloneShared() override;

private:
    VideoCodecBasicSettings basicSettings;
};